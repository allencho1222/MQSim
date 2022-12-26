cmake_minimum_required(VERSION 3.12)
project(MQSim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# print SPDLOG_TRACE
add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

include(build/conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
