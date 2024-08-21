cmake_minimum_required(VERSION 3.22)
project(reactor_c)

set(CMAKE_CXX_STANDARD 17)

add_executable(reactor_c main.c
        server.h
        server.c)
target_link_libraries(reactor_c pthread)