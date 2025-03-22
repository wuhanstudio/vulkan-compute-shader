#include "imageview.h"

#include <vector>
#include <stdexcept>

std::vector<VkImageView> vk_create_image_views(
    VkDevice vk_device, 
    std::vector<VkImage> vk_swap_chain_images,
    VkFormat vk_swapchain_image_format
) {
    std::vector<VkImageView> vk_swapchain_imageviews;
    vk_swapchain_imageviews.resize(vk_swap_chain_images.size());

    for (size_t i = 0; i < vk_swap_chain_images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vk_swap_chain_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vk_swapchain_image_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vk_device, &createInfo, nullptr, &vk_swapchain_imageviews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

	return vk_swapchain_imageviews;
}
