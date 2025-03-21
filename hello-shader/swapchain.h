#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>

#include "surface.h"
#include "device.h"

#include "imageview.h"
#include "framebuffer.h"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails vk_query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR vk_surface);

VkSwapchainKHR vk_create_swapchain(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    VkSurfaceKHR vk_surface, GLFWwindow* window
);

VkExtent2D vk_choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

void vk_cleanup_swap_chain(VkDevice vk_device, VkSwapchainKHR vk_swapchain);

void vk_recreate_swapchain(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    VkSurfaceKHR vk_surface, GLFWwindow* window,
    VkSwapchainKHR vk_swap_chain, VkExtent2D vk_swap_chain_extent,
    std::vector<VkImage> vk_swap_chain_images
);
