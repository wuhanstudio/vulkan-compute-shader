#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

#include "vertex.h"

extern VkRenderPass renderPass;
extern VkPipelineLayout pipelineLayout;
extern VkPipeline graphicsPipeline;

extern VkDescriptorSetLayout descriptorSetLayout;

void createRenderPass();
void createGraphicsPipeline(std::vector<char> vertShaderCode, std::vector<char> fragShaderCode);