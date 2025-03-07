cmake_minimum_required(VERSION 3.25)

set( CMAKE_C_COMPILER "gcc" )
set( CMAKE_CXX_COMPILER "g++" )

project(HelloValidation)

find_package(fmt CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(hello-validation main.cpp)

target_include_directories(hello-validation PRIVATE)
target_link_libraries(hello-validation PRIVATE fmt::fmt Vulkan::Vulkan)
