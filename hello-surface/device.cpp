#include "device.h"

#include <fmt/core.h>
#include <iostream>
#include <set>

#include "validation.h"
#include "swapchain.h"

VkQueue graphicsQueue;
VkQueue presentQueue;

void printDeviceName(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    fmt::println("Device name: {}", deviceProperties.deviceName);
}

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

void printDeviceInfo(VkInstance instance, VkSurfaceKHR surface)
{
    // Query the number of Vulkan physical devices
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    fmt::println("Number of physical devices: {}\n", physicalDeviceCount);

    std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());

    fmt::println("Available devices:");
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        fmt::println("Device name: {}", deviceProperties.deviceName);
        fmt::println("Device type: {}", vkPhysicalDeviceType_as_string(deviceProperties.deviceType));
        fmt::println("API version: {}.{}.{}\n",
            VK_VERSION_MAJOR(deviceProperties.apiVersion),
            VK_VERSION_MINOR(deviceProperties.apiVersion),
            VK_VERSION_PATCH(deviceProperties.apiVersion));

        if (checkDeviceExtensionSupport(device, swapchainExtensions)) {
            fmt::print("Device supports swapchain extensions\n");
        }
        else {
            fmt::print("Device does not support required extensions\n");
        }

        // Print queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        fmt::println("Queue families: {}", queueFamilyCount);
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            fmt::print("  Queue count: {}", queueFamily.queueCount);
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
                fmt::print("| Protected ");
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
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                fmt::print(" [ Present Support ]");
            }
            fmt::print("\n");
            i++;
        }
        fmt::print("\n");
    }
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

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

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> deviceExtensions) {
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

VkPhysicalDevice choosePhysicalDevice(VkInstance instance)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    fmt::println("Number of physical devices: {}\n", physicalDeviceCount);

    std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());

    int deviceIndex = -1;
    fmt::print("Choose a physical device:\n", devices.size());
    while (deviceIndex < 0 || deviceIndex >= devices.size()) {
        fmt::print("Device index from 0 to {}:", -1 + devices.size());
        std::cin >> deviceIndex;
    }
    fmt::println("Chosen Device {}:", deviceIndex);

    return devices[deviceIndex];
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    bool enableValidationLayers = checkValidationLayerSupport();
#ifdef NDEBUG
    enableValidationLayers = false;
    fmt::print("Running in release mode, validation layers disabled\n");
#endif

    VkDevice device;
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    fmt::println("Graphics queue families: {}", indices.graphicsFamily.value());
    fmt::println("Present queue families: {}", indices.presentFamily.value());

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
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

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(swapchainExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = swapchainExtensions.data();

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

    return device;
}
