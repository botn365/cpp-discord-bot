# cpp-discord-bot


# compile

## with auto download (default)

### linux

run cmake on project

make

### windows

open project in visual studio as cmake project

build

## without auto download

create directory libs

get rapidjson

libs/rapidjson/include/rapidjson

get sqlite3 amalgamation source

put it in libs/sqlite/include

dpp
https://github.com/brainboxdotcc/DPP

if you turn off USE_SUBDIRECTORY then it needs to be compiled locally and 

set DPP_LIBS_FOLDER to where dpp puts compile output default is when compiled using clion

# run

all configuration is done with the settings.json file see settings.example.json

this file needs to be in the same directory where the bot is run from

both discord token and server id need to be filled in other options have a default value
