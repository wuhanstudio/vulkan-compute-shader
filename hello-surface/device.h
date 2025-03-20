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

void vk_print_device_info(VkInstance instance, VkSurfaceKHR surface);

bool vk_check_device_extension(VkPhysicalDevice device, const std::vector<const char*> deviceExtensions);
QueueFamilyIndices vk_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

VkPhysicalDevice vk_pick_physical_device(VkInstance instance);
VkDevice vk_create_logical_device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
