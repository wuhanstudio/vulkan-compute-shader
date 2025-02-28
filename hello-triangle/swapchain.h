#pragma once
#include "surface.h"
#include "device.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>

extern VkSwapchainKHR swapChain;

extern std::vector<VkImage> swapChainImages;
extern VkFormat swapChainImageFormat;

extern VkExtent2D swapChainExtent;

extern std::vector<VkImageView> swapChainImageViews;
extern std::vector<VkFramebuffer> swapChainFramebuffers;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void createSwapChain(GLFWwindow* window);
void cleanupSwapChain();

void createImageViews();
