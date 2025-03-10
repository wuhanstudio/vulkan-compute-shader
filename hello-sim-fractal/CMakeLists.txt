cmake_minimum_required(VERSION 3.25)

set( CMAKE_C_COMPILER "gcc" )
set( CMAKE_CXX_COMPILER "g++" )

project(HelloFractal)

find_package(Vulkan REQUIRED)

add_executable(hello-fractal main.cpp compute.c  device.c  instance.c memory.c  pipeline.c)

target_include_directories(hello-fractal PRIVATE)
target_link_libraries(hello-fractal PRIVATE Vulkan::Vulkan)
