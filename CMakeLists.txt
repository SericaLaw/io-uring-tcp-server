cmake_minimum_required(VERSION 3.16)
project(linux_io_examples)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)
include_directories(lib)

add_executable(echo_uring.out echo_uring.cpp)
target_link_libraries(echo_uring.out uring_server)