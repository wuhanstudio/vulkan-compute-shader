#include "app.h"
#include <fmt/core.h>

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

void vk_print_device_name(VkPhysicalDevice device, const char* prefix) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Print device information
    fmt::println("{}Device name: {}", prefix, deviceProperties.deviceName);
    fmt::println("{}Device type: {}", prefix, vkPhysicalDeviceType_as_string(deviceProperties.deviceType));
    fmt::println("{}API version: {}.{}.{}\n",
        prefix,
        VK_VERSION_MAJOR(deviceProperties.apiVersion),
        VK_VERSION_MINOR(deviceProperties.apiVersion),
        VK_VERSION_PATCH(deviceProperties.apiVersion));

    // Print queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    fmt::println("{}Queue families: {}", prefix, queueFamilyCount);
    for (const auto& queueFamily : queueFamilies) {
        fmt::print("\t Queue count: {}", queueFamily.queueCount);
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            fmt::print("| Graphics ");
        }
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            fmt::print("| Compute ");
        }
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            fmt::print("| Transfer ");
        }
        if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
            fmt::print("| Sparse binding ");
        }
        if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) {
            fmt::print("| Protected");
        }
        if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
            fmt::print("| Video Decode ");
        }
        if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
            fmt::print("| Video Encode ");
        }
        if (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
            fmt::print("| Optical Flow ");
        }
        fmt::print("\n");
    }
    fmt::print("\n");

    // Print compute capabilities
    fmt::println("{}Max Dimension 1D: {}", prefix, deviceProperties.limits.maxImageDimension1D);
    fmt::println("{}Max Dimension 2D: {}", prefix, deviceProperties.limits.maxImageDimension2D);
    fmt::println("{}Max Dimension 3D: {}", prefix, deviceProperties.limits.maxImageDimension3D);

    fmt::println("{}Max Compute Work Group Count: [{}, {}, {}]", prefix, deviceProperties.limits.maxComputeWorkGroupCount[0], deviceProperties.limits.maxComputeWorkGroupCount[1], deviceProperties.limits.maxComputeWorkGroupCount[2]);
    fmt::println("{}Max Compute Work Group Size: [{}, {}, {}]", prefix, deviceProperties.limits.maxComputeWorkGroupSize[0], deviceProperties.limits.maxComputeWorkGroupSize[1], deviceProperties.limits.maxComputeWorkGroupSize[2]);
    fmt::println("{}Max Compute Work Group Invocations: {}", prefix, deviceProperties.limits.maxComputeWorkGroupInvocations);
    fmt::println("{}Max Compute Shared Memory Size: {}", prefix, deviceProperties.limits.maxComputeSharedMemorySize);
    fmt::print("\n");
}

QueueFamilyIndices VulkanParticleApp::vk_find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            indices.graphicsAndComputeFamily = i;
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

bool VulkanParticleApp::vk_is_device_suitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = vk_find_queue_families(device);

    bool extensionsSupported = vk_check_device_extension_support(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = vk_query_swapchain_support(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanParticleApp::vk_check_device_extension_support(VkPhysicalDevice device) {
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

void VulkanParticleApp::vk_pick_physical_device() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, devices.data());

    fmt::println("\nAvailable devices:\n");
    for (int i = 0; i < deviceCount; i++) {
        char title[5];
        std::snprintf(title, sizeof(title), "[%d] ", i);
        vk_print_device_name(devices[i], title);
    }

    fmt::println("");

    //for (int i = 0; i < deviceCount; i++) {
    //    if (vk_is_device_suitable(devices[i])) {
    //        printf("Chosen Device %d\n", i);
    //        vk_physical_device = devices[i];
    //        break;
    //    }
    //}

    // Find a suitable device
    int deviceIndex = -1;
    printf("Choose a physical device:\n");
    while (deviceIndex < 0 || deviceIndex >= deviceCount) {
        printf("Device index from 0 to %d:\n", -1 + deviceCount);
        std::cin >> deviceIndex;
        if (!vk_is_device_suitable(devices[deviceIndex]))
        {
            printf("Device not suitable\n");
            deviceIndex = -1;
        }
    }
    printf("Chosen Device %d\n", deviceIndex);
    vk_physical_device = devices[deviceIndex];

    if (vk_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void VulkanParticleApp::vk_create_logical_device() {
    QueueFamilyIndices indices = vk_find_queue_families(vk_physical_device);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsAndComputeFamily.value(), indices.presentFamily.value() };

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

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(vk_physical_device, &createInfo, nullptr, &vk_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(vk_device, indices.graphicsAndComputeFamily.value(), 0, &vk_graphics_queue);
    vkGetDeviceQueue(vk_device, indices.graphicsAndComputeFamily.value(), 0, &vk_compute_queue);
    vkGetDeviceQueue(vk_device, indices.presentFamily.value(), 0, &vk_present_queue);
}
