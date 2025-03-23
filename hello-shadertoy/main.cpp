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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

struct UniformBufferObject {
    glm::vec4 iMouse;
    alignas(16) float iTime;
    alignas(16) glm::vec3 iResolution;
};

GLFWwindow* gWindow;

std::vector<VkSemaphore> vk_image_available_semaphores;
std::vector<VkSemaphore> vk_render_finished_semaphores;
std::vector<VkFence> vk_in_flight_fences;
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
    vk_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    vk_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    vk_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(vk_device, &semaphoreInfo, nullptr, &vk_image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vk_device, &semaphoreInfo, nullptr, &vk_render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vk_device, &fenceInfo, nullptr, &vk_in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

VkDescriptorPool vk_create_descriptor_pool(VkDevice vk_device) {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPool vk_descriptor_pool;
    if (vkCreateDescriptorPool(vk_device, &poolInfo, nullptr, &vk_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    return vk_descriptor_pool;
}

VkDescriptorSetLayout vk_create_descriptor_set_layout(VkDevice vk_device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

	VkDescriptorSetLayout vk_descriptor_set_layout;
    if (vkCreateDescriptorSetLayout(vk_device, &layoutInfo, nullptr, &vk_descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

    return vk_descriptor_set_layout;
}

std::vector<VkDescriptorSet> vk_create_descriptor_sets(
    VkDevice vk_device, 
    VkDescriptorSetLayout vk_descriptor_set_layout, 
    VkDescriptorPool vk_descriptor_pool,
    std::vector<VkBuffer> vk_uniform_buffers
) {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, vk_descriptor_set_layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vk_descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> vk_descriptor_sets;
    vk_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(vk_device, &allocInfo, vk_descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = vk_uniform_buffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = vk_descriptor_sets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(vk_device, 1, &descriptorWrite, 0, nullptr);
    }

	return vk_descriptor_sets;
}

void vk_draw_frame(
    VkPhysicalDevice vk_physical_device, VkDevice vk_device, 
    VkSurfaceKHR vk_surface, VkSwapchainKHR vk_swap_chain, 
    VkExtent2D vk_swap_chain_extent, std::vector<VkImage> vk_swapchain_images,
    std::vector<VkImageView> vk_swapchain_imageviews, VkRenderPass vk_render_pass,
    VkPipelineLayout vk_pipeline_layout, VkPipeline vk_graphics_pipeline,
	VkFormat vk_swapchain_image_format, std::vector<VkDescriptorSet> vk_descriptor_sets,
	VkQueue vk_graphics_queue, VkQueue vk_present_queue, std::vector<VkCommandBuffer> vk_command_buffers,
    std::vector<VkFramebuffer> vk_swapchain_framebuffers,
	VkBuffer vk_vertex_buffer, VkBuffer vk_index_buffer
) {
    vkWaitForFences(vk_device, 1, &vk_in_flight_fences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(vk_device, vk_swap_chain, UINT64_MAX, vk_image_available_semaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vk_recreate_swapchain(
            vk_physical_device, vk_device, 
            vk_surface, gWindow, vk_swap_chain, 
            vk_swap_chain_extent, vk_swapchain_images, vk_swapchain_imageviews,
            vk_render_pass, vk_swapchain_image_format, vk_swapchain_framebuffers
        );
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(vk_device, 1, &vk_in_flight_fences[currentFrame]);

    vkResetCommandBuffer(vk_command_buffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    vk_record_command_buffer(
        vk_command_buffers[currentFrame], imageIndex, 
        vk_swap_chain_extent, vk_render_pass, 
        vk_pipeline_layout, vk_graphics_pipeline,
		vk_descriptor_sets, currentFrame, vk_swapchain_framebuffers,
		vk_vertex_buffer, vk_index_buffer
    );

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { vk_image_available_semaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_command_buffers[currentFrame];

    VkSemaphore signalSemaphores[] = { vk_render_finished_semaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vk_graphics_queue, 1, &submitInfo, vk_in_flight_fences[currentFrame]) != VK_SUCCESS) {
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
            vk_swapchain_images, vk_swapchain_imageviews, vk_render_pass, vk_swapchain_image_format,
            vk_swapchain_framebuffers
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

void glfw_show_fps(GLFWwindow* window) {
    static double previousSeconds = 0.0;
    static int frameCount = 0;
    double elapsedSeconds;
    double currentSeconds = glfwGetTime();

    elapsedSeconds = currentSeconds - previousSeconds;

    if (elapsedSeconds > 0.25) {
        previousSeconds = currentSeconds;
        double fps = (double)frameCount / elapsedSeconds;
        double msPerFrame = 1000.0 / fps;

        char title[80];
        std::snprintf(title, sizeof(title), "Hello Shadertoy @ fps: %.2f, ms/frame: %.2f", fps, msPerFrame);
        glfwSetWindowTitle(window, title);

        frameCount = 0;
    }

    frameCount++;
}

int main() {
    try {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        gWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetKeyCallback(gWindow, glfw_onKey);

		// Print available extensions
        vk_print_extensions();

		// Create Vulkan Instance
        VkDebugUtilsMessengerEXT vk_debug_messenger;
        VkInstance vk_instance = vk_create_instance(vk_debug_messenger);

		// Create Surface
        VkSurfaceKHR vk_surface = vk_create_surface(vk_instance, gWindow);

		// Create Physical Device and Logical Device
        VkPhysicalDevice vk_physical_device = vk_pick_physical_device(vk_instance, vk_surface);

		VkQueue vk_graphics_queue, vk_present_queue;
        VkDevice vk_device = vk_create_logical_device(vk_physical_device, vk_surface, vk_graphics_queue, vk_present_queue);

		// Create Swap Chain
        SwapChainSupportDetails vk_swap_chain_support = vk_query_swapchain_support(vk_physical_device, vk_surface);
        
        VkSurfaceFormatKHR surfaceFormat = vk_choose_swap_surface_format(vk_swap_chain_support.formats);
        VkFormat vk_swapchain_image_format = surfaceFormat.format;

        VkExtent2D vk_swap_chain_extent = vk_choose_swap_extent(vk_swap_chain_support.capabilities, gWindow);

        VkSwapchainKHR vk_swapchain = vk_create_swapchain(vk_physical_device, vk_device, vk_surface, gWindow);
        std::vector<VkImage> vk_swapchain_images = vk_create_swapchain_images(vk_device, vk_swapchain);
        std::vector<VkImageView> vk_swapchain_imageviews = vk_create_image_views(
            vk_device,
            vk_swapchain_images,
            vk_swapchain_image_format
        );

		// Create Render Pass
        VkRenderPass vk_render_pass = vk_create_render_pass(vk_device, vk_swapchain_image_format);

		// Create Graphics Pipeline
        auto vertShaderCode = read_file("shader/main_vert.spv");
        auto fragShaderCode = read_file("shader/fireball_frag.spv");
        VkDescriptorSetLayout vk_descriptor_set_layout = vk_create_descriptor_set_layout(vk_device);
        VkPipelineLayout vk_pipeline_layout = vk_create_pipeline_layout(vk_device, vk_descriptor_set_layout);

        VkPipeline vk_graphics_pipeline = vk_create_graphics_pipeline(
                                            vk_device, 
                                            vertShaderCode, fragShaderCode, 
                                            vk_render_pass, 
                                            vk_descriptor_set_layout, vk_pipeline_layout);

		// Create Command Pool and Command Buffers
        VkCommandPool vk_command_pool = vk_create_command_pool(vk_physical_device, vk_device, vk_surface);
        std::vector<VkCommandBuffer> vk_command_buffers = vk_create_command_buffers(vk_device, vk_swap_chain_extent, vk_command_pool);

		// Create Vertex Buffer
        VkDeviceMemory vk_vertex_buffer_memory;
        VkBuffer vk_vertex_buffer = vk_create_vertex_buffer(
            vk_physical_device, vk_device, 
            vk_vertex_buffer_memory
        );
        
		// Create Index Buffer
        VkDeviceMemory vk_index_buffer_memory;
        VkBuffer vk_index_buffer = vk_create_index_buffer(
            vk_physical_device, vk_device, 
            vk_graphics_queue, vk_command_pool, 
            vk_index_buffer_memory
        );

        // Create Uniform Buffer
        std::vector<VkBuffer> vk_uniform_buffers;
        std::vector<VkDeviceMemory> vk_uniform_buffers_memory;
        std::vector<void*> vk_uniform_buffers_mapped;

        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        vk_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        vk_uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
        vk_uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk_create_buffer(
				vk_physical_device, vk_device,
                bufferSize, 
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                vk_uniform_buffers[i], 
                vk_uniform_buffers_memory[i]
            );

            vkMapMemory(vk_device, vk_uniform_buffers_memory[i], 0, bufferSize, 0, &vk_uniform_buffers_mapped[i]);
        }

        // Create Descriptor Pool and Descriptor Sets
        VkDescriptorPool vk_descriptor_pool = vk_create_descriptor_pool(vk_device);
        std::vector<VkDescriptorSet> vk_descriptor_sets = vk_create_descriptor_sets(
            vk_device, 
            vk_descriptor_set_layout, vk_descriptor_pool,
            vk_uniform_buffers
            );

		// Create Sync Objects
        vk_create_sync_objects(vk_device);

		// Create Framebuffers
        std::vector<VkFramebuffer> vk_swapchain_framebuffers = vk_create_frame_buffers(vk_device, vk_swap_chain_extent, vk_swapchain_imageviews, vk_render_pass);

		double start_time = glfwGetTime();
        while (!glfwWindowShouldClose(gWindow)) {
            glfwPollEvents();
            glfw_show_fps(gWindow);
            
			// Update Uniform Buffer
            float iTime = glfwGetTime() - start_time;

            UniformBufferObject ubo{};
            ubo.iMouse = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            ubo.iTime = iTime;
            ubo.iResolution = glm::vec3(WIDTH, HEIGHT, 0);

            memcpy(vk_uniform_buffers_mapped[currentFrame], &ubo, sizeof(ubo));

            vk_draw_frame(
                vk_physical_device, vk_device, 
                vk_surface, 
                vk_swapchain, vk_swap_chain_extent, 
                vk_swapchain_images, vk_swapchain_imageviews, 
                vk_render_pass, 
                vk_pipeline_layout, vk_graphics_pipeline, 
                vk_swapchain_image_format, vk_descriptor_sets,
                vk_graphics_queue, vk_present_queue, 
                vk_command_buffers, vk_swapchain_framebuffers,
				vk_vertex_buffer, vk_index_buffer
                );
        }

        vkDeviceWaitIdle(vk_device);

        // Clean Up
        vk_cleanup_swap_chain(vk_device, vk_swapchain, vk_swapchain_imageviews, vk_swapchain_framebuffers);

        vkDestroyPipeline(vk_device, vk_graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, nullptr);
        vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(vk_device, vk_uniform_buffers[i], nullptr);
            vkFreeMemory(vk_device, vk_uniform_buffers_memory[i], nullptr);
        }

        vkDestroyDescriptorPool(vk_device, vk_descriptor_pool, nullptr);

        vkDestroyDescriptorSetLayout(vk_device, vk_descriptor_set_layout, nullptr);

        vkDestroyBuffer(vk_device, vk_index_buffer, nullptr);
        vkFreeMemory(vk_device, vk_index_buffer_memory, nullptr);

        vkDestroyBuffer(vk_device, vk_vertex_buffer, nullptr);
        vkFreeMemory(vk_device, vk_vertex_buffer_memory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(vk_device, vk_render_finished_semaphores[i], nullptr);
            vkDestroySemaphore(vk_device, vk_image_available_semaphores[i], nullptr);
            vkDestroyFence(vk_device, vk_in_flight_fences[i], nullptr);
        }

        vkDestroyCommandPool(vk_device, vk_command_pool, nullptr);

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
