cmake_minimum_required(VERSION 3.22)
project(reactorHttpC)
set(CMAKE_CXX_STANDARD 17)
add_executable(reactorHttpC 
                main.c
                Channel.h
                Channel.c
                )
target_link_libraries(reactorHttpC pthread)