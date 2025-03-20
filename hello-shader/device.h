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

bool vk_is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR vk_surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);

void vk_pick_physical_device(VkInstance instance, VkSurfaceKHR vk_surface);
void vk_create_logical_device(VkSurfaceKHR vk_surface);

QueueFamilyIndices vk_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR vk_surface);
