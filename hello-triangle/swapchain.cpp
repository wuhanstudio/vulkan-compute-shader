#include "swapchain.h"

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

//void cleanupSwapChain() {
//    for (auto framebuffer : swapChainFramebuffers) {
//        vkDestroyFramebuffer(device, framebuffer, nullptr);
//    }
//
//    for (auto imageView : swapChainImageViews) {
//        vkDestroyImageView(device, imageView, nullptr);
//    }
//
//    vkDestroySwapchainKHR(device, swapChain, nullptr);
//}

//void recreateSwapChain() {
//    int width = 0, height = 0;
//    glfwGetFramebufferSize(window, &width, &height);
//    while (width == 0 || height == 0) {
//        glfwGetFramebufferSize(window, &width, &height);
//        glfwWaitEvents();
//    }
//
//    vkDeviceWaitIdle(device);
//
//    cleanupSwapChain();
//
//    //createSwapChain();
//    //createImageViews();
//    //createFramebuffers();
//}
