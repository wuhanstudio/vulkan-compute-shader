#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

VkSurfaceKHR vk_create_surface(GLFWwindow* window, VkInstance instance);
