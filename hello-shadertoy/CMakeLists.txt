cmake_minimum_required(VERSION 3.25)

set( CMAKE_C_COMPILER "gcc" )
set( CMAKE_CXX_COMPILER "g++" )
set(CMAKE_CXX_STANDARD 17)

project(HelloShaderToy)

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(hello-shadertoy main.cpp instance.cpp validation.cpp device.cpp surface.cpp framebuffer.cpp imageview.cpp swapchain.cpp pipeline.cpp command.cpp vertex.cpp memory.cpp)

target_include_directories(hello-shadertoy PRIVATE)
target_link_libraries(hello-shadertoy PRIVATE fmt::fmt glfw glm::glm Vulkan::Vulkan)
