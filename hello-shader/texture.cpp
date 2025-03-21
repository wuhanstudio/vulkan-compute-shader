#include "texture.h"
#include <cstring>

VkImageView vk_create_imageview(VkDevice vk_device, VkImage vk_texture_image, VkFormat format) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = vk_texture_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(vk_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return imageView;
}

void vk_create_image(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    uint32_t width, uint32_t height, VkFormat format, 
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
    VkImage* vk_texture_image, VkDeviceMemory* vk_texture_image_memory
) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vk_device, &imageInfo, nullptr, vk_texture_image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vk_device, *vk_texture_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vk_find_memory_type(vk_physical_device, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vk_device, &allocInfo, nullptr, vk_texture_image_memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(vk_device, *vk_texture_image, *vk_texture_image_memory, 0);
}

VkCommandBuffer vk_begin_single_time_commands(VkDevice vk_device, VkCommandPool vk_command_pool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vk_command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vk_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void vk_end_single_time_commands(VkDevice vk_device, VkCommandBuffer vk_command_buffer, VkQueue vk_graphics_queue, VkCommandPool vk_command_pool) {
    vkEndCommandBuffer(vk_command_buffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_command_buffer;

    vkQueueSubmit(vk_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vk_graphics_queue);

    vkFreeCommandBuffers(vk_device, vk_command_pool, 1, &vk_command_buffer);
}

void vk_transition_image_layout(
    VkDevice vk_device, VkImage image, VkFormat format, 
    VkImageLayout oldLayout, VkImageLayout newLayout,
    VkQueue vk_graphics_queue, VkCommandPool vk_command_pool
) {
    VkCommandBuffer vk_command_buffer = vk_begin_single_time_commands(vk_device, vk_command_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        vk_command_buffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vk_end_single_time_commands(vk_device, vk_command_buffer, vk_graphics_queue, vk_command_pool);
}

void vk_copy_buffer_to_image(
    VkDevice vk_device, VkBuffer buffer, 
    VkImage image, uint32_t width, uint32_t height,
    VkQueue vk_graphics_queue, VkCommandPool vk_command_pool
) {
    VkCommandBuffer commandBuffer = vk_begin_single_time_commands(vk_device, vk_command_pool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vk_end_single_time_commands(vk_device, commandBuffer, vk_graphics_queue, vk_command_pool);
}

VkImageView vk_create_texture_imageview(VkDevice vk_device, VkImage vk_texture_image) {
    VkImageView vk_texture_imageview = vk_create_imageview(vk_device, vk_texture_image, VK_FORMAT_R8G8B8A8_SRGB);

	return vk_texture_imageview;
}

VkSampler vk_create_texture_sampler(VkPhysicalDevice vk_physical_device, VkDevice vk_device) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(vk_physical_device, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSampler vk_texture_sampler;
    if (vkCreateSampler(vk_device, &samplerInfo, nullptr, &vk_texture_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    return vk_texture_sampler;
}

VkImage vk_create_texture_image(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    int texWidth, int texHeight, int texChannels, uint8_t* testData, 
    VkQueue vk_graphics_queue, VkCommandPool vk_command_pool,
    VkDeviceMemory* vk_texture_image_memory
) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // Initialize the test texture data
    for (int i = 0; i < texWidth * texHeight; ++i) {
        if (i < (texWidth * texHeight / 2)) {
            testData[i * 4 + 0] = 255; // R
            testData[i * 4 + 1] = 0;   // G
            testData[i * 4 + 2] = 0;   // B
        }
        else
        {
            testData[i * 4 + 0] = 0;   // R
            testData[i * 4 + 1] = 255; // G
            testData[i * 4 + 2] = 0;   // B
        }

        testData[i * 4 + 3] = 255; // A
    }
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    vk_create_buffer(
        vk_physical_device, vk_device, 
        imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory
    );

    void* data;
    vkMapMemory(vk_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, testData, static_cast<size_t>(imageSize));
    vkUnmapMemory(vk_device, stagingBufferMemory);

    VkImage vk_texture_image;

    vk_create_image(
        vk_physical_device, vk_device, 
        texWidth, texHeight, 
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        &vk_texture_image, vk_texture_image_memory);

    vk_transition_image_layout(
        vk_device, vk_texture_image, 
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        vk_graphics_queue, vk_command_pool
    );

    vk_copy_buffer_to_image(
        vk_device, stagingBuffer, 
        vk_texture_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 
        vk_graphics_queue, vk_command_pool
    );

    vk_transition_image_layout(
        vk_device, vk_texture_image, 
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
        vk_graphics_queue, vk_command_pool
    );

    vkDestroyBuffer(vk_device, stagingBuffer, nullptr);
    vkFreeMemory(vk_device, stagingBufferMemory, nullptr);

	return vk_texture_image;
}
