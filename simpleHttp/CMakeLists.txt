cmake_minimum_required(VERSION 3.22)
project(simpleHttp)

set(CMAKE_CXX_STANDARD 17)

add_executable(simpleHttp main.c
        server.h
        server.c)
target_link_libraries(simpleHttp pthread)