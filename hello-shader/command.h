#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "pipeline.h"

#include "vertex.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

VkCommandPool vk_create_command_pool(VkPhysicalDevice vk_physical_device, VkDevice vk_device, VkSurfaceKHR vk_surface);
std::vector<VkCommandBuffer> vk_create_command_buffers(VkDevice vk_device, VkExtent2D vk_swap_chain_extent, VkCommandPool vk_command_pool);

void vk_record_command_buffer(
    VkCommandBuffer commandBuffer, uint32_t imageIndex,
    VkExtent2D vk_swap_chain_extent, VkRenderPass vk_render_pass,
    VkPipelineLayout vk_pipeline_layout, VkPipeline vk_graphics_pipeline,
	std::vector<VkDescriptorSet> vk_descriptor_sets, uint32_t currentFrame
);
