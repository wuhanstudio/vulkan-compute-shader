#ifdef __cplusplus
extern "C" {
#endif

#include "memory.h"
#include "device.h"
#include <stdio.h>
#include <string.h>

#include "instance.h"
#include "compute.h"

uint32_t FindMemoryIndexByType(VkPhysicalDevice PhysicalDevice, uint32_t allowedTypesMask, VkMemoryPropertyFlags flags)
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

VkBuffer vk_create_buffer_and_memory(
    VkPhysicalDevice vk_phy_device, VkDevice vk_device, 
    uint32_t size, VkDeviceMemory* deviceMemory)
{
    VkBufferCreateInfo bufferInfo;
    memset(&bufferInfo, 0, sizeof(bufferInfo));

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBuffer buffer;
    if (vkCreateBuffer(vk_device, &bufferInfo, NULL, &buffer) != VK_SUCCESS)
    {
        printf("Failed to create a buffer.\n");
        return VK_NULL_HANDLE;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(vk_device, buffer, &memoryRequirements);
    
    VkMemoryAllocateInfo memAllocInfo;
    memset(&memAllocInfo, 0, sizeof(memAllocInfo));

    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memoryRequirements.size;
    memAllocInfo.memoryTypeIndex = FindMemoryIndexByType(vk_phy_device, memoryRequirements.memoryTypeBits,
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    if (vkAllocateMemory(vk_device, &memAllocInfo, NULL, deviceMemory) != VK_SUCCESS)
    {
        printf("Failed to allocate memory for the buffer.\n");
        vkDestroyBuffer(vk_device, buffer, NULL);
        return VK_NULL_HANDLE;
    }

    if (vkBindBufferMemory(vk_device, buffer, *deviceMemory, 0) != VK_SUCCESS)
    {
        printf("Failed to bind buffer and memory.\n");
    }

    return buffer;
}

void vk_create_buffers(
    VkDevice vk_device, 
    VkDescriptorSet vk_descriptor_set, 
    uint32_t vk_input_size, 
    uint32_t vk_output_size, 
    VkBuffer vk_input_buffer, 
    VkBuffer vk_output_buffer)
{
    VkDescriptorBufferInfo descriptorBuffers[2];
    descriptorBuffers[0].buffer = vk_input_buffer;
    descriptorBuffers[0].offset = 0;
    descriptorBuffers[0].range = vk_input_size;

    descriptorBuffers[1].buffer = vk_output_buffer;
    descriptorBuffers[1].offset = 0;
    descriptorBuffers[1].range = vk_output_size;

    VkWriteDescriptorSet writeDescriptorSet;
    memset(&writeDescriptorSet, 0, sizeof(writeDescriptorSet));
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = vk_descriptor_set;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 2;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pBufferInfo = descriptorBuffers;

    vkUpdateDescriptorSets(vk_device, 1, &writeDescriptorSet, 0, NULL);
}

void vk_destroy_buffers(VkDevice vk_device, 
    VkBuffer vk_input_buffer, VkBuffer vk_output_buffer,
    VkDeviceMemory vk_input_buffer_memory, VkDeviceMemory vk_output_buffer_memory)
{
    vkDestroyBuffer(vk_device, vk_input_buffer, NULL);
    vkFreeMemory(vk_device, vk_input_buffer_memory, NULL);

    vkDestroyBuffer(vk_device, vk_output_buffer, NULL);
    vkFreeMemory(vk_device, vk_output_buffer_memory, NULL);
}

void vk_copy_to_input_buffer(VkDevice vk_device, void *data, uint32_t size, VkDeviceMemory vk_input_buffer_memory)
{
    void *address;

    if (vkMapMemory(vk_device, vk_input_buffer_memory, 0, size, 0, &address) != VK_SUCCESS)
    {
        printf("Failed to map input buffer memory.\n");
        return;
    }

    memcpy(address, data, size);

    vkUnmapMemory(vk_device, vk_input_buffer_memory);
}

void vk_copy_from_output_buffer(VkDevice vk_device, void *data, uint32_t size, VkDeviceMemory vk_output_buffer_memory)
{
    void *address;

    if (vkMapMemory(vk_device, vk_output_buffer_memory, 0, size, 0, &address) != VK_SUCCESS)
    {
        printf("Failed to map input buffer memory.\n");
        return;
    }

    memcpy(data, address, size);

    vkUnmapMemory(vk_device, vk_output_buffer_memory);
}

#ifdef __cplusplus
}
#endif
