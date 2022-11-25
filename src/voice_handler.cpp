//
// Created by bot on 05.01.22.
//

#include "../include/voice_handler.hpp"
//#include "app.cpp"


Bot::VoiceHandler::VoiceHandler(Bot::App &app) : app(app) {
//    auto &doc = app.getGlobalSettings().getDocument();
//    if (doc.HasMember("voice_handler")) {
//        auto &voice = doc["voice_handler"];
//        voiceRollId = voice["roll_id"].GetInt64();
//        AFKVoiceChannel = voice["AFK_channel"].GetInt64();
//    } else {
//        rapidjson::Value voice;
//        voice.SetObject();
//        rapidjson::Value rollId;
//        rollId.SetUint64(0);
//        voice.AddMember("roll_id",rollId,doc.GetAllocator());
//        rapidjson::Value AFK;
//        AFK.SetUint64(0);
//        voice.AddMember("AFK_channel",AFK,doc.GetAllocator());
//        doc.AddMember("voice_handler", voice, doc.GetAllocator());
//        voiceRollId = 0;
//        AFKVoiceChannel = 0;
//        app.getGlobalSettings().save();
//    }
}

void Bot::VoiceHandler::onVoiceStateUpdate(const dpp::voice_state_update_t &event) {
//    if (voiceRollId.operator==(0)) return;
//    auto channel = event.state.channel_id;
//    if (channel.operator==(0) || channel == AFKVoiceChannel) {
//        app.bot->guild_member_delete_role(event.state.guild_id,event.state.user_id,voiceRollId);
//    } else {
//        app.bot->guild_member_add_role(event.state.guild_id,event.state.user_id,voiceRollId);
//    }
}

void Bot::VoiceHandler::addSettings(dpp::slashcommand &baseCommand) {
//    dpp::command_option command(dpp::co_sub_command_group, "voice", "voice settings");
//    {
//        dpp::command_option setVoiceRoll(dpp::co_sub_command,"set_voice_roll",
//                                         "set wich roll to give when joining voice");
//        setVoiceRoll.add_option(
//                dpp::command_option(
//                        dpp::co_role,"voice_roll","leave empty to have no roll"
//                        ));
//        command.add_option(setVoiceRoll);
//        app.registerSetting(baseCommand,command,[this](const dpp::interaction_create_t &interaction){
//            dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
//            auto &data = cmd_data.options[0].options[0];
//            if (!data.options.empty()) {
//                voiceRollId = std::get<dpp::snowflake>(data.options[0].value);
//                interaction.reply(dpp::ir_channel_message_with_source,
//                                  dpp::message()
//                                          .set_type(dpp::mt_reply)
//                                          .set_flags(dpp::m_ephemeral)
//                                          .set_content(
//                                                  "<@" + std::to_string(voiceRollId) + "> has been set as voice roll"));
//
//            } else {
//                voiceRollId = 0;
//                interaction.reply(dpp::ir_channel_message_with_source,
//                                  dpp::message()
//                                          .set_type(dpp::mt_reply)
//                                          .set_flags(dpp::m_ephemeral)
//                                          .set_content(
//                                                  "voice roll has been reset"));
//            }
//            auto &doc = app.getGlobalSettings().getDocument();
//            auto &voice = doc["voice_handler"];
//            voice["roll_id"].SetUint64(voiceRollId);
//            app.getGlobalSettings().save();
//        },0);
//    }
//
//    {
//        dpp::command_option setAFKChannel(dpp::co_sub_command,"set_afk_channel","set which is the afk channel");
//
//        setAFKChannel.add_option(
//                dpp::command_option(
//                        dpp::co_channel,"afk_channel","leave empty to have no AFK channel"
//                ));
//        command.add_option(setAFKChannel);
//        app.registerSetting(baseCommand,command,[this](const dpp::interaction_create_t &interaction){
//            dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
//            auto &data = cmd_data.options[0].options[0];
//            if (!data.options.empty()) {
//                AFKVoiceChannel = std::get<dpp::snowflake>(data.options[0].value);
//                interaction.reply(dpp::ir_channel_message_with_source,
//                                  dpp::message()
//                                          .set_type(dpp::mt_reply)
//                                          .set_flags(dpp::m_ephemeral)
//                                          .set_content(
//                                                  "<#" + std::to_string(AFKVoiceChannel) + "> has been set as afk channel"));
//            } else {
//                AFKVoiceChannel = 0;
//                interaction.reply(dpp::ir_channel_message_with_source,
//                                  dpp::message()
//                                          .set_type(dpp::mt_reply)
//                                          .set_flags(dpp::m_ephemeral)
//                                          .set_content(
//                                                  "afk channel has been reset"));
//            }
//            auto &doc = app.getGlobalSettings().getDocument();
//            auto &voice = doc["voice_handler"];
//            voice["AFK_channel"].SetUint64(AFKVoiceChannel);
//            app.getGlobalSettings().save();
//        },1);
//    }
//    baseCommand.add_option(command);
}