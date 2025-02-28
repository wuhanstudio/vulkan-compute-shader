#include "device.h"

#include <fmt/core.h>
#include <set>

//#include "swapchain.h"

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;

VkQueue graphicsQueue;
VkQueue presentQueue;

void printDeviceName(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    fmt::println("Device name: {}", deviceProperties.deviceName);
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
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

        //VkBool32 presentSupport = false;
        //vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        //if (presentSupport) {
        //    indices.presentFamily = i;
        //}

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
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

bool isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    //bool extensionsSupported = checkDeviceExtensionSupport(device);

    //bool swapChainAdequate = false;
    //if (extensionsSupported) {
    //    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    //    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    //}

    return indices.isComplete();
    //return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	fmt::println("\nAvailable devices:");
    for (const auto& device : devices) {
        printDeviceName(device);
    }

	fmt::println("");

	// Find a suitable device
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            fmt::println("Selected device:");
            printDeviceName(device);

            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}
