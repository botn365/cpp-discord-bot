set(SQLITE_INCLUDE ${CMAKE_SOURCE_DIR}/libs/sqlite/include)

add_library(sqlite3 STATIC
       ${SQLITE_INCLUDE}/sqlite3.c
        ${SQLITE_INCLUDE}/sqlite3.h)

find_package(Threads REQUIRED)
target_link_libraries(sqlite3 INTERFACE Threads::Threads ${CMAKE_DL_LIBS})