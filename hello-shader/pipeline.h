#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

#include "vertex.h"

extern VkRenderPass renderPass;
extern VkPipelineLayout pipelineLayout;
extern VkPipeline graphicsPipeline;

extern VkDescriptorSetLayout descriptorSetLayout;

void vk_create_render_pass(VkDevice vk_device);
void vk_create_graphics_pipeline(VkDevice vk_device, std::vector<char> vertShaderCode, std::vector<char> fragShaderCode);