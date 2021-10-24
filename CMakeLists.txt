cmake_minimum_required(VERSION 3.20)
project(discord_bot)

set(CMAKE_CXX_STANDARD 17)

#using this slows down clion fpr me so when developing i like it turned off
option(USE_SUBDIRECTORY "include dpp trough subdirectory" ON)
option(DOWNLOAD_LIBS "if it should download libs" ON)

#download libs
if(DOWNLOAD_LIBS)
    include(FetchContent)


    #dpp
    FetchContent_Declare(
            dpp_dw
            GIT_REPOSITORY git@github.com:brainboxdotcc/DPP.git
            GIT_TAG  origin/master
    )
    FetchContent_GetProperties(dpp_dw)
    if (NOT dpp_dw_POPULATED)
        FetchContent_Populate(dpp_dw)
    endif ()

    add_subdirectory(${dpp_dw_SOURCE_DIR} ${dpp_dw_BINARY_DIR})
    set(DPP_LIB dpp)
    set(INCLUDE_DPP ${dpp_dw_SOURCE_DIR}/include)
    set(DPP_LIBS_FOLDER ${dpp_dw_BINARY_DIR})


    #rapidjson
    FetchContent_Declare(
            rapidjson_dw
            GIT_REPOSITORY git@github.com:Tencent/rapidjson.git
            GIT_TAG  origin/master
    )
    FetchContent_GetProperties(rapidjson_dw)
    if (NOT rapidjson_dw_POPULATED)
        FetchContent_Populate(rapidjson_dw)
    endif ()
    set(INCLUDE_RAPID_JSON ${rapidjson_dw_SOURCE_DIR}/include)


    #sqlite
    set(CMAKE_TLS_VERIFY true)
    FetchContent_Declare(
            sqlite_dw
            URL https://www.sqlite.org/2021/sqlite-amalgamation-3360000.zip
            URL_HASH SHA3_256=d25609210ec93b3c8c7da66a03cf82e2c9868cfbd2d7d866982861855e96f972
    )
    FetchContent_GetProperties(sqlite_dw)
    if (NOT sqlite_dw_POPULATED)
        FetchContent_Populate(sqlite_dw)
    endif ()
    set(SQLITE_INCLUDE ${sqlite_dw_SOURCE_DIR})
    include(${CMAKE_SOURCE_DIR}/cmake/addSqlite3.cmake)
    set(SQLITE_LIB sqlite3)
endif()



if(NOT DOWNLOAD_LIBS)
    set(INCLUDE_RAPID_JSON libs/rapidjson/include)

    #handel dpp
    set(INCLUDE_DPP libs/DPP/include)
    if (USE_SUBDIRECTORY)
        add_subdirectory(libs/DPP REQUIRED)
        set(DPP_LIB dpp)
    else()
        string(TOLOWER ${CMAKE_BUILD_TYPE} LOWER_MODE)
        set(BUILD_FOLDER_CLION cmake-build-${LOWER_MODE})
        set(DPP_LIBS_FOLDER libs/DPP/${BUILD_FOLDER_CLION})
        link_directories(${DPP_LIBS_FOLDER})
        if (UNIX)
            set(DPP_LIB libdpp.so)
        else()
            set(DPP_LIB dpp.lib)
        endif()
    endif()

    #handel sqlite
    set(SQLITE_INCLUDE ${CMAKE_SOURCE_DIR}/libs/sqlite/include)
    include(${CMAKE_SOURCE_DIR}/cmake/addSqlite3.cmake)
    set(SQLITE_LIB sqlite3)
endif()



file(GLOB SRC CONFIGURE_DEPENDS "include/*.hpp" "src/*.cpp")

add_executable(${PROJECT_NAME}
        ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DPP}
        ${INCLUDE_RAPID_JSON}
        ${SQLITE_INCLUDE})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${DPP_LIB}
        ${SQLITE_LIB})

if (NOT UNIX)
    if(USE_SUBDIRECTORY OR DOWNLOAD_LIBS)
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${DPP_LIBS_FOLDER}/dpp.dll
                ${CMAKE_CURRENT_BINARY_DIR}/dpp.dll
        )
    else()
        configure_file("${DPP_LIBS_FOLDER}/dpp.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
    endif()
    configure_file("${DPP_LIBS_FOLDER}/zlib1.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
    configure_file("${DPP_LIBS_FOLDER}/libcrypto-1_1-x64.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
    configure_file("${DPP_LIBS_FOLDER}/libssl-1_1-x64.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
    configure_file("${DPP_LIBS_FOLDER}/libsodium.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
    configure_file("${DPP_LIBS_FOLDER}/opus.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
endif()

