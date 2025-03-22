#include "surface.h"
#include <stdexcept>

VkSurfaceKHR vk_create_surface(VkInstance vk_instance, GLFWwindow* window) {
	VkSurfaceKHR vk_surface;
    if (glfwCreateWindowSurface(vk_instance, window, nullptr, &vk_surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
	return vk_surface;
}
