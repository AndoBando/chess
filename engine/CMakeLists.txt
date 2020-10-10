cmake_minimum_required(VERSION 3.16)
project(chessengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -s -O3")
add_executable(chessengine main.cpp board.cpp board.h legalmoves.cpp legalmoves.h)