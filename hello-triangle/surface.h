#pragma once

#include "instance.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

extern VkSurfaceKHR surface;

void createSurface(GLFWwindow* window);
