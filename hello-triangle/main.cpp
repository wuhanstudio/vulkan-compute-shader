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
#include "pipline.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

GLFWwindow* window;

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
        createImageViews();
 
        createRenderPass();

        auto vertShaderCode = readFile("shader/triangle.vert.spv");
        auto fragShaderCode = readFile("shader/triangle.frag.spv");
        createGraphicsPipeline(vertShaderCode, fragShaderCode);
 
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

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

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
