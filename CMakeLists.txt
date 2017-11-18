cmake_minimum_required(VERSION 3.0)
project(iniparser)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/iniparser.cpp src/iniparser.h)
add_library(iniparser ${SOURCE_FILES})

target_link_libraries(iniparser toolboxpp)


add_executable(dev main.cpp)
target_link_libraries(dev iniparser)