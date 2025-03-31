#include "app.h"

void VulkanParticleApp::vk_create_command_pool() {
    QueueFamilyIndices queueFamilyIndices = vk_find_queue_families(vk_physical_device);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

    if (vkCreateCommandPool(vk_device, &poolInfo, nullptr, &vk_command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void VulkanParticleApp::vk_create_graphics_command_buffers() {
    vk_graphics_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vk_command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)vk_graphics_command_buffers.size();

    if (vkAllocateCommandBuffers(vk_device, &allocInfo, vk_graphics_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanParticleApp::vk_create_lbm_compute_command_buffers() {
    vk_lbm_compute_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vk_command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)vk_lbm_compute_command_buffers.size();

    if (vkAllocateCommandBuffers(vk_device, &allocInfo, vk_lbm_compute_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute command buffers!");
    }
}

void VulkanParticleApp::vk_create_particle_compute_command_buffers() {
    vk_particle_compute_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vk_command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)vk_particle_compute_command_buffers.size();

    if (vkAllocateCommandBuffers(vk_device, &allocInfo, vk_particle_compute_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute command buffers!");
    }
}

void VulkanParticleApp::vk_record_graphics_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, int num_obstacle) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vk_render_pass;
    renderPassInfo.framebuffer = vk_swapchain_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = vk_swapchain_extent;

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    VkRect2D scissor{};

    if (num_obstacle > 0) {

        // Draw Obstacles
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_obstacle_graphics_pipeline);

        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)vk_swapchain_extent.width;
        viewport.height = (float)vk_swapchain_extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        scissor.offset = { 0, 0 };
        scissor.extent = vk_swapchain_extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { vk_obstacle_vertex_buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }

    // Draw particles
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_particle_graphics_pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_particle_graphics_pipeline_layout, 0, 1, &vk_particle_graphics_descriptor_sets[currentFrame], 0, nullptr);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vk_swapchain_extent.width;
    viewport.height = (float)vk_swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    scissor.offset = { 0, 0 };
    scissor.extent = vk_swapchain_extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}


void VulkanParticleApp::vk_record_lbm_compute_command_buffer(VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vk_lbm_compute_pipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vk_lbm_compute_pipeline_layout, 0, 1, &vk_lbm_compute_descriptor_sets[currentFrame], 0, nullptr);

    vkCmdDispatch(commandBuffer, NX / 10, NY / 10, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record LBM compute command buffer!");
    }
}

void VulkanParticleApp::vk_record_particle_compute_command_buffer(VkCommandBuffer commandBuffer) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Dailed to begin recording compute command buffer!");
	}

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vk_particle_compute_pipeline);
	
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vk_particle_compute_pipeline_layout, 0, 1, &vk_particle_compute_descriptor_sets[currentFrame], 0, nullptr);
	
    vkCmdDispatch(commandBuffer, NUM_PARTICLE / 1000, 1, 1);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record particle compute command buffer!");
	}
}