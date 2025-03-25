cmake_minimum_required(VERSION 3.25)

set( CMAKE_C_COMPILER "gcc" )
set( CMAKE_CXX_COMPILER "g++" )
set(CMAKE_CXX_STANDARD 17)

project(HelloLBM)

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(hello-lbm app_buffer.cpp  app_command.cpp  app.cpp  app_device.cpp  app_imageviews.cpp  app_instance.cpp  app_pipeline.cpp  app_surface.cpp  app_swapchain.cpp  app_validation.cpp  main.cpp)

target_include_directories(hello-lbm PRIVATE)
target_link_libraries(hello-lbm PRIVATE fmt::fmt glfw glm::glm Vulkan::Vulkan)
