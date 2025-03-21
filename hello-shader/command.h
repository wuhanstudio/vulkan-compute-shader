#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "pipeline.h"

#include "vertex.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

extern VkCommandPool commandPool;
extern std::vector<VkCommandBuffer> commandBuffers;

void vk_create_command_pool(VkPhysicalDevice vk_physical_device, VkDevice vk_device, VkSurfaceKHR vk_surface);
void vk_create_command_buffers(VkDevice vk_device, VkExtent2D vk_swap_chain_extent);
void vk_record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkExtent2D vk_swap_chain_extent);
