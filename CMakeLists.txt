cmake_minimum_required(VERSION 2.8)
project(Brandes)
set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -O3 -pthread")
add_executable(brandes brandes.cpp)
