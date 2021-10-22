cmake_minimum_required(VERSION 3.20)
project(discord_bot)

set(CMAKE_CXX_STANDARD 17)

#using this slows down clion fpr me so when developing i like it turned off
option(USE_SUBDIRECTORY "include dpp trough subdirectory" ON)
option(DOWNLOAD_LIBS "if it should download libs" ON)

#download libs
if(DOWNLOAD_LIBS)
    include(FetchContent)

    FetchContent_Declare(
            DPP_DW
            GIT_REPOSITORY git@github.com:brainboxdotcc/DPP.git
            GIT_TAG  origin/master
    )
    FetchContent_GetProperties(DPP_DW)
    if (NOT DPP_DW_POPULATED)
        FetchContent_Populate(DPP_DW)
    endif ()

    add_subdirectory(${dpp_dw_SOURCE_DIR} ${dpp_dw_BINARY_DIR})
    set(DPP_LIB dpp)
    set(INCLUDE_DPP ${dpp_dw_SOURCE_DIR}/include)
endif()




#handel dpp
if(NOT DOWNLOAD_LIBS)
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
endif()

    #handel sqlite
    set(SQLITE_LINK_DIR libs/sqlite/lib)
    if (UNIX)
        include(${CMAKE_SOURCE_DIR}/cmake/addSqlite3.cmake)
        set(SQLITE_LIB sqlite3)
    else()
        set(SQLITE_LIB sqlite3.dll)
        link_directories (${SQLITE_LINK_DIR})
    endif()
#endif()



file(GLOB SRC CONFIGURE_DEPENDS "include/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME}
        ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DPP}
        libs/rapidjson/include
        libs/sqlite/include)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${DPP_LIB}
        ${SQLITE_LIB})

if (NOT UNIX)
    if (NOT ${USE_SUBDIRECTORY})
        configure_file("${DPP_LIBS_FOLDER}/dpp.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
        configure_file("${DPP_LIBS_FOLDER}/zlib1.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
        configure_file("${DPP_LIBS_FOLDER}/libcrypto-1_1-x64.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
        configure_file("${DPP_LIBS_FOLDER}/libssl-1_1-x64.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
        configure_file("${DPP_LIBS_FOLDER}/libsodium.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
        configure_file("${DPP_LIBS_FOLDER}/opus.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
        configure_file("${SQLITE_LINK_DIR}/sqlite3.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
    endif()
endif()

