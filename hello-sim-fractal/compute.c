#ifdef __cplusplus
extern "C" {
#endif

#include "compute.h"
#include <string.h>
#include <stdio.h>
#include "device.h"
#include "pipeline.h"

VkCommandBuffer vk_prepare_command_buffer(
    VkDevice vk_device, 
    VkPipeline vk_pipeline, 
    VkPipelineLayout vk_pipeline_layout, 
    VkDescriptorSet vk_descriptor_set,
    VkCommandPool vk_compute_cmd_pool)
{
    VkCommandBufferAllocateInfo allocInfo;
    memset(&allocInfo, 0, sizeof(allocInfo));

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vk_compute_cmd_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

	VkCommandBuffer vk_command_buffer;
    if (vkAllocateCommandBuffers(vk_device, &allocInfo, &vk_command_buffer) != VK_SUCCESS)
    {
        printf("Failed to allocate the buffer\n");
        return VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo beginInfo;
    memset(&beginInfo, 0, sizeof(beginInfo));

    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(vk_command_buffer, &beginInfo) != VK_SUCCESS)
    {
        printf("Failed to begin the buffer\n");
        return VK_NULL_HANDLE;
    }

    vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, vk_pipeline);
    vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, vk_pipeline_layout,
                            0, 1, &vk_descriptor_set, 0, NULL);
    vkCmdDispatch(vk_command_buffer, 256, 1, 1);

    if (vkEndCommandBuffer(vk_command_buffer) != VK_SUCCESS)
    {
        printf("Failed to end the buffer\n");
        return VK_NULL_HANDLE;
    }

	return vk_command_buffer;
}

int vk_compute(VkDevice vk_device, VkQueue vk_queue_compute, VkCommandBuffer vk_command_buffer)
{
    VkFenceCreateInfo fenceCreateInfo;
    memset(&fenceCreateInfo, 0, sizeof(fenceCreateInfo));

    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence;
    if (vkCreateFence(vk_device, &fenceCreateInfo, NULL, &fence) != VK_SUCCESS)
    {
        printf("Failed to create a fence.\n");
    }

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(submitInfo));

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_command_buffer;

    if (vkQueueSubmit(vk_queue_compute, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        printf("Submitting the command buffer failed\n");
        return -1;
    }

    if (vkWaitForFences(vk_device, 1, &fence, VK_TRUE, 1000000000) != VK_SUCCESS)
    {
        printf("Failed to wait for the fence.\n");
    }

    vkDestroyFence(vk_device, fence, NULL);

    return 0;
}

VkDescriptorSet vk_create_descriptor_set(
    VkDevice vk_device, 
    VkDescriptorSetLayout vk_descriptor_set_layout, 
    VkDescriptorPool vk_descriptor_pool)
{
    VkDescriptorSet vk_descriptor_set = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo;
    memset(&descriptorSetAllocInfo, 0, sizeof(descriptorSetAllocInfo));

    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &vk_descriptor_set_layout;
    descriptorSetAllocInfo.descriptorPool = vk_descriptor_pool;

    if (vkAllocateDescriptorSets(vk_device, &descriptorSetAllocInfo, &vk_descriptor_set) != VK_SUCCESS)
    {
        printf("Failed to allocate the descriptor set.");
    }
    return vk_descriptor_set;
}

#ifdef __cplusplus
}
#endif