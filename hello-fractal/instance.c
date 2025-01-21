#include <string.h>
#include <stdio.h>
#include "instance.h"

VkInstance Instance = VK_NULL_HANDLE;
VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;

void GetPhysicalDevice(void)
{
    VkPhysicalDevice devices[100];
    uint32_t count = 100;

    if (vkEnumeratePhysicalDevices(Instance, &count, devices) != VK_SUCCESS)
    {
        printf("Enumerating physical devices failed\n");
        return;
    }

    PhysicalDevice = devices[0];

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(PhysicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(PhysicalDevice, &deviceFeatures);
}

void CreateInstance(void)
{
    const char *layers[] = { "VK_LAYER_KHRONOS_validation" };
    VkInstanceCreateInfo instanceCreateInfo;
    memset(&instanceCreateInfo, 0, sizeof(instanceCreateInfo));
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.ppEnabledLayerNames = layers;
    instanceCreateInfo.enabledLayerCount = 1;

    if (vkCreateInstance(&instanceCreateInfo, NULL, &Instance) != VK_SUCCESS)
    {
        printf("Instance not created\n");
    }
}
