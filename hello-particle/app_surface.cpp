#include "app.h"

void VulkanParticleApp::vk_create_surface() {
    if (glfwCreateWindowSurface(vk_instance, gWindow, nullptr, &vk_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
