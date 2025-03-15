#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

VkDescriptorSetLayout vk_create_descriptor_set_layout(VkDevice vk_device);
VkPipelineLayout vk_create_pipeline_layout(VkDevice vk_device, VkDescriptorSetLayout vk_descriptor_set_layout);

VkShaderModule vk_create_compute_shader(VkDevice vk_device, const char* filename);

VkPipeline vk_create_pipline(VkDevice vk_device, VkPipelineLayout vk_pipeline_layout, VkDescriptorSetLayout vk_descriptor_set_layout, VkShaderModule vk_shader_module);
void vk_destroy_pipeline(VkDevice vk_device, VkPipeline vk_pipeline, VkPipelineLayout vk_pipeline_layout, VkDescriptorSetLayout vk_descriptor_set_layout);

#ifdef __cplusplus
}
#endif
