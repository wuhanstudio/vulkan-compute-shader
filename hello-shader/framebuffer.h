#pragma once

#include <vulkan/vulkan.h>

#include "pipeline.h"

extern std::vector<VkFramebuffer> swapChainFramebuffers;

void vk_create_frame_buffers(
	VkDevice vk_device, VkExtent2D vk_swap_chain_extent, 
	std::vector<VkImageView> vk_swapchain_imageviews, VkRenderPass vk_render_pass
);
