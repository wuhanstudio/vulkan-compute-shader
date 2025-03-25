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
VkSurfaceFormatKHR vk_choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);

std::vector<VkImage> vk_create_swapchain_images(VkDevice vk_device, VkSwapchainKHR vk_swap_chain);

void vk_cleanup_swap_chain(
    VkDevice vk_device, VkSwapchainKHR vk_swapchain, 
    std::vector<VkImageView> vk_swapchain_imageviews,
    std::vector<VkFramebuffer> vk_swapchain_framebuffers
);

void vk_recreate_swapchain(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    VkSurfaceKHR vk_surface, GLFWwindow* window,
    VkSwapchainKHR& vk_swapchain, VkExtent2D& vk_swapchain_extent,
    std::vector<VkImage>& vk_swapchain_images,
    std::vector<VkImageView>& vk_swapchain_imageviews,
    VkRenderPass vk_render_pass, VkFormat& vk_swapchain_image_format,
    std::vector<VkFramebuffer>& vk_swapchain_framebuffers
);
