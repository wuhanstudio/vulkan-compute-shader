#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>

#include "surface.h"
#include "device.h"

#include "imageview.h"
#include "framebuffer.h"

extern VkSwapchainKHR swapChain;

extern VkExtent2D swapChainExtent;
extern VkFormat swapChainImageFormat;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void createSwapChain(GLFWwindow* window);
void cleanupSwapChain();

void createImageViews();
