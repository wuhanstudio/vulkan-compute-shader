#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include "device.h"
#include "instance.h"
#include "memory.h"

VkDevice vk_create_device_and_compute_queue(VkPhysicalDevice vk_phy_device, uint32_t* vk_queue_family_index, VkQueue* vk_queue_compute)
{
    VkQueueFamilyProperties families[MAX_QUEUE_FAMILY];
    uint32_t count = MAX_QUEUE_FAMILY;

    vkGetPhysicalDeviceQueueFamilyProperties(vk_phy_device, &count, families);

    printf("Found %u queue families\n", count);
     
    uint32_t i = 0;
    while ((i < count) &&
            (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)
    {
        i++;
    }
	*vk_queue_family_index = i;
    if (*vk_queue_family_index == count)
    {
        printf("Compute queue not found\n");
        return VK_NULL_HANDLE;
    }

    float prio = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo;
    memset(&queueCreateInfo, 0, sizeof(queueCreateInfo));

    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = *vk_queue_family_index;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &prio;

    VkDeviceCreateInfo deviceCreateInfo;
    memset(&deviceCreateInfo, 0, sizeof(deviceCreateInfo));

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;

    VkDevice vk_device;
    if (vkCreateDevice(vk_phy_device, &deviceCreateInfo, NULL, &vk_device) != VK_SUCCESS)
    {
        printf("Failed to crate logical device\n");
        return VK_NULL_HANDLE;
    }
    vkGetDeviceQueue(vk_device, *vk_queue_family_index, 0, vk_queue_compute);
    
	return vk_device;
}
 
VkCommandPool vk_create_command_pool(VkDevice vk_device, uint32_t vk_queue_family_index)
{
	VkCommandPool vk_compute_cmd_pool = VK_NULL_HANDLE;

    VkCommandPoolCreateInfo poolCreateInfo;
    memset(&poolCreateInfo, 0, sizeof(poolCreateInfo));
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = vk_queue_family_index;

    if (vkCreateCommandPool(vk_device, &poolCreateInfo, NULL, &vk_compute_cmd_pool) != VK_SUCCESS)
    {
        printf("Failed to create a command pool\n");
		return VK_NULL_HANDLE;
    }
	return vk_compute_cmd_pool;
}

VkDescriptorPool vk_create_descriptor_pool(VkDevice vk_device)
{
	VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;

    VkDescriptorPoolSize descriptorPoolSize;
    memset(&descriptorPoolSize, 0, sizeof(descriptorPoolSize));
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize.descriptorCount = 2;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
    memset(&descriptorPoolCreateInfo, 0, sizeof(descriptorPoolCreateInfo));
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
    descriptorPoolCreateInfo.poolSizeCount = 1;

    if (vkCreateDescriptorPool(vk_device, &descriptorPoolCreateInfo, NULL, &vk_descriptor_pool) != VK_SUCCESS)
    {
        printf("Failed to create the descriptor pool.\n");
    }

	return vk_descriptor_pool;
}

#ifdef __cplusplus
}
#endif
