#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "surface.h"

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

void printDeviceInfo(VkInstance instance, VkSurfaceKHR surface);

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> deviceExtensions);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

VkPhysicalDevice choosePhysicalDevice(VkInstance instance);
VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
