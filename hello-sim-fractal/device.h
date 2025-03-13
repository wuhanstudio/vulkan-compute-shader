#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

VkDevice vk_create_device_and_compute_queue(
    VkPhysicalDevice vk_phy_device, 
	uint32_t* vk_queue_family_index,
    VkQueue* vk_queue_compute
);

VkCommandPool vk_create_command_pool(
    VkDevice vk_device, 
    uint32_t vk_queue_family_index
);

void vk_destroy_command_pool_and_device(
    VkDevice vk_device,
    VkBuffer vk_input_buffer,
    VkBuffer vk_output_buffer,
    VkDescriptorPool vk_descriptor_pool,
    VkCommandPool vk_compute_cmd_pool
);

VkDescriptorPool vk_create_descriptor_pool(VkDevice vk_device);

#ifdef __cplusplus
}
#endif
