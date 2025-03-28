#include "app.h"

std::vector<char> VulkanParticleApp::read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

/*--------------------- Update obstacle flags -------------------------------------------------------------*/
void VulkanParticleApp::lbm_update_obstacle(void)
{
    VkDeviceSize bufferSize = sizeof(float) * NX * NY * NUM_VECTORS;

    // Create a staging buffer used to upload data to the gpu
    VkBuffer dcf_Buffer;
    VkDeviceMemory dcf_BufferMemory;
    vk_create_buffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        dcf_Buffer,
        dcf_BufferMemory
    );

    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, cF_SSB);
    //int* F_temp = (int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NX * NY * sizeof(int), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    void* dcu_temp;
    vkMapMemory(vk_device, dcf_BufferMemory, 0, bufferSize, 0, &dcu_temp);

    float* F_temp = (float*)dcu_temp;

    for (int x = 0; x < NX; x++)
        for (int y = 0; y < NY; y++)
        {
            int xx = x - (xMouse) * NX / 2.0;
            int yy = y - (yMouse) * NY / 2.0;
            int idx = x + y * NX;
            if (idx > NX * NY)	break;
            if (sqrt(float((xx - NX / 2) * (xx - NX / 2) + (yy - NY / 2) * (yy - NY / 2))) < NX / 14)
            {
                F_cpu[idx] = 0;
                F_temp[idx] = 0;
            }
            else
            {
                F_cpu[idx] = 1;
                F_temp[idx] = 1;
            }
        }

	// Clear the boundary
    for (int x = 0; x < NX; x++)
        F_temp[x + 0 * NX] = F_temp[x + (NY - 1) * NX] = 0;

    vkUnmapMemory(vk_device, dcf_BufferMemory);

    //glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    vk_dcf_storage_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk_dcf_storage_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);

    // Copy initial data to storage buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk_create_buffer(bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vk_dcf_storage_buffers[i],
            vk_dcf_storage_buffers_memory[i]
        );
        vk_copy_buffer(dcf_Buffer, vk_dcf_storage_buffers[i], bufferSize);
    }

    vkDestroyBuffer(vk_device, dcf_Buffer, nullptr);
    vkFreeMemory(vk_device, dcf_BufferMemory, nullptr);
}

void VulkanParticleApp::lbm_init_ssb(void)
{
    VkDeviceSize bufferSize = sizeof(float) * NX * NY * NUM_VECTORS;

    // Create a staging buffer used to upload data to the gpu
    VkBuffer dcu_Buffer;
    VkDeviceMemory dcu_BufferMemory;
    vk_create_buffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        dcu_Buffer,
        dcu_BufferMemory
    );

    void* dcu_data;
    vkMapMemory(vk_device, dcu_BufferMemory, 0, bufferSize, 0, &dcu_data);

    float* temp = (float*)dcu_data;
    for (int y = 0; y < NY; y++)
        for (int x = 0; x < NX; x++)
            temp[x + y * NX] = 0.0;
    vkUnmapMemory(vk_device, dcu_BufferMemory);

    vk_dcu_storage_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk_dcu_storage_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);

    // Copy initial data to storage buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk_create_buffer(bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vk_dcu_storage_buffers[i],
            vk_dcu_storage_buffers_memory[i]
        );
        vk_copy_buffer(dcu_Buffer, vk_dcu_storage_buffers[i], bufferSize);
    }

    vkDestroyBuffer(vk_device, dcu_Buffer, nullptr);
    vkFreeMemory(vk_device, dcu_BufferMemory, nullptr);

    // Create a staging buffer used to upload data to the gpu
    VkBuffer dcv_Buffer;
    VkDeviceMemory dcv_BufferMemory;
    vk_create_buffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        dcv_Buffer,
        dcv_BufferMemory
    );

    void* dcv_data;
    vkMapMemory(vk_device, dcv_BufferMemory, 0, bufferSize, 0, &dcv_data);

    float* dcv_temp = (float*)dcv_data;
    for (int y = 0; y < NY; y++)
        for (int x = 0; x < NX; x++)
            dcv_temp[x + y * NX] = 0.0;
    vkUnmapMemory(vk_device, dcv_BufferMemory);

    vk_dcv_storage_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk_dcv_storage_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);

    // Copy initial data to storage buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk_create_buffer(bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vk_dcv_storage_buffers[i],
            vk_dcv_storage_buffers_memory[i]
        );
        vk_copy_buffer(dcv_Buffer, vk_dcv_storage_buffers[i], bufferSize);
    }

    vkDestroyBuffer(vk_device, dcv_Buffer, nullptr);
    vkFreeMemory(vk_device, dcv_BufferMemory, nullptr);
}

void VulkanParticleApp::vk_create_render_pass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vk_swapchain_image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vk_device, &renderPassInfo, nullptr, &vk_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanParticleApp::vk_create_framebuffers() {
    vk_swapchain_framebuffers.resize(vk_swapchain_imageviews.size());

    for (size_t i = 0; i < vk_swapchain_imageviews.size(); i++) {
        VkImageView attachments[] = {
            vk_swapchain_imageviews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vk_render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vk_swapchain_extent.width;
        framebufferInfo.height = vk_swapchain_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vk_device, &framebufferInfo, nullptr, &vk_swapchain_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

VkShaderModule VulkanParticleApp::vk_create_shader_module(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(vk_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void VulkanParticleApp::vk_create_lbm_shader_storage_buffers() {

    /*---------------------- Initialise LBM vector state as SSB on GPU --------------------------------------*/
    float w[] = { 
        (4.0 / 9.0),
        (1.0 / 9.0),
        (1.0 / 9.0),
        (1.0 / 9.0),
        (1.0 / 9.0),
        (1.0 / 36.0),
        (1.0 / 36.0),
        (1.0 / 36.0),
        (1.0 / 36.0)
    };

    VkDeviceSize bufferSize = sizeof(float) * NX * NY * NUM_VECTORS;

    float* temp = (float*)malloc(bufferSize);
    for (int k = 0; k < NUM_VECTORS; k++)
        for (int y = 0; y < NY; y++)
            for (int x = 0; x < NX; x++)
                temp[k + x * NUM_VECTORS + y * NX * NUM_VECTORS] = w[k];

    // Create a staging buffer used to upload data to the gpu
    VkBuffer df0_Buffer;
    VkDeviceMemory df0_BufferMemory;
    vk_create_buffer(bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        df0_Buffer, 
        df0_BufferMemory
    );

    void* df0_data;
    vkMapMemory(vk_device, df0_BufferMemory, 0, bufferSize, 0, &df0_data);
    memcpy(df0_data, temp, (size_t)bufferSize);
    vkUnmapMemory(vk_device, df0_BufferMemory);

    vk_df0_storage_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk_df0_storage_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);

    // Copy initial data to storage buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk_create_buffer(bufferSize, 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            vk_df0_storage_buffers[i], 
            vk_df0_storage_buffers_memory[i]
        );
        vk_copy_buffer(df0_Buffer, vk_df0_storage_buffers[i], bufferSize);
    }

    vkDestroyBuffer(vk_device, df0_Buffer, nullptr);
    vkFreeMemory(vk_device, df0_BufferMemory, nullptr);

    // Create a staging buffer used to upload data to the gpu
    VkBuffer df1_Buffer;
    VkDeviceMemory df1_BufferMemory;
    vk_create_buffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        df1_Buffer,
        df1_BufferMemory
    );

    void* df1_data;
    vkMapMemory(vk_device, df1_BufferMemory, 0, bufferSize, 0, &df1_data);
    memcpy(df1_data, temp, (size_t)bufferSize);
    vkUnmapMemory(vk_device, df1_BufferMemory);

    vk_df1_storage_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk_df1_storage_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);

    // Copy initial data to storage buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk_create_buffer(bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vk_df1_storage_buffers[i],
            vk_df1_storage_buffers_memory[i]
        );
        vk_copy_buffer(df1_Buffer, vk_df1_storage_buffers[i], bufferSize);
    }

    vkDestroyBuffer(vk_device, df1_Buffer, nullptr);
    vkFreeMemory(vk_device, df1_BufferMemory, nullptr);

	lbm_update_obstacle();

    lbm_init_ssb();
}

void VulkanParticleApp::vk_create_uniform_buffers() {
    VkDeviceSize bufferSize = sizeof(LBMUniformBufferObject);

    vk_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk_uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
    vk_uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk_create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vk_uniform_buffers[i], vk_uniform_buffers_memory[i]);

        vkMapMemory(vk_device, vk_uniform_buffers_memory[i], 0, bufferSize, 0, &vk_uniform_buffers_mapped[i]);
    }
}

void VulkanParticleApp::vk_update_uniform_buffer(uint32_t currentImage) {
    LBMUniformBufferObject ubo{};
    ubo.deltaTime = lastFrameTime * 2.0f;

    memcpy(vk_uniform_buffers_mapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanParticleApp::vk_create_descriptor_pool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 5;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(vk_device, &poolInfo, nullptr, &vk_lbm_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanParticleApp::vk_create_compute_descriptor_sets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, vk_lbm_compute_descriptor_set_layout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vk_lbm_descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    vk_lbm_compute_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(vk_device, &allocInfo, vk_lbm_compute_descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = vk_uniform_buffers[i];
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(LBMUniformBufferObject);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = vk_lbm_compute_descriptor_sets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

        VkDescriptorBufferInfo storageBufferInfoDF0{};
        storageBufferInfoDF0.buffer = vk_df0_storage_buffers[(i - 1) % MAX_FRAMES_IN_FLIGHT];
        storageBufferInfoDF0.offset = 0;
        storageBufferInfoDF0.range = sizeof(float) * NX * NY;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = vk_lbm_compute_descriptor_sets[i];
        descriptorWrites[1].dstBinding = 0;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &storageBufferInfoDF0;

        VkDescriptorBufferInfo storageBufferInfoDF1{};
        storageBufferInfoDF1.buffer = vk_df0_storage_buffers[i];
        storageBufferInfoDF1.offset = 0;
        storageBufferInfoDF1.range = sizeof(float) * NX * NY;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = vk_lbm_compute_descriptor_sets[i];
        descriptorWrites[2].dstBinding = 1;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &storageBufferInfoDF1;

        VkDescriptorBufferInfo storageBufferInfoDCF{};
        storageBufferInfoDCF.buffer = vk_df0_storage_buffers[i];
        storageBufferInfoDCF.offset = 0;
        storageBufferInfoDCF.range = sizeof(float) * NX * NY;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = vk_lbm_compute_descriptor_sets[i];
        descriptorWrites[3].dstBinding = 2;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &storageBufferInfoDCF;

        VkDescriptorBufferInfo storageBufferInfoDCU{};
        storageBufferInfoDCU.buffer = vk_df0_storage_buffers[i];
        storageBufferInfoDCU.offset = 0;
        storageBufferInfoDCU.range = sizeof(float) * NX * NY;

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = vk_lbm_compute_descriptor_sets[i];
        descriptorWrites[4].dstBinding = 3;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pBufferInfo = &storageBufferInfoDCU;

        VkDescriptorBufferInfo storageBufferInfoDCV{};
        storageBufferInfoDCV.buffer = vk_df0_storage_buffers[i];
        storageBufferInfoDCV.offset = 0;
        storageBufferInfoDCV.range = sizeof(float) * NX * NY;

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = vk_lbm_compute_descriptor_sets[i];
        descriptorWrites[5].dstBinding = 4;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pBufferInfo = &storageBufferInfoDCV;

        vkUpdateDescriptorSets(vk_device, 6, descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanParticleApp::vk_create_lbm_compute_descriptor_set_layout() {
    std::array<VkDescriptorSetLayoutBinding, 6> layoutBindings{};

    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].pImmutableSamplers = nullptr;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[1].binding = 0;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[1].pImmutableSamplers = nullptr;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[2].binding = 1;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[2].pImmutableSamplers = nullptr;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[3].binding = 2;
    layoutBindings[3].descriptorCount = 1;
    layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[3].pImmutableSamplers = nullptr;
    layoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[4].binding = 3;
    layoutBindings[4].descriptorCount = 1;
    layoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[4].pImmutableSamplers = nullptr;
    layoutBindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[5].binding = 4;
    layoutBindings[5].descriptorCount = 1;
    layoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[5].pImmutableSamplers = nullptr;
    layoutBindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 6;
    layoutInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(vk_device, &layoutInfo, nullptr, &vk_lbm_compute_descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute descriptor set layout!");
    }
}

void VulkanParticleApp::vk_create_sync_objects() {
    vk_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    vk_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    vk_compute_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    vk_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    vk_compute_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(vk_device, &semaphoreInfo, nullptr, &vk_image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vk_device, &semaphoreInfo, nullptr, &vk_render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vk_device, &fenceInfo, nullptr, &vk_in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics synchronization objects for a frame!");
        }
        if (vkCreateSemaphore(vk_device, &semaphoreInfo, nullptr, &vk_compute_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vk_device, &fenceInfo, nullptr, &vk_compute_in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute synchronization objects for a frame!");
        }
    }
}

void VulkanParticleApp::vk_cleanup() {
    vk_cleanup_swapchain();

    vkDestroyPipeline(vk_device, vk_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(vk_device, vk_graphics_pipeline_layout, nullptr);

    vkDestroyPipeline(vk_device, vk_lbm_compute_pipeline, nullptr);
    vkDestroyPipelineLayout(vk_device, vk_lbm_compute_pipeline_layout, nullptr);

    vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(vk_device, vk_uniform_buffers[i], nullptr);
        vkFreeMemory(vk_device, vk_uniform_buffers_memory[i], nullptr);
    }

    vkDestroyDescriptorPool(vk_device, vk_lbm_descriptor_pool, nullptr);

    vkDestroyDescriptorSetLayout(vk_device, vk_lbm_compute_descriptor_set_layout, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(vk_device, vk_df0_storage_buffers[i], nullptr);
        vkFreeMemory(vk_device, vk_df0_storage_buffers_memory[i], nullptr);

        vkDestroyBuffer(vk_device, vk_df1_storage_buffers[i], nullptr);
        vkFreeMemory(vk_device, vk_df1_storage_buffers_memory[i], nullptr);

        vkDestroyBuffer(vk_device, vk_dcf_storage_buffers[i], nullptr);
        vkFreeMemory(vk_device, vk_dcf_storage_buffers_memory[i], nullptr);
    
        vkDestroyBuffer(vk_device, vk_dcu_storage_buffers[i], nullptr);
        vkFreeMemory(vk_device, vk_dcu_storage_buffers_memory[i], nullptr);

        vkDestroyBuffer(vk_device, vk_dcv_storage_buffers[i], nullptr);
        vkFreeMemory(vk_device, vk_dcv_storage_buffers_memory[i], nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vk_device, vk_render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(vk_device, vk_image_available_semaphores[i], nullptr);
        vkDestroySemaphore(vk_device, vk_compute_finished_semaphores[i], nullptr);
        vkDestroyFence(vk_device, vk_in_flight_fences[i], nullptr);
        vkDestroyFence(vk_device, vk_compute_in_flight_fences[i], nullptr);
    }

    vkDestroyCommandPool(vk_device, vk_command_pool, nullptr);

    vkDestroyDevice(vk_device, nullptr);

    if (vk_check_validation_layer_support()) {
        vk_destroy_debug_utils_messenger_ext(nullptr);
    }

    vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
    vkDestroyInstance(vk_instance, nullptr);

    glfwDestroyWindow(gWindow);

    glfwTerminate();
}

