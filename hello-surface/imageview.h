#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "device.h"

void createImageViews(VkDevice device, VkFormat swapChainImageFormat, std::vector<VkImageView> swapChainImageViews, std::vector<VkImage> swapChainImages);
