#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "device.h"

void vk_create_image_views(VkDevice device, VkFormat swapChainImageFormat, std::vector<VkImageView> swapChainImageViews, std::vector<VkImage> swapChainImages);
