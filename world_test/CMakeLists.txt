cmake_minimum_required(VERSION 3.8)
project(World)

set(CMAKE_CXX_STANDARD 14)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wextra -Wall -pedantic")
endif()

set(SOURCE_FILES main.cpp)
add_executable(World ${SOURCE_FILES})
