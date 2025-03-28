#include <fmt/core.h>
#include "app.h"

int mousedown = 0;

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

    gWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(gWindow, this);

    glfwSetKeyCallback(gWindow, glfw_onKey);
    glfwSetMouseButtonCallback(gWindow, glfw_onMouse);
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

    vk_create_graphics_pipeline("shader/vert.spv", "shader/frag.spv");
    vk_create_compute_pipeline("shader/lbm.spv");

    vk_create_framebuffers();
    vk_create_command_pool();
    
    vk_create_obstacle_vertex_buffer();
    vk_create_lbm_shader_storage_buffers();
    vk_create_uniform_buffers();

    vk_create_descriptor_pool();
    vk_create_compute_descriptor_sets();

    vk_create_command_buffers();
    vk_create_compute_command_buffers();

    vk_create_sync_objects();
}

void VulkanParticleApp::vk_draw_frame() {
    if (mousedown) {
        // glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        double lastMouseX, lastMouseY;
        // Get the current mouse cursor position delta
        glfwGetCursorPos(gWindow, &lastMouseX, &lastMouseY);

        // if (FULLSCREEN) {
            // xMouse = 2.0 * ((float)lastMouseX / (float)gWindowWidthFull - 0.5);
            // yMouse = -2.0 * ((float)lastMouseY / (float)gWindowHeightFull - 0.5);
        // }
        // else
        // {
            xMouse = 2.0 * ((float)lastMouseX / (float)WIDTH - 0.5);
            yMouse = 2.0 * ((float)lastMouseY / (float)HEIGHT - 0.5);
        // }
        // fmt::println("Mouse: {} {}", xMouse, yMouse);
        lbm_update_obstacle();
    }

    // if (glfwGetTime() - lastTime > abs(dt) / 10)
    // {
    //     lastTime = glfwGetTime();
    // }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Compute submission        
    vkWaitForFences(vk_device, 1, &vk_compute_in_flight_fences[currentFrame], VK_TRUE, UINT64_MAX);

    vk_update_uniform_buffer(currentFrame);

    vkResetFences(vk_device, 1, &vk_compute_in_flight_fences[currentFrame]);

    vkResetCommandBuffer(vk_compute_command_buffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    vk_record_compute_command_buffer(vk_compute_command_buffers[currentFrame]);

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_compute_command_buffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk_compute_finished_semaphores[currentFrame];

    if (vkQueueSubmit(vk_compute_queue, 1, &submitInfo, vk_compute_in_flight_fences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    };
    vkQueueWaitIdle(vk_compute_queue);

    // Graphics submission
    vkWaitForFences(vk_device, 1, &vk_in_flight_fences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, vk_image_available_semaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vk_recreate_swapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(vk_device, 1, &vk_in_flight_fences[currentFrame]);

    vkResetCommandBuffer(vk_graphics_command_buffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    vk_record_graphics_command_buffer(vk_graphics_command_buffers[currentFrame], imageIndex);

    VkSemaphore waitSemaphores[] = { vk_compute_finished_semaphores[currentFrame], vk_image_available_semaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_graphics_command_buffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk_render_finished_semaphores[currentFrame];

    if (vkQueueSubmit(vk_graphics_queue, 1, &submitInfo, vk_in_flight_fences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    vkQueueWaitIdle(vk_graphics_queue);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vk_render_finished_semaphores[currentFrame];

    VkSwapchainKHR swapChains[] = { vk_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vk_present_queue, &presentInfo);

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
