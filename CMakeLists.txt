cmake_minimum_required(VERSION 3.16)
project(oven CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(oven
  src/main.cpp
  src/core/StateMachine.cpp

)
