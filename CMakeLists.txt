cmake_minimum_required(VERSION 3.14)
project(try_opencl)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

set (opencl_example_src main.cpp)

add_executable(try_opencl ${opencl_example_src})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/experimental.cl ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
target_link_libraries(try_opencl "-framework OpenCL")
