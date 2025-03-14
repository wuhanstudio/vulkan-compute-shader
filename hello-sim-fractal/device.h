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

VkDescriptorPool vk_create_descriptor_pool(VkDevice vk_device);

#ifdef __cplusplus
}
#endif
