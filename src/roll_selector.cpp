//
// Created by bot on 06.11.21.
//

#include "../include/roll_selector.hpp"
//#include "app.cpp"

namespace Bot {
    RollSelector::Emote::Emote(std::string str) {
        auto pos1 = str.find_first_of('<');
        if (pos1 == std::string::npos) {
            id = 0;
            name = str;
            return;
        }
        pos1 += 2;
        auto pos2 = str.find_first_of(':', pos1);
        auto pos3 = str.find_first_of('>');
        name = str.substr(pos1, pos2 - pos1);
        pos2++;
        id = std::strtoull(str.substr(pos2, pos3 - pos2).c_str(), nullptr, 10);
    }

    std::string RollSelector::Emote::getStringId() const {
        if (id ==(uint64_t) 0) {
            return name;
        } else {
            return name + ":" + std::to_string(id);
        }
    }

    bool RollSelector::Emote::operator==(const RollSelector::Emote &emote) const {
        if (this->id ==(uint64_t) 0) {
            return emote.id ==(uint64_t) 0 && this->name == emote.name;
        } else {
            return this->id == emote.id;
        }
    }

    bool RollSelector::Emote::operator==(const dpp::emoji emote) const {
        if (this->id ==(uint64_t) 0) {
            return this->name == emote.name;
        } else {
            return this->id == emote.id;
        }
    }


    RollSelector::RollSelector(ServerSettings &settings) {
        auto &doc = settings.getDocument();
        if (doc.HasMember("roll_settings")) {
            for (auto &value: doc["roll_settings"].GetArray()) {
                rolls.emplace_back(Roll{value["msgID"].GetUint64(), value["failRollID"].GetUint64(),
                                        Emote(value["emoteID"].GetUint64(), value["emoteName"].GetString())});
            }
        } else {
            rapidjson::Value rollSettings;
            rollSettings.SetArray();
            doc.AddMember("roll_settings", rollSettings, doc.GetAllocator());
        }
    }

    void RollSelector::addSettings(dpp::slashcommand &baseCommand, App *app) {
//        dpp::command_option command(dpp::co_sub_command_group, "reaction_roll", "controll reaction rolls");
//        {
//            dpp::command_option add(dpp::co_sub_command, "add", "add a reaction roll");
//            dpp::command_option rollID(dpp::co_role, "roll", "roll to bind", true);
//            dpp::command_option msgID(dpp::co_string, "msg_id", "id of the message where reaction take place", true);
//            dpp::command_option emote(dpp::co_string, "emoji", "emoji the roll is bind to", true);
//            add.add_option(rollID);
//            add.add_option(msgID);
//            add.add_option(emote);
//            command.add_option(add);
//            app->registerSetting(baseCommand, command, [app, this](const dpp::interaction_create_t &interaction) {
//                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
//                auto &int_data = cmd_data.options[0].options[0];
//                Roll roll;
//                try {
//                    roll.messageId = std::stoull(std::get<std::string>(int_data.options[1].value));
//                    roll.emote = Emote(std::get<std::string>(int_data.options[2].value));
//                    roll.rollId = std::get<dpp::snowflake>(int_data.options[0].value);
//                } catch (std::exception &e) {
//                    std::string what = e.what();
//                    std::cout << what;
//                    return;
//                }
//
//                for (auto &iRoll: rolls) {
//                    if (iRoll.messageId == roll.messageId && iRoll.emote == roll.emote) {
//                        interaction.reply(dpp::ir_channel_message_with_source, dpp::message()
//                                .set_type(dpp::mt_reply)
//                                .set_flags(dpp::m_ephemeral)
//                                .set_content("reaction roll has already been bound"));
//                        return;
//                    }
//                }
//
//                rolls.push_back(roll);
//
//                app->bot->message_add_reaction(roll.messageId, interaction.command.channel_id,
//                                               roll.emote.getStringId());
//
//                auto &doc = app->getGlobalSettings().getDocument();
//                auto &rollSettings = doc["roll_settings"];
//                rapidjson::Value newRoll;
//                newRoll.SetObject();
//                rapidjson::Value msgID;
//                msgID.SetUint64(roll.messageId);
//                rapidjson::Value rollID;
//                rollID.SetUint64(roll.rollId);
//                rapidjson::Value emoteID;
//                emoteID.SetUint64(roll.emote.id);
//                rapidjson::Value emoteName;
//                emoteName.SetString(roll.emote.name.c_str(), doc.GetAllocator());
//                newRoll.AddMember("msgID", msgID, doc.GetAllocator());
//                newRoll.AddMember("failRollID", rollID, doc.GetAllocator());
//                newRoll.AddMember("emoteID", emoteID, doc.GetAllocator());
//                newRoll.AddMember("emoteName", emoteName, doc.GetAllocator());
//                rollSettings.PushBack(newRoll, doc.GetAllocator());
//
//                app->getGlobalSettings().save();
//
//                interaction.reply(dpp::ir_channel_message_with_source, dpp::message()
//                        .set_type(dpp::mt_reply)
//                        .set_flags(dpp::m_ephemeral)
//                        .set_content("reaction roll added"));
//            });
//        }
//        {
//            dpp::command_option remove(dpp::co_sub_command, "remove", "remove a reaction roll");
//            dpp::command_option msgID(dpp::co_string, "msg_id", "id of the message where reaction take place", true);
//            dpp::command_option emote(dpp::co_string, "emoji", "emoji the roll is bind to", true);
//            remove.add_option(msgID);
//            remove.add_option(emote);
//            command.add_option(remove);
//            app->registerSetting(baseCommand, command, [this, app](const dpp::interaction_create_t &interaction) {
//                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
//                auto &int_data = cmd_data.options[0].options[0];
//                dpp::snowflake messageID = std::stoull(std::get<std::string>(int_data.options[0].value));
//                Emote emote(std::get<std::string>(int_data.options[1].value));
//
//                for (auto roll = rolls.begin(); roll != rolls.end();) {
//                    if (roll->messageId == messageID && roll->emote == emote) {
//                        removeAllRollsOfReaction(app,interaction.command.channel_id,*roll);
//                        roll = rolls.erase(roll);
//                    } else {
//                        roll++;
//                    }
//                }
//
//                auto &doc = app->getGlobalSettings().getDocument();
//                auto &rollSettings = doc["roll_settings"];
//
//                for (auto value = rollSettings.Begin(); value != rollSettings.End();) {
//                    auto &dvalue = *value;
//                    if (dvalue["msgID"].GetUint64() == messageID) {
//                        Emote jEmote(dvalue["emoteID"].GetUint64(), dvalue["emoteName"].GetString());
//                        if (jEmote ==emote) {
//                            value = rollSettings.Erase(value);
//                            continue;
//                        }
//                    }
//                    value++;
//                }
//
//                app->getGlobalSettings().save();
//
//                interaction.reply(dpp::ir_channel_message_with_source, dpp::message()
//                        .set_type(dpp::mt_reply)
//                        .set_flags(dpp::m_ephemeral)
//                        .set_content("reaction roll removed"));
//            }, 1);
//        }
//        baseCommand.add_option(command);
//    }
//
//    void RollSelector::removeAllRollsOfReaction(App *app,dpp::snowflake channelID,Roll &roll) {
//        dpp::snowflake rollID = roll.rollId;
//        auto msgID = roll.messageId;
//        auto emote = roll.emote.getStringId();
//        app->bot->message_get_reactions(roll.messageId,channelID,emote,0,0,0,[app,rollID,msgID,channelID,emote](const dpp::confirmation_callback_t &callback){
//            if (callback.is_error()) {
//                auto err = callback.get_error();
//                std::cout<<err.message<<"\n";
//                return;
//            }
//            auto &users = std::get<dpp::user_map>(callback.value);
//            for (auto &user : users) {
//                //app->bot->guild_member_delete_role(app->getGlobalSettings().getServerId(), user.second.id, rollID);
//            }
//            app->bot->message_delete_reaction_emoji(msgID, channelID,emote);
//        });
//    }
//
//    void RollSelector::onMessageReactionAdd(App *app, const dpp::message_reaction_add_t &event) {
//        for (auto &roll: rolls) {
//            if (roll.messageId == event.message_id && roll.emote == event.reacting_emoji) {
//                auto userID = event.reacting_member.user_id;
//                app->bot->guild_get_member(event.reacting_member.guild_id, event.reacting_member.user_id,
//                                           [app, this, roll, userID](const dpp::confirmation_callback_t &event) {
//                                               auto &user = std::get<dpp::guild_member>(event.value);
//                                               for (auto &guild_roll: user.roles) {
//                                                   for (auto &iRoll: rolls) {
//                                                       if (guild_roll == iRoll.rollId && guild_roll != roll.rollId) {
//                                                           return;
//                                                       }
//                                                   }
//                                               }
//                                               //app->bot->guild_member_add_role(app->getGlobalSettings().getServerId(), userID,
//                                                                               //roll.rollId);
//                                           });
//                break;
//            }
//        }
//    }
//
//    void RollSelector::onMessageReactionRemove(App *app, const dpp::message_reaction_remove_t &event) {
//        rapidjson::Document doc;
//        doc.Parse(event.raw_event.c_str());
//        auto &data = doc["d"];
//        dpp::snowflake userID = std::strtoull(data["user_id"].GetString(), nullptr, 10);
//        for (auto &roll: rolls) {
//            if (roll.messageId == event.message_id && roll.emote == event.reacting_emoji) {
//                //app->bot->guild_member_delete_role(app->getGlobalSettings().getServerId(), userID, roll.rollId);
//                break;
//            }
//        }
    }
}