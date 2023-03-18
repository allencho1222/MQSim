cmake_minimum_required(VERSION 3.25)

file(GLOB_RECURSE SIM_SRCS CONFIGURE_DEPENDS *.cpp)
add_executable(mqsim ${SIM_SRCS})
target_include_directories(mqsim 
  PRIVATE src
  PRIVATE exec
  PRIVATE host
  PRIVATE nvm_chip
  PRIVATE nvm_chip/flash_memory
  PRIVATE sim
  PRIVATE ssd
  PRIVATE utils)
target_link_libraries(mqsim 
  Boost::program_options 
  fmt
  spdlog::spdlog
  yaml-cpp
)
target_compile_options(mqsim PRIVATE -Wall -O3 -g)
