#pragma once

#include "device.h"
#include "swapchain.h"

#include <vulkan/vulkan.h>

extern VkRenderPass renderPass;
extern VkPipelineLayout pipelineLayout;
extern VkPipeline graphicsPipeline;

void createRenderPass();
void createGraphicsPipeline(std::vector<char> vertShaderCode, std::vector<char> fragShaderCode);