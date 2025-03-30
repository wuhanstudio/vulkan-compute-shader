#include <fmt/core.h>
#include "app.h"

int mousedown = 0;

int gWindowWidth = 800;
int gWindowHeight = 600;

void glfw_onFramebufferSize(GLFWwindow* window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;
}

// Press ESC to close the window
static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
};

void glfw_onMouse(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (GLFW_PRESS == action)
        {
            mousedown = 1;
        }
        else if (GLFW_RELEASE == action)
            mousedown = 0;
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
        std::snprintf(title, sizeof(title), "Hello Vulkan @ fps: %.2f, ms/frame: %.2f", fps, msPerFrame);
        glfwSetWindowTitle(window, title);

        frameCount = 0;
    }

    frameCount++;
}

void VulkanParticleApp::vk_init_window() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(gWindow, this);

    glfwSetKeyCallback(gWindow, glfw_onKey);
    glfwSetMouseButtonCallback(gWindow, glfw_onMouse);

    glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);
}

void VulkanParticleApp::vk_init() {
    vk_create_instance();

    vk_create_surface();

    vk_pick_physical_device();
    vk_create_logical_device();

    vk_create_swapchain();
    vk_create_imageviews();
    vk_create_render_pass();

    vk_create_lbm_compute_descriptor_set_layout();
    vk_create_particle_compute_descriptor_set_layout();

    vk_create_particle_graphics_descriptor_set_layout();

    vk_create_obstacle_graphics_pipeline("shader/vert.spv", "shader/frag.spv");
    vk_create_particle_graphics_pipeline("shader/vert_particle.spv", "shader/frag_particle.spv");

    vk_create_lbm_compute_pipeline("shader/lbm.spv");
	vk_create_particle_compute_pipeline("shader/particles.spv");

    vk_create_framebuffers();
    vk_create_command_pool();
    
    vk_create_obstacle_vertex_buffer();

    vk_create_lbm_shader_storage_buffers();
    vk_create_particle_shader_storage_buffer();

    vk_create_lbm_uniform_buffers();
	vk_create_particle_uniform_buffers();

    vk_create_lbm_descriptor_pool();
	vk_create_particle_descriptor_pool();
    vk_create_particle_graphics_descriptor_pool();

    vk_create_lbm_compute_descriptor_sets();
	vk_create_particle_compute_descriptor_sets();
    vk_create_particle_graphics_descriptor_sets();

    vk_create_obstacle_graphics_command_buffers();
    vk_create_particle_graphics_command_buffers();

    vk_create_lbm_compute_command_buffers();
    vk_create_particle_compute_command_buffers();

    vk_create_sync_objects();
}

void VulkanParticleApp::vk_draw_frame() {
    if (mousedown) {
        // Get the current mouse cursor position delta
        double lastMouseX, lastMouseY;
        glfwGetCursorPos(gWindow, &lastMouseX, &lastMouseY);

        xMouse = 2.0 * ((float)lastMouseX / (float)gWindowWidth - 0.5);
        yMouse = 2.0 * ((float)lastMouseY / (float)gWindowHeight - 0.5);

        lbm_update_obstacle();
    }

    VkSubmitInfo submitInfo{};

    // LBM Compute submission   
    // TODO: Implement NUMR  
    //for (int i = 0; i < NUMR; i++)
    //{
        vkWaitForFences(vk_device, 1, &vk_lbm_compute_in_flight_fences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(vk_device, 1, &vk_lbm_compute_in_flight_fences[currentFrame]);

        vk_update_lbm_uniform_buffer(currentFrame);

        vkResetCommandBuffer(vk_lbm_compute_command_buffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        vk_record_lbm_compute_command_buffer(vk_lbm_compute_command_buffers[currentFrame]);

        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vk_lbm_compute_command_buffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &vk_lbm_compute_finished_semaphores[currentFrame];

        if (vkQueueSubmit(vk_compute_queue, 1, &submitInfo, vk_lbm_compute_in_flight_fences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit compute command buffer!");
        };
        vkQueueWaitIdle(vk_compute_queue);
    //}

	// Particle Compute submission
	vkWaitForFences(vk_device, 1, &vk_particle_compute_in_flight_fences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(vk_device, 1, &vk_particle_compute_in_flight_fences[currentFrame]);

    vk_update_particle_uniform_buffer(currentFrame);

	vkResetCommandBuffer(vk_particle_compute_command_buffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
	vk_record_particle_compute_command_buffer(vk_particle_compute_command_buffers[currentFrame]);

    VkSemaphore particleWaitSemaphores[] = { vk_lbm_compute_finished_semaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };

    submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = particleWaitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_particle_compute_command_buffers[currentFrame];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk_particle_compute_finished_semaphores[currentFrame];

	if (vkQueueSubmit(vk_compute_queue, 1, &submitInfo, vk_particle_compute_in_flight_fences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit compute command buffer!");
	};
	vkQueueWaitIdle(vk_compute_queue);


    // Obstacle Graphics submission
    vkWaitForFences(vk_device, 1, &vk_obstacle_in_flight_fences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, vk_image_available_semaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vk_recreate_swapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    vkResetFences(vk_device, 1, &vk_obstacle_in_flight_fences[currentFrame]);

    vkResetCommandBuffer(vk_obstacle_graphics_command_buffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    vk_record_obstacle_graphics_command_buffer(vk_obstacle_graphics_command_buffers[currentFrame], imageIndex);

    VkSemaphore waitSemaphores[] = { vk_particle_compute_finished_semaphores[currentFrame], vk_image_available_semaphores[currentFrame] };
    VkPipelineStageFlags particleWaitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = particleWaitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_obstacle_graphics_command_buffers[currentFrame];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk_obstacle_render_finished_semaphores[currentFrame];

    if (vkQueueSubmit(vk_graphics_queue, 1, &submitInfo, vk_obstacle_in_flight_fences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    vkQueueWaitIdle(vk_graphics_queue);

	// Particle Graphics submission
    vkWaitForFences(vk_device, 1, &vk_particle_in_flight_fences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(vk_device, 1, &vk_particle_in_flight_fences[currentFrame]);

    vkResetCommandBuffer(vk_particle_graphics_command_buffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    vk_record_particle_graphics_command_buffer(vk_particle_graphics_command_buffers[currentFrame], imageIndex);

    VkSemaphore particleGraphicsWaitSemaphores[] = { vk_obstacle_render_finished_semaphores[currentFrame]};
    VkPipelineStageFlags particleGraphicsWaitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = particleGraphicsWaitSemaphores;
    submitInfo.pWaitDstStageMask = particleGraphicsWaitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_particle_graphics_command_buffers[currentFrame];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk_particle_render_finished_semaphores[currentFrame];

    if (vkQueueSubmit(vk_graphics_queue, 1, &submitInfo, vk_particle_in_flight_fences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    vkQueueWaitIdle(vk_graphics_queue);

	// Present submission
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore presentWaitSemaphores[] = { vk_particle_render_finished_semaphores[currentFrame] };

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = presentWaitSemaphores;

    VkSwapchainKHR swapChains[] = { vk_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vk_present_queue, &presentInfo);

	// Recreate swapchain if necessary
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        vk_recreate_swapchain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanParticleApp::vk_main_loop() {
    while (!glfwWindowShouldClose(gWindow)) {
        glfwPollEvents();

        glfw_show_fps(gWindow);
        vk_draw_frame();

        // We want to animate the particle system using the last frames time to get smooth, frame-rate independent animation
        double currentTime = glfwGetTime();
        lastFrameTime = (currentTime - lastTime) * 1000.0;
        lastTime = currentTime;
    }

    vkDeviceWaitIdle(vk_device);
};

int main() {
    VulkanParticleApp app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
