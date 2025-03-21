#include "framebuffer.h"

#include <stdexcept>

std::vector<VkFramebuffer> swapChainFramebuffers;

void vk_create_frame_buffers(VkDevice vk_device, VkExtent2D vk_swap_chain_extent) {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vk_swap_chain_extent.width;
        framebufferInfo.height = vk_swap_chain_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vk_device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

