cmake_minimum_required(VERSION 3.14)
project(try_opencl)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

#find_package(X11 REQUIRED)
#link_libraries(${X11_LIBRARIES})
#include_directories(${X11_INCLUDE_DIR})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
#include_directories("/usr/local/Cellar/cimg/2.5.4/include/")

set (opencl_example_src main.cpp engine.cpp engine.h renderer.cpp renderer.h timer.cpp timer.h png_wrapper.cpp png_wrapper.h)

add_executable(try_opencl ${opencl_example_src})
target_link_libraries(try_opencl "-framework OpenCL")
target_link_libraries(try_opencl ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
target_link_libraries(try_opencl ${PNG_LIBRARY})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/experimental.cl ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bobmarley512g.png ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
