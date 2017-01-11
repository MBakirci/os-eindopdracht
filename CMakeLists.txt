cmake_minimum_required(VERSION 2.8.4)
project(os_eindopdracht)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    CGI/main.c
    daemon/main.c)

add_executable(os_eindopdracht ${SOURCE_FILES})