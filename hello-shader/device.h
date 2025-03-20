#pragma once

#include <vulkan/vulkan.h>
#include <optional>

#include "instance.h"
#include "surface.h"

extern VkQueue vk_graphics_queue;
extern VkQueue vk_present_queue;

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
bool vk_check_device_extension(VkPhysicalDevice device);

VkPhysicalDevice vk_pick_physical_device(VkInstance instance, VkSurfaceKHR vk_surface);
VkDevice vk_create_logical_device(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface);

QueueFamilyIndices vk_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR vk_surface);
