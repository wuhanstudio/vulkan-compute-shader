#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

void vk_update_descriptor_set(
	VkDevice vk_device, 
	VkDescriptorSet vk_descriptor_set, 
	uint32_t vk_input_size, 
	uint32_t vk_output_size, 
	VkBuffer vk_input_buffer, 
	VkBuffer vk_output_buffer
);

VkBuffer vk_create_buffer_and_memory(
	VkPhysicalDevice vk_phy_device,
	VkDevice vk_device, 
	uint32_t size, 
	VkDeviceMemory* deviceMemory
);

void vk_destroy_buffers(VkDevice vk_device,
	VkBuffer vk_input_buffer, VkBuffer vk_output_buffer,
	VkDeviceMemory vk_input_buffer_memory, VkDeviceMemory vk_output_buffer_memory
);

void vk_copy_to_input_buffer(VkDevice vk_device, void* data, uint32_t size, VkDeviceMemory vk_input_buffer_memory);
void vk_copy_from_output_buffer(VkDevice vk_device, void* data, uint32_t size, VkDeviceMemory vk_output_buffer_memory);

#ifdef __cplusplus
}
#endif
