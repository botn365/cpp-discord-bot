add_library(sqlite3 STATIC
       ${SQLITE_INCLUDE}/sqlite3.c
        ${SQLITE_INCLUDE}/sqlite3.h)

if (UNIX)
    find_package(Threads REQUIRED)
endif()
target_link_libraries(sqlite3 INTERFACE Threads::Threads ${CMAKE_DL_LIBS})