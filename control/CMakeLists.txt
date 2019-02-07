cmake_minimum_required(VERSION 3.0)

project(TOMCONTROL)

FILE(GLOB SRC src/*.cpp)

add_library(tomcontrol SHARED ${SRC})
