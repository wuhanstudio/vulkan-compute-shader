#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "device.h"

std::vector<VkImageView> vk_create_image_views(
    VkDevice vk_device,
    std::vector<VkImage> vk_swap_chain_images,
    VkFormat vk_swapchain_image_format
);