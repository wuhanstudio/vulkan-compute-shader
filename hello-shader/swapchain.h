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

void vk_create_swapchain(VkSurfaceKHR vk_surface, GLFWwindow* window);
void cleanupSwapChain();

void createImageViews();
void vk_recreate_swapchain(VkSurfaceKHR vk_surface, GLFWwindow* window);
