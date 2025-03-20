#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "validation.h"

void vk_print_extensions();
VkInstance vk_create_instance();
std::vector<const char*> vk_get_required_extensions();
