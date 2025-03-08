#pragma once

#include <vulkan/vulkan.h>

#include "pipeline.h"

extern std::vector<VkFramebuffer> swapChainFramebuffers;

void createFramebuffers();
