#pragma once

#include <vulkan/vulkan.h>
#include <optional>

#include "instance.h"
#include "surface.h"

extern VkPhysicalDevice physicalDevice;
extern VkDevice device;

extern VkQueue graphicsQueue;
extern VkQueue presentQueue;

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

bool isDeviceSuitable(VkPhysicalDevice device);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);

void pickPhysicalDevice();
void createLogicalDevice();

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
