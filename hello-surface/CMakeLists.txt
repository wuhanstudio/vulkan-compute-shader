cmake_minimum_required(VERSION 3.25)

set( CMAKE_C_COMPILER "gcc" )
set( CMAKE_CXX_COMPILER "g++" )

project(HelloSurface)

find_package(fmt CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(hello-surface main.cpp validation.cpp instance.cpp surface.cpp device.cpp swapchain.cpp imageview.cpp)

target_include_directories(hello-surface PRIVATE)
target_link_libraries(hello-surface PRIVATE fmt::fmt glfw Vulkan::Vulkan)
