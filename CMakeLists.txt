cmake_minimum_required(VERSION 3.6)
project(parse)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    compiler.c
    compiler.h parse.cpp)

add_executable(parse ${SOURCE_FILES})