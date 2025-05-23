#include "app.h"

VkSurfaceFormatKHR VulkanParticleApp::vk_choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanParticleApp::vk_choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanParticleApp::vk_choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(gWindow, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapChainSupportDetails VulkanParticleApp::vk_query_swapchain_support(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vk_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vk_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vk_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


void VulkanParticleApp::vk_recreate_swapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(gWindow, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(gWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vk_device);

    vk_cleanup_swapchain();

    vk_create_swapchain();
    vk_create_imageviews();
    vk_create_framebuffers();
}

void VulkanParticleApp::vk_create_swapchain() {
    SwapChainSupportDetails swapChainSupport = vk_query_swapchain_support(vk_physical_device);

    VkSurfaceFormatKHR surfaceFormat = vk_choose_swap_surface_format(swapChainSupport.formats);
    VkPresentModeKHR presentMode = vk_choose_swap_present_mode(swapChainSupport.presentModes);
    VkExtent2D extent = vk_choose_swap_extent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vk_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = vk_find_queue_families(vk_physical_device);
    uint32_t queueFamilyIndices[] = { indices.graphicsAndComputeFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsAndComputeFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(vk_device, &createInfo, nullptr, &vk_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &imageCount, nullptr);
    vk_swapchain_images.resize(imageCount);
    vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &imageCount, vk_swapchain_images.data());

    vk_swapchain_image_format = surfaceFormat.format;
    vk_swapchain_extent = extent;
}

void VulkanParticleApp::vk_cleanup_swapchain() {
    for (auto framebuffer : vk_swapchain_framebuffers) {
        vkDestroyFramebuffer(vk_device, framebuffer, nullptr);
    }

    for (auto imageView : vk_swapchain_imageviews) {
        vkDestroyImageView(vk_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
}
