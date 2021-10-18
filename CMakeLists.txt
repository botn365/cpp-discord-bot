cmake_minimum_required(VERSION 3.20)
project(discord_bot)

set(CMAKE_CXX_STANDARD 17)

string(TOLOWER ${CMAKE_BUILD_TYPE} LOWER_MODE)

set(BUILD_FOLDER_CLION cmake-build-${LOWER_MODE})
set(DPP_LIBS_FOLDER libs/DPP/${BUILD_FOLDER_CLION})
link_directories(${DPP_LIBS_FOLDER})

set(SQLITE_LINK_DIR libs/sqlite/lib)
link_directories (${SQLITE_LINK_DIR})

file(GLOB SRC CONFIGURE_DEPENDS "include/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME}
        ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
        libs/DPP/include
        libs/rapidjson/include
        libs/sqlite/include)

target_link_libraries(${PROJECT_NAME}
        dpp.lib
        sqlite3.dll)


configure_file("${DPP_LIBS_FOLDER}/dpp.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
configure_file("${DPP_LIBS_FOLDER}/zlib1.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
configure_file("${DPP_LIBS_FOLDER}/libcrypto-1_1-x64.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
configure_file("${DPP_LIBS_FOLDER}/libssl-1_1-x64.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
configure_file("${DPP_LIBS_FOLDER}/libsodium.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
configure_file("${DPP_LIBS_FOLDER}/opus.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
configure_file("${SQLITE_LINK_DIR}/sqlite3.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)

