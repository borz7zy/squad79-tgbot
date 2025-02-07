cmake_minimum_required(VERSION 3.19)
project(squad79-tgbot)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system)
find_package(TgBot REQUIRED)

set(SQLITE3_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/sqlite3.c)
set(SQLITE3_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/sqlite3.h)
add_library(sqlite3 STATIC ${SQLITE3_SRCS})
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite3)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/escapeMarkdownV2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bot/core.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE TgBot::TgBot sqlite3)
