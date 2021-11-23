//
// Created by bot on 06.11.21.
//

#include "../include/app.hpp"
#include "../include/counting_game.hpp"
#include "../include/string_calculator.hpp"
#include "../include/load_operators.hpp"

namespace Bot {
    using Str = Bot::StringCalculator;
    void App::run() {
        settings = std::make_unique<Settings>("settings.json");
        rollSelector = std::make_unique<RollSelector>(*settings);

        Bot::LoadOperators::loadNumbers(settings->getUnicodeTranslationLocation());
        Bot::LoadOperators::loadOperators();

        uint32_t intents = 0;
        intents += dpp::i_guild_integrations;
        intents += dpp::i_guild_messages;
        intents += dpp::i_guild_message_reactions;
        intents += dpp::i_guild_message_typing;
        intents += dpp::i_guilds;

        bot = std::make_unique<dpp::cluster>(settings->getToken(), intents);
        Bot::CountingGame game(this);

        bot->on_message_create([this,&game](const dpp::message_create_t &message) {
            if (message.msg->author->id == bot->me.id) return;
            game.count(*bot,message);
        });

        bot->on_interaction_create([&game,this](const dpp::interaction_create_t &interaction) {
            game.onInteraction(interaction, this);
        });

        bot->on_ready([this, &game](const dpp::ready_t &event) {
            this->registerSettings(*bot,*settings);
            game.addCommands(*bot,*settings,this);
            std::cout << "bot ready" << "\n";
        });

        bot->on_message_reaction_add([this](const dpp::message_reaction_add_t &event){
            rollSelector->onMessageReactionAdd(this,event);
        });

        bot->on_message_reaction_remove([this](const dpp::message_reaction_remove_t &event){
            rollSelector->onMessageReactionRemove(this,event);
        });

        std::cout << "start bot" << "\n";
        bot->start(false);
    }

    void App::registerSettingsModuals(dpp::slashcommand &baseCommand) {
        rollSelector->addSettings(baseCommand,this);
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

        registerCommand(bot,settings,baseCommand,[this](const dpp::interaction_create_t &interaction){
            dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
            if (!cmd_data.options.empty()) {
                auto option = cmd_data.options[0];
                if (option.type == dpp::co_sub_command_group && !option.options.empty()) {
                    auto callBack = settingCallBacks.find(option.name+" "+option.options[0].name);
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
                interaction.reply(dpp::ir_pong,"");
            }
        });
    }

    using call_back = std::function<void(const dpp::interaction_create_t &)>;

    void App::registerSetting(dpp::slashcommand baseCommand, dpp::command_option &command, call_back callBack,int location) {
        if (command.type == dpp::co_sub_command) {
            settingCallBacks.insert(std::pair<std::string,call_back>(command.name,callBack));
        } else if (command.type == dpp::co_sub_command_group) {
            settingCallBacks.insert(std::pair<std::string,call_back>(command.name+" "+command.options[location].name,callBack));
        }
    }

    void App::registerCommand(dpp::cluster &bot, Settings &settings, dpp::slashcommand &command,
                                       std::function<void(
                                               const dpp::interaction_create_t &interaction)> interactionCallBack) {
        //TODO batch it up later
        bot.guild_command_create(command, settings.getServerId(), [&bot, command, this, interactionCallBack](
                const dpp::confirmation_callback_t &callback) {
            if (callback.is_error()) {
                std::cout << callback.get_error().message << "\n";
                for (auto &v : callback.get_error().errors) {
                    std::cout<<"code="<<v.code<<"\n field="<<v.field<<"\n object="<<v.object<<"\n reason="<<v.reason<<"\n";
                }
            }
            dpp::slashcommand cmd_data = std::get<dpp::slashcommand>(callback.value);
            uint64_t id = cmd_data.id;
            auto returnValue = this->commands.insert(std::pair<uint64_t, dpp::slashcommand>(id, command));
            if (returnValue.second) {
                returnValue.first->second.id = id;
                this->interactions.insert(
                        std::pair<uint64_t, std::function<void(const dpp::interaction_create_t &interaction)>>(
                                id,
                                interactionCallBack
                        ));
                std::cout << "command " << returnValue.first->second.name << " ready\n";
            } else {
                std::cout << "ERROR failed to register command" << "\n";
            }
        });
    }
}