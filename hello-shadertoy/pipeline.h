#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

#include "vertex.h"

VkRenderPass vk_create_render_pass(VkDevice vk_device, VkFormat vk_swapchain_image_format);
VkPipelineLayout vk_create_pipeline_layout(VkDevice vk_device, VkDescriptorSetLayout vk_descriptor_set_layout);

VkPipeline vk_create_graphics_pipeline(
	VkDevice vk_device, 
	std::vector<char> vertShaderCode, std::vector<char> fragShaderCode,
	VkRenderPass vk_render_pass, VkDescriptorSetLayout vk_descriptor_set_layout, VkPipelineLayout vk_pipeline_layout
);
