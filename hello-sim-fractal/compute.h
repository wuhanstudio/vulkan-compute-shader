#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

VkDescriptorSet vk_create_descriptor_set(
    VkDevice vk_device,
    VkDescriptorSetLayout vk_descriptor_set_layout,
    VkDescriptorPool* vk_descriptor_pool
);

void vk_prepare_command_buffer(
    VkDevice vk_device,
    VkPipeline vk_pipeline,
    VkPipelineLayout vk_pipeline_layout,
    VkDescriptorSet vk_descriptor_set,
    VkCommandPool vk_compute_cmd_pool
);

int vk_compute(VkDevice vk_device, VkQueue vk_queue_compute);

#ifdef __cplusplus
}
#endif