#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "validation.h"

void vk_print_extensions();
VkInstance vk_create_instance(VkDebugUtilsMessengerEXT* vk_debug_messenger);
std::vector<const char*> vk_get_required_extensions();
