#include "device.h"

#include <fmt/core.h>
#include <iostream>
#include <set>

#include "swapchain.h"

VkQueue vk_graphics_queue;
VkQueue vk_present_queue;

void vk_print_device_name(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    fmt::println("Device name: {}", deviceProperties.deviceName);
}

QueueFamilyIndices vk_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR vk_surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk_surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool vk_check_device_extension(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool vk_is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR vk_surface) {
    QueueFamilyIndices indices = vk_find_queue_families(device, vk_surface);

    bool extensionsSupported = vk_check_device_extension(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = vk_query_swapchain_support(device, vk_surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

VkPhysicalDevice vk_pick_physical_device(VkInstance vk_instance, VkSurfaceKHR vk_surface) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, devices.data());

	fmt::println("\nAvailable devices:\n");
    for (const auto& device : devices) {
        vk_print_device_name(device);
    }

	fmt::println("");

	// Find a suitable device
    int deviceIndex = -1;
    printf("Choose a physical device:\n");
    while (deviceIndex < 0 || deviceIndex >= deviceCount) {
        printf("Device index from 0 to %d:\n", -1 + deviceCount);
        std::cin >> deviceIndex;
        if (!vk_is_device_suitable(devices[deviceIndex], vk_surface))
        {
			deviceIndex = -1;
        }
    }
    printf("Chosen Device %d\n", deviceIndex);

	return devices[deviceIndex];
}

VkDevice vk_create_logical_device(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface) {
    QueueFamilyIndices indices = vk_find_queue_families(vk_physical_device, vk_surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vk_check_validation_layer()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

	VkDevice vk_device;
    if (vkCreateDevice(vk_physical_device, &createInfo, nullptr, &vk_device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(vk_device, indices.graphicsFamily.value(), 0, &vk_graphics_queue);
    vkGetDeviceQueue(vk_device, indices.presentFamily.value(), 0, &vk_present_queue);

	return vk_device;
}
