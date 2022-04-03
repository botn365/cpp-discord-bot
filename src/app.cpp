//
// Created by bot on 06.11.21.
//

#include "../include/app.hpp"
#include "../include/counting_game.hpp"

namespace Bot {

    void App::run() {
        settings = std::make_unique<Settings>("settings.json");
        rollSelector = std::make_unique<RollSelector>(*settings);
        voiceHandler = std::make_unique<VoiceHandler>(*this);

        uint32_t intents = 0;
        intents += dpp::i_guild_integrations;
        intents += dpp::i_guild_messages;
        intents += dpp::i_guild_message_reactions;
        intents += dpp::i_guild_message_typing;
        intents += dpp::i_guilds;
        intents += dpp::i_guild_voice_states;

        bot = std::make_unique<dpp::cluster>(settings->getToken(), intents);
        countingGame = std::make_unique<CountingGame>(this);

        bot->on_message_create([this](const dpp::message_create_t &message) {
            if (message.msg->author->id == bot->me.id) return;
            countingGame->count(this, message);
        });

        bot->on_interaction_create([this](const dpp::interaction_create_t &interaction) {
            if (interaction.command.type == dpp::it_application_command) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                auto result = commands.find(cmd_data.id);
                if (result != commands.end()) {
                    result->second.callBack(interaction);
                }
            }
        });

        bot->on_ready([this](const dpp::ready_t &event) {
            this->registerSettings(*bot, *settings);
            countingGame->addCommands(*bot, *settings, this);
            batchUploadCommands();
            std::cout << "bot ready" << "\n";
        });

        bot->on_message_reaction_add([this](const dpp::message_reaction_add_t &event) {
            rollSelector->onMessageReactionAdd(this, event);
        });

        bot->on_message_reaction_remove([this](const dpp::message_reaction_remove_t &event) {
            rollSelector->onMessageReactionRemove(this, event);
        });

        bot->on_voice_state_update([this](const dpp::voice_state_update_t &event) {
            voiceHandler->onVoiceStateUpdate(event);
        });

        bot->on_message_delete([this](const dpp::message_delete_t &event){
            countingGame->onMessageDelete(this,event);
        });

        std::cout << "start bot" << "\n";
        bot->start(false);
    }

    void App::registerSettingsModuals(dpp::slashcommand &baseCommand) {
        rollSelector->addSettings(baseCommand, this);
        countingGame->addSettings(baseCommand, this);
        voiceHandler->addSettings(baseCommand);
    }

    void App::registerSettings(dpp::cluster &bot, Settings &settings) {
        dpp::slashcommand baseCommand;
        std::string name = "settings";
        baseCommand.set_name(name);
        baseCommand.set_description("set the settings");
        baseCommand.set_type(dpp::ctxm_chat_input);
        baseCommand.set_application_id(bot.me.id);
        baseCommand.disable_default_permissions();
        for (auto &com: settings.getCommandPermissions(name)) {
            baseCommand.add_permission(com);
        }

        registerSettingsModuals(baseCommand);

        registerCommand(bot, settings, baseCommand, [this](const dpp::interaction_create_t &interaction) {
            dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
            if (!cmd_data.options.empty()) {
                auto option = cmd_data.options[0];
                if (option.type == dpp::co_sub_command_group && !option.options.empty()) {
                    auto callBack = settingCallBacks.find(option.name + " " + option.options[0].name);
                    if (callBack != settingCallBacks.end()) {
                        callBack->second(interaction);
                    }
                } else if (option.type == dpp::co_sub_command) {
                    auto callBack = settingCallBacks.find(option.name);
                    if (callBack != settingCallBacks.end()) {
                        callBack->second(interaction);
                    }
                }
            } else {
                interaction.reply(dpp::ir_pong, "");
            }
        });
    }

    using call_back = std::function<void(const dpp::interaction_create_t &)>;

    void App::registerSetting(dpp::slashcommand baseCommand, dpp::command_option &command, call_back callBack,
                              int location) {
        if (command.type == dpp::co_sub_command) {
            settingCallBacks.insert(std::pair<std::string, call_back>(command.name, callBack));
        } else if (command.type == dpp::co_sub_command_group) {
            settingCallBacks.insert(
                    std::pair<std::string, call_back>(command.name + " " + command.options[location].name, callBack));
        }
    }

    void App::registerCommand(dpp::cluster &bot, Settings &settings, dpp::slashcommand &command,
                              std::function<void(
                                      const dpp::interaction_create_t &interaction)> interactionCallBack) {
        tempCommandVector.push_back(command);
        tempCallBackVector.push_back(std::move(interactionCallBack));
    }

    void App::batchUploadCommands() {
        bot->guild_bulk_command_create(tempCommandVector, settings->getServerId(),
                                       [this](const dpp::confirmation_callback_t &callBack) {
                                           if (!callBack.is_error()) {
                                               auto commandMap = std::get<dpp::slashcommand_map>(callBack.value);
                                               for (auto &com: commandMap) {
                                                   for (int i = 0; i < tempCommandVector.size(); i++) {
                                                       auto &tempCom = tempCommandVector[i];
                                                       if (com.second.name == tempCom.name) {
                                                           tempCom.id = com.second.id;
                                                           commands.insert(std::pair{tempCom.id, Interaction{tempCom,
                                                                                                             std::move(
                                                                                                                     tempCallBackVector[i])}});
                                                           if (com.second.name == "settings") {
                                                               for (auto &value: com.second.options) {
                                                                   for (auto &command : value.options) {
                                                                       std::cout << "regitert command " << value.name << " " << command.name << "\n";
                                                                   }
                                                               }
                                                           }
                                                           std::cout << "regitert command " << com.second.name << "\n";
                                                       }
                                                   }
                                               }
                                           } else {
                                               throw std::runtime_error("failed to create commands");
                                           }
                                           std::cout << "commands registert\n";
                                           tempCommandVector.clear();
                                           tempCallBackVector.clear();
                                       });
    }
}