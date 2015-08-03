cmake_minimum_required(VERSION 2.8)

string(FIND ${CMAKE_BINARY_DIR} "/" pos REVERSE)
math(EXPR pos "${pos} + 1")
string(SUBSTRING ${CMAKE_BINARY_DIR} ${pos} -1 proj)
#message(${proj})
project(${proj})

include(project/${CMAKE_PROJECT_NAME}/config.cmake)
include(config/common.cmake)

if(MSVC)
    include(config/msvc.cmake)
endif()

add_subdirectory(share)
add_subdirectory(project/${CMAKE_PROJECT_NAME})
