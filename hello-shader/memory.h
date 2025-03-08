#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>

#include "device.h"

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
