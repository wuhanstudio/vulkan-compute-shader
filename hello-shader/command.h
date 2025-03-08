#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "pipeline.h"

#include "vertex.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

extern VkCommandPool commandPool;
extern std::vector<VkCommandBuffer> commandBuffers;

void createCommandPool();
void createCommandBuffers();
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
