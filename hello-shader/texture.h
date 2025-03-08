#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

#include "device.h"
#include "command.h"

extern VkImage textureImage;
extern VkDeviceMemory textureImageMemory;
extern VkImageView textureImageView;
extern VkSampler textureSampler;

extern VkDescriptorSetLayout descriptorSetLayout;

extern VkDescriptorPool descriptorPool;

extern std::vector<VkDescriptorSet> descriptorSets;

void createDescriptorPool();

void createTextureImageView();
void createTextureSampler();

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

void createDescriptorSetLayout();
void createDescriptorSets();

void createTextureImage(int texWidth, int texHeight, int texChannels, uint8_t* testData);
