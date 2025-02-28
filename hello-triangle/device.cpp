#include "device.h"

#include "swapchain.h"

//bool isDeviceSuitable(VkPhysicalDevice device) {
//    QueueFamilyIndices indices = findQueueFamilies(device);
//
//    bool extensionsSupported = checkDeviceExtensionSupport(device);
//
//    bool swapChainAdequate = false;
//    if (extensionsSupported) {
//        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
//        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//    }
//
//    return indices.isComplete() && extensionsSupported && swapChainAdequate;
//}
