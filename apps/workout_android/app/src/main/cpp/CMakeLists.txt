cmake_minimum_required(VERSION 3.22.1)
project(workout_android_native LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WORKOUT_CORE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../workout_core/src)
set(WORKOUT_IO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../workout_io/src)
set(CJSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson)
set(SQLITE_VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite)

function(_ensure_sqlite3_target)
    if(EXISTS "${SQLITE_VENDOR_DIR}/sqlite3.c" AND EXISTS "${SQLITE_VENDOR_DIR}/sqlite3.h")
        message(STATUS "Android: using vendored sqlite amalgamation.")
        add_library(workout_sqlite3 STATIC
            "${SQLITE_VENDOR_DIR}/sqlite3.c"
        )
        target_include_directories(workout_sqlite3 PUBLIC
            "${SQLITE_VENDOR_DIR}"
        )
        target_compile_definitions(workout_sqlite3 PUBLIC
            SQLITE_THREADSAFE=1
            SQLITE_OMIT_LOAD_EXTENSION
        )

        add_library(SQLite::SQLite3 ALIAS workout_sqlite3)
        add_library(SQLite3 ALIAS workout_sqlite3)
        return()
    endif()

    if(TARGET SQLite::SQLite3 OR TARGET SQLite3)
        if(TARGET SQLite::SQLite3 AND NOT TARGET SQLite3)
            add_library(SQLite3 INTERFACE IMPORTED)
            target_link_libraries(SQLite3 INTERFACE SQLite::SQLite3)
        endif()
        return()
    endif()

    find_package(SQLite3 QUIET)
    if(TARGET SQLite::SQLite3 OR TARGET SQLite3)
        if(TARGET SQLite::SQLite3 AND NOT TARGET SQLite3)
            add_library(SQLite3 INTERFACE IMPORTED)
            target_link_libraries(SQLite3 INTERFACE SQLite::SQLite3)
        endif()
        return()
    endif()

    message(STATUS "Android: fetching sqlite amalgamation for workout_android.")
    FetchContent_Declare(
        workout_sqlite_amalgamation
        URL "https://www.sqlite.org/2026/sqlite-amalgamation-3510200.zip"
        URL_HASH "SHA256=6E2A845A493026BDBAD0618B2B5A0CF48584FAAB47384480ED9F592D912F23EC"
    )
    FetchContent_MakeAvailable(workout_sqlite_amalgamation)

    add_library(workout_sqlite3 STATIC
        "${workout_sqlite_amalgamation_SOURCE_DIR}/sqlite3.c"
    )
    target_include_directories(workout_sqlite3 PUBLIC
        "${workout_sqlite_amalgamation_SOURCE_DIR}"
    )
    target_compile_definitions(workout_sqlite3 PUBLIC
        SQLITE_THREADSAFE=1
        SQLITE_OMIT_LOAD_EXTENSION
    )

    add_library(SQLite::SQLite3 ALIAS workout_sqlite3)
    add_library(SQLite3 ALIAS workout_sqlite3)
endfunction()

_ensure_sqlite3_target()

file(GLOB_RECURSE WORKOUT_CORE_SOURCES
    "${WORKOUT_CORE_SRC_DIR}/*.cpp"
)

file(GLOB_RECURSE WORKOUT_IO_SOURCES
    "${WORKOUT_IO_SRC_DIR}/*.cpp"
)

add_library(
    workout_android
    SHARED
    jni/workout_android_bridge.cpp
    ${WORKOUT_CORE_SOURCES}
    ${WORKOUT_IO_SOURCES}
    ${CJSON_DIR}/cJSON.c
)

target_include_directories(
    workout_android
    PRIVATE
    ${WORKOUT_CORE_SRC_DIR}
    ${WORKOUT_IO_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

find_library(log-lib log)
if(NOT log-lib)
    message(FATAL_ERROR "Android log library not found.")
endif()

target_link_libraries(
    workout_android
    PRIVATE
    ${log-lib}
    SQLite3
)
