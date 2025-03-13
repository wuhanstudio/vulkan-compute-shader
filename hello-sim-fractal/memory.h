#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

void vk_create_buffers(VkPhysicalDevice vk_phy_device, VkDevice vk_device, VkDescriptorSet vk_descriptor_set, uint32_t vk_input_size, uint32_t vk_output_size, VkBuffer* vk_input_buffer, VkBuffer* vk_output_buffer);
void vk_destroy_buffers(VkDevice vk_device, VkBuffer vk_input_buffer, VkBuffer vk_output_buffer);

void vk_copy_to_input_buffer(VkDevice vk_device, void* data, uint32_t size);
void vk_copy_from_output_buffer(VkDevice vk_device, void* data, uint32_t size);

#ifdef __cplusplus
}
#endif
