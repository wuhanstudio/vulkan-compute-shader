#include "command.h"

#include <stdexcept>

VkCommandPool vk_create_command_pool(VkPhysicalDevice vk_physical_device, VkDevice vk_device, VkSurfaceKHR vk_surface) {
    QueueFamilyIndices queueFamilyIndices = vk_find_queue_families(vk_physical_device, vk_surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	VkCommandPool vk_command_pool;
    if (vkCreateCommandPool(vk_device, &poolInfo, nullptr, &vk_command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

	return vk_command_pool;
}

std::vector<VkCommandBuffer> vk_create_command_buffers(VkDevice vk_device, VkExtent2D vk_swap_chain_extent, VkCommandPool vk_command_pool) {
    std::vector<VkCommandBuffer> vk_command_buffers;
    vk_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vk_command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)vk_command_buffers.size();

    if (vkAllocateCommandBuffers(vk_device, &allocInfo, vk_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

	return vk_command_buffers;
}

void vk_record_command_buffer(
    VkCommandBuffer vk_command_buffer, uint32_t imageIndex, 
    VkExtent2D vk_swap_chain_extent, VkRenderPass vk_render_pass,
    VkPipelineLayout vk_pipeline_layout, VkPipeline vk_graphics_pipeline,
	std::vector<VkDescriptorSet> vk_descriptor_sets, uint32_t currentFrame
) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(vk_command_buffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vk_render_pass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = vk_swap_chain_extent;

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(vk_command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vk_swap_chain_extent.width;
    viewport.height = (float)vk_swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(vk_command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = vk_swap_chain_extent;
    vkCmdSetScissor(vk_command_buffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(vk_command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_descriptor_sets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(vk_command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(vk_command_buffer);

    if (vkEndCommandBuffer(vk_command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}