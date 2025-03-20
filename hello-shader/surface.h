#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "instance.h"

VkSurfaceKHR vk_create_surface(VkInstance vk_instance, GLFWwindow* window);
