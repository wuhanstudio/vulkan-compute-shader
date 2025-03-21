#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <fmt/core.h>
#include <fstream>
#include <vector>

#include "instance.h"
#include "device.h"
#include "framebuffer.h"
#include "swapchain.h"
#include "pipeline.h"
#include "command.h"

#include "vertex.h"
#include "texture.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Testing texture data
uint8_t testData[WIDTH * HEIGHT * 4];

GLFWwindow* gWindow;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
uint32_t currentFrame = 0;

static std::vector<char> read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void vk_create_sync_objects(VkDevice vk_device) {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(vk_device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vk_device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vk_device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void vk_draw_frame(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    VkSurfaceKHR vk_surface, VkSwapchainKHR vk_swap_chain, 
    VkExtent2D vk_swap_chain_extent, std::vector<VkImage> vk_swapchain_images,
    std::vector<VkImageView> vk_swapchain_imageviews, VkRenderPass vk_render_pass,
    VkPipelineLayout vk_pipeline_layout, VkPipeline vk_graphics_pipeline
) {
    vkWaitForFences(vk_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(vk_device, vk_swap_chain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vk_recreate_swapchain(
            vk_physical_device, vk_device, 
            vk_surface, gWindow, vk_swap_chain, 
            vk_swap_chain_extent, vk_swapchain_images, vk_swapchain_imageviews,
            vk_render_pass
        );
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(vk_device, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    vk_record_command_buffer(
        commandBuffers[currentFrame], imageIndex, 
        vk_swap_chain_extent, vk_render_pass, 
        vk_pipeline_layout, vk_graphics_pipeline
    );

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vk_graphics_queue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { vk_swap_chain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vk_present_queue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        vk_recreate_swapchain(
            vk_physical_device, vk_device, 
            vk_surface, gWindow, 
            vk_swap_chain, vk_swap_chain_extent, 
            vk_swapchain_images, vk_swapchain_imageviews, vk_render_pass
        );
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

        gWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetKeyCallback(gWindow, glfw_onKey);

		// Print available extensions
        vk_print_extensions();

        VkDebugUtilsMessengerEXT vk_debug_messenger;
        VkInstance vk_instance = vk_create_instance(&vk_debug_messenger);

        VkSurfaceKHR vk_surface = vk_create_surface(vk_instance, gWindow);

        VkPhysicalDevice vk_physical_device = vk_pick_physical_device(vk_instance, vk_surface);
        VkDevice vk_device = vk_create_logical_device(vk_physical_device, vk_surface);

        VkSwapchainKHR vk_swapchain = vk_create_swapchain(vk_physical_device, vk_device, vk_surface, gWindow);

        std::vector<VkImage> vk_swapchain_images = vk_create_swapchain_images(vk_device, vk_swapchain);
        std::vector<VkImageView> vk_swapchain_imageviews = vk_create_image_views(vk_device, vk_swapchain_images);
        VkRenderPass vk_render_pass = vk_create_render_pass(vk_device);

        auto vertShaderCode = read_file("shader/vert.spv");
        auto fragShaderCode = read_file("shader/frag.spv");
        VkDescriptorSetLayout vk_descriptor_set_layout = vk_create_descriptor_set_layout(vk_device);
        VkPipelineLayout vk_pipeline_layout = vk_create_pipeline_layout(vk_device, vk_descriptor_set_layout);

        VkPipeline vk_graphics_pipeline = vk_create_graphics_pipeline(
                                            vk_device, vertShaderCode, fragShaderCode, 
                                            vk_render_pass, vk_descriptor_set_layout, vk_pipeline_layout);

        SwapChainSupportDetails vk_swap_chain_support = vk_query_swapchain_support(vk_physical_device, vk_surface);
        VkExtent2D vk_swap_chain_extent = vk_choose_swap_extent(vk_swap_chain_support.capabilities, gWindow);
        vk_create_frame_buffers(vk_device, vk_swap_chain_extent, vk_swapchain_imageviews, vk_render_pass);

        vk_create_command_pool(vk_physical_device, vk_device, vk_surface);

        vk_create_texture_image(vk_physical_device, vk_device, WIDTH, HEIGHT, 4, testData);
        vk_create_texture_image_view(vk_device);
        vk_create_texture_sampler(vk_physical_device, vk_device);

        vk_create_vertex_buffer(vk_physical_device, vk_device);
        vk_create_index_buffer(vk_physical_device, vk_device);

        vk_create_descriptor_pool(vk_device);
        vk_update_descriptor_sets(vk_device, vk_descriptor_set_layout);

        vk_create_command_buffers(vk_device, vk_swap_chain_extent);

        vk_create_sync_objects(vk_device);

        while (!glfwWindowShouldClose(gWindow)) {
            glfwPollEvents();
            vk_draw_frame(
                vk_physical_device, vk_device, vk_surface, 
                vk_swapchain, vk_swap_chain_extent, 
                vk_swapchain_images, vk_swapchain_imageviews, 
                vk_render_pass, vk_pipeline_layout, vk_graphics_pipeline);
        }

        vkDeviceWaitIdle(vk_device);

        // Clean Up
        vk_cleanup_swap_chain(vk_device, vk_swapchain, vk_swapchain_imageviews);

        vkDestroyPipeline(vk_device, vk_graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, nullptr);
        vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);

        vkDestroyDescriptorPool(vk_device, descriptorPool, nullptr);

        vkDestroySampler(vk_device, textureSampler, nullptr);
        vkDestroyImageView(vk_device, textureImageView, nullptr);

        vkDestroyImage(vk_device, textureImage, nullptr);
        vkFreeMemory(vk_device, textureImageMemory, nullptr);

        vkDestroyDescriptorSetLayout(vk_device, vk_descriptor_set_layout, nullptr);

        vkDestroyBuffer(vk_device, indexBuffer, nullptr);
        vkFreeMemory(vk_device, indexBufferMemory, nullptr);

        vkDestroyBuffer(vk_device, vertexBuffer, nullptr);
        vkFreeMemory(vk_device, vertexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(vk_device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(vk_device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(vk_device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(vk_device, commandPool, nullptr);

        vkDestroyDevice(vk_device, nullptr);

        if (vk_check_validation_layer()) {
            vk_destroy_debug_utils_messenger_ext(vk_instance, vk_debug_messenger, nullptr);
        }

        vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
        vkDestroyInstance(vk_instance, nullptr);

        glfwDestroyWindow(gWindow);
        glfwTerminate();
    }
    catch (const std::exception& e) {
        fmt::print("Exception: {}\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
