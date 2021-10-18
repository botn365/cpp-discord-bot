# cpp-discord-bot


#compile

create directory libs

get rapidjson

libs/rapidjson/include/rapidjson

get sqlite3

libs/sqlite/include libs/sqlite/lib

on windows you need to compile the sqlite3.def file

dpp
https://github.com/brainboxdotcc/DPP

needs to be compiled locally

set DPP_LIBS_FOLDER to where dpp puts compile output default is when compiled using clion

#run

all configuration is done with the settings.json file see settings.example.json

this file needs to be in the same directory where the bot is run from

both discord token and server id need to be filled in other options have a default value
