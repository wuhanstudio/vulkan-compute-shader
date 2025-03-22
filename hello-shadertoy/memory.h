#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>

#include "device.h"

uint32_t vk_find_memory_type(VkPhysicalDevice vk_physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
