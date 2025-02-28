#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "instance.h"

extern VkSurfaceKHR surface;

void createSurface(GLFWwindow* window);
