#include "memory.h"
#include "device.h"
#include "instance.h"
#include <stdio.h>
#include <string.h>
#include "compute.h"

VkBuffer InputBuffer = VK_NULL_HANDLE;
VkBuffer OutputBuffer = VK_NULL_HANDLE;
VkDeviceMemory InputBufferMemory = VK_NULL_HANDLE;
VkDeviceMemory OutputBufferMemory = VK_NULL_HANDLE;

uint32_t FindMemoryIndexByType(uint32_t allowedTypesMask, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memProperties);

    uint32_t typeMask = 1;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++, typeMask <<= 1)
    {
        if ((allowedTypesMask & typeMask) != 0)
        {
            if ((memProperties.memoryTypes[i].propertyFlags & flags) == flags)
            {
                return i;
            }
        }
    }

    printf("Failed to find memory type index.\n");
    return 0;
}

VkBuffer CreateBufferAndMemory(uint32_t size, VkDeviceMemory *deviceMemory)
{
    VkBuffer buffer;
    VkDeviceMemory memory;

    VkBufferCreateInfo bufferInfo;
    memset(&bufferInfo, 0, sizeof(bufferInfo));
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    if (vkCreateBuffer(LogicalDevice, &bufferInfo, NULL, &buffer) != VK_SUCCESS)
    {
        printf("Failed to create a buffer.\n");
        return VK_NULL_HANDLE;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(LogicalDevice, buffer, &memoryRequirements);
    
    VkMemoryAllocateInfo memAllocInfo;
    memset(&memAllocInfo, 0, sizeof(memAllocInfo));
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memoryRequirements.size;
    memAllocInfo.memoryTypeIndex = FindMemoryIndexByType(memoryRequirements.memoryTypeBits,
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    if (vkAllocateMemory(LogicalDevice, &memAllocInfo, NULL, &memory) != VK_SUCCESS)
    {
        printf("Failed to allocate memory for the buffer.\n");
        vkDestroyBuffer(LogicalDevice, buffer, NULL);
        return VK_NULL_HANDLE;
    }

    if (vkBindBufferMemory(LogicalDevice, buffer, memory, 0) != VK_SUCCESS)
    {
        printf("Failed to bind buffer and memory.\n");
    }

    *deviceMemory = memory;
    return buffer;
}

void CreateBuffers(uint32_t inputSize, uint32_t outputSize)
{
    InputBuffer = CreateBufferAndMemory(inputSize, &InputBufferMemory);
    OutputBuffer = CreateBufferAndMemory(outputSize, &OutputBufferMemory);

    VkDescriptorBufferInfo descriptorBuffers[2];
    descriptorBuffers[0].buffer = InputBuffer;
    descriptorBuffers[0].offset = 0;
    descriptorBuffers[0].range = inputSize;
    descriptorBuffers[1].buffer = OutputBuffer;
    descriptorBuffers[1].offset = 0;
    descriptorBuffers[1].range = outputSize;

    VkWriteDescriptorSet writeDescriptorSet;
    memset(&writeDescriptorSet, 0, sizeof(writeDescriptorSet));
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = DescriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 2;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pBufferInfo = descriptorBuffers;

    vkUpdateDescriptorSets(LogicalDevice, 1, &writeDescriptorSet, 0, NULL);
}

void DestroyBuffers(void)
{
    vkDestroyBuffer(LogicalDevice, InputBuffer, NULL);
    vkFreeMemory(LogicalDevice, InputBufferMemory, NULL);
    vkDestroyBuffer(LogicalDevice, OutputBuffer, NULL);
    vkFreeMemory(LogicalDevice, OutputBufferMemory, NULL);
}

void CopyToInputBuffer(void *data, uint32_t size)
{
    void *address;

    if (vkMapMemory(LogicalDevice, InputBufferMemory, 0, size, 0, &address) != VK_SUCCESS)
    {
        printf("Failed to map input buffer memory.\n");
        return;
    }

    memcpy(address, data, size);

    vkUnmapMemory(LogicalDevice, InputBufferMemory);
}

void CopyFromOutputBuffer(void *data, uint32_t size)
{
    void *address;

    if (vkMapMemory(LogicalDevice, OutputBufferMemory, 0, size, 0, &address) != VK_SUCCESS)
    {
        printf("Failed to map input buffer memory.\n");
        return;
    }

    memcpy(data, address, size);

    vkUnmapMemory(LogicalDevice, OutputBufferMemory);
}
