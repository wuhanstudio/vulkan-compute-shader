#include "surface.h"
#include <stdexcept>

VkSurfaceKHR createSurface(GLFWwindow* window, VkInstance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    return surface;
}
