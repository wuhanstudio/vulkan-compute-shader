#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>

#include "surface.h"
#include "device.h"

#include "imageview.h"

const std::vector<const char*> swapchainExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

VkSwapchainKHR createSwapChain(GLFWwindow* window, VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, std::vector<VkImage> swapChainImages, VkFormat* swapChainImageFormat, VkExtent2D* swapChainExtent);
void cleanupSwapChain(VkDevice device, VkSwapchainKHR swapChain, std::vector<VkImageView> swapChainImageViews);
