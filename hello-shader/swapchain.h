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

SwapChainSupportDetails vk_query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR vk_surface);

void vk_create_swapchain(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    VkSurfaceKHR vk_surface, GLFWwindow* window
);

void vk_cleanup_swap_chain(VkDevice vk_device);

void vk_recreate_swapchain(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    VkSurfaceKHR vk_surface, GLFWwindow* window
);
