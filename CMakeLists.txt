cmake_minimum_required(VERSION 3.30)
project(projekt1_sockets C)

set(CMAKE_C_STANDARD 11)

add_executable(projekt1_sockets main.c
        client.c
        server.c)
