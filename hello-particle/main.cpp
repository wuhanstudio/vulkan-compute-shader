#include "app.h"

// Press ESC to close the window
static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
};

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

    lastTime = glfwGetTime();
}

void VulkanParticleApp::vk_init() {
    vk_create_instance();
    vk_setup_debug_messenger();
    vk_create_surface();
    vk_pick_physical_device();
    vk_create_logical_device();
    vk_create_swapchain();
    vk_create_imageviews();
    vk_create_render_pass();
    vk_create_compute_descriptor_set_layout();
    vk_create_graphics_pipeline();
    vk_create_compute_pipeline();
    vk_create_framebuffers();
    vk_create_command_pool();
    vk_create_shader_storage_buffers();
    vk_create_uniform_buffers();
    vk_create_descriptor_pool();
    vk_create_compute_descriptor_sets();
    vk_create_command_buffers();
    vk_create_compute_command_buffers();
    vk_create_sync_objects();
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