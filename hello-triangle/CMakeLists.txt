cmake_minimum_required(VERSION 3.25)

set( CMAKE_C_COMPILER "gcc" )
set( CMAKE_CXX_COMPILER "g++" )
set(CMAKE_CXX_STANDARD 17)

project(HelloTriangle)

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(hello-triangle main.cpp instance.cpp validation.cpp device.cpp surface.cpp framebuffer.cpp imageview.cpp swapchain.cpp pipeline.cpp command.cpp)

target_include_directories(hello-triangle PRIVATE)
target_link_libraries(hello-triangle PRIVATE fmt::fmt glfw glm::glm Vulkan::Vulkan)
