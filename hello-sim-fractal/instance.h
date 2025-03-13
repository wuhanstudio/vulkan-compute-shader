#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

#define MAX_PHY_DEVICE 8
#define MAX_QUEUE_FAMILY 32

extern const char* vk_validation_layers[];

VkInstance vk_create_instance();
VkPhysicalDevice vk_create_physical_device(VkInstance instance);

#ifdef __cplusplus
}
#endif
