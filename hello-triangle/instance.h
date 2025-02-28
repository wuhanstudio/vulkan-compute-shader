#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "validation.h"

extern VkInstance instance;

void printExtensions();
std::vector<const char*> getRequiredExtensions();

void createInstance();
