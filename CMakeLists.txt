cmake_minimum_required(VERSION 3.21)
project(second_try C)

set(CMAKE_C_STANDARD 11)


find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(second_try
        game.c
        linked_list.c
        server_utils.c
        client.c
        npc_client.c
        beast_client.c)

target_link_libraries(second_try ${CURSES_LIBRARIES})
