#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <fmt/core.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <set>

#include "instance.h"
#include "device.h"
#include "swapchain.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

GLFWwindow* window;

VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

VkCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
uint32_t currentFrame = 0;

bool framebufferResized = false;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    //auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    framebufferResized = true;
}

static std::vector<char> readFile(const std::string& filename) {
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

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
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

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    //void createGraphicsPipeline() {
    //    auto vertShaderCode = readFile("shader/triangle.vert.spv");
    //    auto fragShaderCode = readFile("shader/triangle.frag.spv");

    //    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    //    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    //    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    //    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    //    vertShaderStageInfo.module = vertShaderModule;
    //    vertShaderStageInfo.pName = "main";

    //    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    //    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    //    fragShaderStageInfo.module = fragShaderModule;
    //    fragShaderStageInfo.pName = "main";

    //    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    //    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    //    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //    vertexInputInfo.vertexBindingDescriptionCount = 0;
    //    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    //    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    //    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    //    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //    VkPipelineViewportStateCreateInfo viewportState{};
    //    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    //    viewportState.viewportCount = 1;
    //    viewportState.scissorCount = 1;

    //    VkPipelineRasterizationStateCreateInfo rasterizer{};
    //    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    //    rasterizer.depthClampEnable = VK_FALSE;
    //    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    //    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //    rasterizer.lineWidth = 1.0f;
    //    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    //    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //    rasterizer.depthBiasEnable = VK_FALSE;

    //    VkPipelineMultisampleStateCreateInfo multisampling{};
    //    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    //    multisampling.sampleShadingEnable = VK_FALSE;
    //    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    //    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    //    colorBlendAttachment.blendEnable = VK_FALSE;

    //    VkPipelineColorBlendStateCreateInfo colorBlending{};
    //    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    //    colorBlending.logicOpEnable = VK_FALSE;
    //    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    //    colorBlending.attachmentCount = 1;
    //    colorBlending.pAttachments = &colorBlendAttachment;
    //    colorBlending.blendConstants[0] = 0.0f;
    //    colorBlending.blendConstants[1] = 0.0f;
    //    colorBlending.blendConstants[2] = 0.0f;
    //    colorBlending.blendConstants[3] = 0.0f;

    //    std::vector<VkDynamicState> dynamicStates = {
    //        VK_DYNAMIC_STATE_VIEWPORT,
    //        VK_DYNAMIC_STATE_SCISSOR
    //    };
    //    VkPipelineDynamicStateCreateInfo dynamicState{};
    //    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    //    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    //    dynamicState.pDynamicStates = dynamicStates.data();

    //    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    //    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //    pipelineLayoutInfo.setLayoutCount = 0;
    //    pipelineLayoutInfo.pushConstantRangeCount = 0;

    //    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create pipeline layout!");
    //    }

    //    VkGraphicsPipelineCreateInfo pipelineInfo{};
    //    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //    pipelineInfo.stageCount = 2;
    //    pipelineInfo.pStages = shaderStages;
    //    pipelineInfo.pVertexInputState = &vertexInputInfo;
    //    pipelineInfo.pInputAssemblyState = &inputAssembly;
    //    pipelineInfo.pViewportState = &viewportState;
    //    pipelineInfo.pRasterizationState = &rasterizer;
    //    pipelineInfo.pMultisampleState = &multisampling;
    //    pipelineInfo.pColorBlendState = &colorBlending;
    //    pipelineInfo.pDynamicState = &dynamicState;
    //    pipelineInfo.layout = pipelineLayout;
    //    pipelineInfo.renderPass = renderPass;
    //    pipelineInfo.subpass = 0;
    //    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    //    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create graphics pipeline!");
    //    }

    //    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    //    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    //}

    //void createFramebuffers() {
    //    swapChainFramebuffers.resize(swapChainImageViews.size());

    //    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    //        VkImageView attachments[] = {
    //            swapChainImageViews[i]
    //        };

    //        VkFramebufferCreateInfo framebufferInfo{};
    //        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //        framebufferInfo.renderPass = renderPass;
    //        framebufferInfo.attachmentCount = 1;
    //        framebufferInfo.pAttachments = attachments;
    //        framebufferInfo.width = swapChainExtent.width;
    //        framebufferInfo.height = swapChainExtent.height;
    //        framebufferInfo.layers = 1;

    //        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
    //            throw std::runtime_error("failed to create framebuffer!");
    //        }
    //    }
    //}

    //void createCommandPool() {
    //    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    //    VkCommandPoolCreateInfo poolInfo{};
    //    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    //    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    //    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create command pool!");
    //    }
    //}

    void createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    //void drawFrame() {
    //    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    //    uint32_t imageIndex;
    //    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    //    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    //        recreateSwapChain();
    //        return;
    //    }
    //    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    //        throw std::runtime_error("failed to acquire swap chain image!");
    //    }

    //    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    //    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    //    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    //    VkSubmitInfo submitInfo{};
    //    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    //    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    //    submitInfo.waitSemaphoreCount = 1;
    //    submitInfo.pWaitSemaphores = waitSemaphores;
    //    submitInfo.pWaitDstStageMask = waitStages;

    //    submitInfo.commandBufferCount = 1;
    //    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    //    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    //    submitInfo.signalSemaphoreCount = 1;
    //    submitInfo.pSignalSemaphores = signalSemaphores;

    //    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to submit draw command buffer!");
    //    }

    //    VkPresentInfoKHR presentInfo{};
    //    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    //    presentInfo.waitSemaphoreCount = 1;
    //    presentInfo.pWaitSemaphores = signalSemaphores;

    //    VkSwapchainKHR swapChains[] = { swapChain };
    //    presentInfo.swapchainCount = 1;
    //    presentInfo.pSwapchains = swapChains;

    //    presentInfo.pImageIndices = &imageIndex;

    //    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    //    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
    //        framebufferResized = false;
    //        recreateSwapChain();
    //    }
    //    else if (result != VK_SUCCESS) {
    //        throw std::runtime_error("failed to present swap chain image!");
    //    }

    //    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    //}

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

// Press ESC to close the window
static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

int main() {
    try {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetKeyCallback(window, glfw_onKey);

        //glfwSetWindowUserPointer(window, this);
        //glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

        printExtensions();
        createInstance();

        createSurface(window);
        pickPhysicalDevice();
        createLogicalDevice();

        createSwapChain(window);
        //createImageViews();
 
        //createRenderPass();
        //createGraphicsPipeline();
 
        //createFramebuffers();
        //createCommandPool();
        //createCommandBuffers();

        //createSyncObjects();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            //drawFrame();
        }

        vkDeviceWaitIdle(device);

        // Clean Up
        cleanupSwapChain();

        //vkDestroyPipeline(device, graphicsPipeline, nullptr);
        //vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        //vkDestroyRenderPass(device, renderPass, nullptr);

        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //    vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        //    vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        //    vkDestroyFence(device, inFlightFences[i], nullptr);
        //}

        //vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (const std::exception& e) {
		fmt::print("Exception: {}\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
