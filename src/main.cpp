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

    uint32_t intents = 0;
    intents += dpp::i_guild_integrations;
    intents += dpp::i_guild_messages;
    intents += dpp::i_guild_message_reactions;
    intents += dpp::i_guild_message_typing;

    dpp::cluster bot(settings.getToken(), intents);
    Bot::CountingGame game(settings.getCountDBLocation());

    bot.on_message_create([&bot,&game](const dpp::message_create_t &message) {
        if (message.msg->author->id == bot.me.id) return;
        game.count(bot,message);
    });

    bot.on_interaction_create([&bot,&game](const dpp::interaction_create_t &interaction) {
        game.onInteraction(interaction);
    });

    bot.on_ready([&bot, &settings, &game](const dpp::ready_t &event) {
        game.addCommands(bot,settings);
        std::cout << "bot ready" << "\n";
    });

    std::cout << "start bot" << "\n";
    bot.start(false);
    return 0;
}


