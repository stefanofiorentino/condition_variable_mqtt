cmake_minimum_required(VERSION 3.14)
project(condition_variable_mqtt)

set(CMAKE_CXX_STANDARD 14)

add_executable(condition_variable_mqtt main.cpp mqtt.cpp)
target_link_libraries(condition_variable_mqtt pthread mosquittopp)
