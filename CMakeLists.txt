cmake_minimum_required(VERSION 3.25)
project(MQSim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# print SPDLOG_TRACE
add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
