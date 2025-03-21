#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "device.h"

extern VkFormat vk_swap_chain_image_format;
extern std::vector<VkImage> vk_swap_chain_images;
extern std::vector<VkImageView> swapChainImageViews;

void vk_create_image_views(VkDevice vk_device, std::vector<VkImage> vk_swap_chain_images);
