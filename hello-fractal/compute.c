#include "compute.h"
#include <string.h>
#include <stdio.h>
#include "device.h"
#include "pipeline.h"

VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;

void PrepareCommandBuffer(void) 
{
    VkCommandBufferAllocateInfo allocInfo;
    memset(&allocInfo, 0, sizeof(allocInfo));
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = ComputeCmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(LogicalDevice, &allocInfo, &CommandBuffer) != VK_SUCCESS)
    {
        printf("Failed to allocate the buffer\n");
        return;
    }

    VkCommandBufferBeginInfo beginInfo;
    memset(&beginInfo, 0, sizeof(beginInfo));
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(CommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        printf("Failed to begin the buffer\n");
        return;
    }

    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
    vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayout,
                            0, 1, &DescriptorSet, 0, NULL);
    vkCmdDispatch(CommandBuffer, 1000, 1, 1);

    if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
    {
        printf("Failed to end the buffer\n");
        return;
    }
}

int Compute(void)
{
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo;
    memset(&fenceCreateInfo, 0, sizeof(fenceCreateInfo));
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (vkCreateFence(LogicalDevice, &fenceCreateInfo, NULL, &fence) != VK_SUCCESS)
    {
        printf("Failed to create a fence.\n");
    }

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(submitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &CommandBuffer;

    if (vkQueueSubmit(ComputingQueue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        printf("submitting the command buffer failed\n");
        return -1;
    }

    if (vkWaitForFences(LogicalDevice, 1, &fence, VK_TRUE, 1000000000) != VK_SUCCESS)
    {
        printf("Failed to wait for the fence.\n");
    }

    vkDestroyFence(LogicalDevice, fence, NULL);

    return 0;
}

void CreateDescriptorSet(void)
{
    CreateDescriptorPool();

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo;
    memset(&descriptorSetAllocInfo, 0, sizeof(descriptorSetAllocInfo));
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &DescriptorSetLayout;
    descriptorSetAllocInfo.descriptorPool = DescriptorPool;

    if (vkAllocateDescriptorSets(LogicalDevice, &descriptorSetAllocInfo, &DescriptorSet) != VK_SUCCESS)
    {
        printf("Failed to allocate the descriptor set.");
    }
}