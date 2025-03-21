#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

#include "device.h"
#include "command.h"

extern VkImage textureImage;
extern VkDeviceMemory textureImageMemory;
extern VkImageView textureImageView;
extern VkSampler textureSampler;

VkDescriptorPool vk_create_descriptor_pool(VkDevice vk_device);

void vk_create_texture_imageview(VkDevice vk_device);
void vk_create_texture_sampler(VkPhysicalDevice vk_physical_device, VkDevice vk_device);

void vk_transition_image_layout(VkDevice vk_device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue vk_graphics_queue, VkCommandPool vk_command_pool);
void vk_copy_buffer_to_image(VkDevice vk_device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkQueue vk_graphics_queue, VkCommandPool vk_command_pool);

void vk_create_image(
	VkPhysicalDevice vk_physical_device, VkDevice vk_device,
	uint32_t width, uint32_t height, VkFormat format, 
	VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
	VkImage& image, VkDeviceMemory& imageMemory
);

void vk_create_texture_image(
	VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
	int texWidth, int texHeight, int texChannels, uint8_t* testData,
	VkQueue vk_graphics_queue, VkCommandPool vk_command_pool
);
