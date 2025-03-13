#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "instance.h"

const char* vk_validation_layers[] = { "VK_LAYER_KHRONOS_validation" };

const char* vkPhysicalDeviceType_as_string(VkPhysicalDeviceType type) {
	switch (type)
	{
	case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		return "Other";

	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return "Integrated GPU";

	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return "Discrete GPU";

	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return "Virtualized GPU";

	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		return "CPU";

	default:
		return "Unknown";
	}
}

VkPhysicalDevice vk_create_physical_device(VkInstance instance)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
    printf("Number of physical devices: %d\n\n", physicalDeviceCount);

	VkPhysicalDevice devices[MAX_PHY_DEVICE];
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices);

	printf("Available devices:\n\n");
	for (int i = 0; i < physicalDeviceCount; i++) {
		VkPhysicalDevice device = devices[i];

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Print device information
		printf("Device name: %s", deviceProperties.deviceName);
		printf("Device type: %s", vkPhysicalDeviceType_as_string(deviceProperties.deviceType));
		printf("API version: %d.%d.%d\n",
			VK_VERSION_MAJOR(deviceProperties.apiVersion),
			VK_VERSION_MINOR(deviceProperties.apiVersion),
			VK_VERSION_PATCH(deviceProperties.apiVersion));

		// Print queue families
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

		VkQueueFamilyProperties queueFamilies[MAX_QUEUE_FAMILY];
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

		printf("Queue families: %d\n", queueFamilyCount);
		for (int i = 0; i < queueFamilyCount; i++) {
			VkQueueFamilyProperties queueFamily = queueFamilies[i];
			printf("  Queue count: %d ", queueFamily.queueCount);
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				printf("| Graphics ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
				printf("| Compute ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
				printf("| Transfer ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
				printf("| Sparse binding ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) {
				printf("| Protected");
			}
			if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
				printf("| Video Decode ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
				printf("| Video Encode ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
				printf("| Optical Flow ");
			}
			printf("\n");
		}
		printf("\n");

		// Print compute capabilities
		printf("Max Dimension 1D: %d\n", deviceProperties.limits.maxImageDimension1D);
		printf("Max Dimension 2D: %d\n", deviceProperties.limits.maxImageDimension2D);
		printf("Max Dimension 3D: %d\n", deviceProperties.limits.maxImageDimension3D);

		printf("Max Compute Work Group Count: [%d, %d, %d]\n", deviceProperties.limits.maxComputeWorkGroupCount[0], deviceProperties.limits.maxComputeWorkGroupCount[1], deviceProperties.limits.maxComputeWorkGroupCount[2]);
		printf("Max Compute Work Group Size: [%d, %d, %d]\n", deviceProperties.limits.maxComputeWorkGroupSize[0], deviceProperties.limits.maxComputeWorkGroupSize[1], deviceProperties.limits.maxComputeWorkGroupSize[2]);
		printf("Max Compute Work Group Invocations: %d\n", deviceProperties.limits.maxComputeWorkGroupInvocations);
		printf("Max Compute Shared Memory Size: %d\n", deviceProperties.limits.maxComputeSharedMemorySize);
		printf("\n");
	}

    int deviceIndex = -1;
    printf("Choose a physical device:\n");
    while (deviceIndex < 0 || deviceIndex >= physicalDeviceCount) {
        printf("Device index from 0 to %d:\n", -1 + physicalDeviceCount);
		scanf("%d", &deviceIndex);
    }
    printf("Chosen Device %d\n", deviceIndex);

    return devices[deviceIndex];
}

VkInstance vk_create_instance(void)
{
    VkInstance vk_instance;

    VkInstanceCreateInfo instanceCreateInfo;
    memset(&instanceCreateInfo, 0, sizeof(instanceCreateInfo));

    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.ppEnabledLayerNames = vk_validation_layers;
    instanceCreateInfo.enabledLayerCount = 1;

    if (vkCreateInstance(&instanceCreateInfo, NULL, &vk_instance) != VK_SUCCESS)
    {
        printf("Instance not created\n");
    }

    return vk_instance;
}

#ifdef __cplusplus
}
#endif
