#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

VkSurfaceKHR createSurface(GLFWwindow* window, VkInstance instance);
