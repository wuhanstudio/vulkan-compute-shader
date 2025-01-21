cmake_minimum_required(VERSION 3.25)

set( CMAKE_C_COMPILER "gcc" )
set( CMAKE_CXX_COMPILER "g++" )

project(HelloFractal)

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(hello-fractal main.cpp)

target_include_directories(hello-fractal PRIVATE)
target_link_libraries(hello-ftactal PRIVATE fmt::fmt glfw glm::glm Vulkan::Vulkan)
