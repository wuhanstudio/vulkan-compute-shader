#include "surface.h"
#include <stdexcept>

VkSurfaceKHR vk_create_surface(GLFWwindow* window, VkInstance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    return surface;
}
