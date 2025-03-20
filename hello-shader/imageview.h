#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "device.h"

extern VkFormat swapChainImageFormat;
extern std::vector<VkImage> swapChainImages;
extern std::vector<VkImageView> swapChainImageViews;

void vk_create_image_views(VkDevice vk_device);
