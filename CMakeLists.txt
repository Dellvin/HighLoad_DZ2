cmake_minimum_required(VERSION 3.15)
project(DellvinX)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS -lstdc++fs -pthread -std=c++17)

add_executable(DellvinX main.cpp
        server/Server.cpp server/Server.h conf/config.h
        )
if(WIN32)
    target_link_libraries(DellvinX ws2_32)
endif()
