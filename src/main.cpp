#include <iostream>
#include <dpp/dpp.h>
#include "../include/counting_game.hpp"
#include "../include/string_calculator.hpp"
#include "../include/load_operators.hpp"
#include "../include/settings.hpp"

using Str = Bot::StringCalculator;

int main() {

    Bot::Settings settings("settings.json");

    Bot::LoadOperators::loadNumbers(settings.getUnicodeTranslationLocation());
    Bot::LoadOperators::loadOperators();

    std::string_view in = "4!";
    auto list = Bot::StringCalculator::convertStringToRPNList(in);
    double value = Bot::StringCalculator::calculateFromRPNList(list);
    std::cout<<value<<"\n";
    return 0;

    uint32_t intents = 0;
    intents += dpp::i_guild_integrations;
    intents += dpp::i_guild_messages;
    intents += dpp::i_guild_message_reactions;
    intents += dpp::i_guild_message_typing;

    dpp::cluster bot(settings.getToken(), intents);
    dpp::snowflake server(settings.getServerId());
    Bot::CountingGame game(settings.getCountDBLocation());

    bot.on_message_create([&bot,&game](const dpp::message_create_t &message) {
        if (message.msg->author->id == bot.me.id) return;
        game.count(bot,message);
    });

    bot.on_interaction_create([&bot,&game](const dpp::interaction_create_t &interaction) {
        if (interaction.command.type == dpp::it_application_command) {
            dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
            if (cmd_data.name == "set_counting_channel") {
                auto value = std::get<dpp::snowflake>(cmd_data.options[0].value);
                interaction.reply(dpp::ir_channel_message_with_source,
                                  dpp::message()
                                  .set_type(dpp::mt_reply)
                                  .set_flags(dpp::m_ephemeral)
                                  .set_content("<#"+ std::to_string(value)+"> has been set as counting channel")
                                  );
                game.setCountChannel(value);
            }
        }
    });

    bot.on_ready([&bot, &server, &settings](const dpp::ready_t &event) {

        dpp::slashcommand setChannel;
        setChannel.set_name("set_counting_channel");
        setChannel.set_description("set the channel in wich the counting happens");
        setChannel.set_type(dpp::ctxm_chat_input);
        std::cout<<bot.me.id<<"\n";
        setChannel.set_application_id(bot.me.id);

        //TODO replace this completly to get it from json and modify trough app commands
        std::string ref = "MEGA_bot";
        auto *admin = settings.getCommandPermission(ref);
        if (admin != nullptr) {
            std::cout<<"set admin"<<"\n";
            setChannel.add_permission(*admin);
        }
        ref = "community_manger";
        admin = settings.getCommandPermission(ref);
        if (admin != nullptr) {
            std::cout<<"set admin"<<"\n";
            setChannel.add_permission(*admin);
        }
        ref = "bass";
        admin = settings.getCommandPermission(ref);
        if (admin != nullptr) {
            std::cout<<"set admin"<<"\n";
            setChannel.add_permission(*admin);
        }

        setChannel.disable_default_permissions();
        setChannel.add_option(
                dpp::command_option(
                        dpp::co_channel, "channel", "set the channel where counting happens", true
                ));
        bot.guild_command_create(setChannel,server,[&bot](const dpp::confirmation_callback_t &callback){
            if (callback.is_error()) {
                std::cout << callback.get_error().message << "\n";
            }
        });
    });

    std::cout << "start bot" << "\n";
    bot.start(false);
    return 0;
}


