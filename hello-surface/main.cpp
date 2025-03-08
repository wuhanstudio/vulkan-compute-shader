#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <fmt/core.h>
#include <vector>
#include <set>
#include <algorithm>
#include <optional>
#include <iostream>

#include "instance.h"
#include "validation.h"
#include "surface.h"
#include "swapchain.h"
#include "imageview.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* window;

// Print available extensions
void printExtensions() {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> vk_extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vk_extensions.data());

	fmt::println("Available extensions:");
	for (const auto& extension : vk_extensions) {
		fmt::println("{}", extension.extensionName);
	}
	fmt::print("\n");
}

// Press ESC to close the window
static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

int main() {

	// Create a window
	if (!glfwInit())
	{
		fmt::println("GLFW initialization failed");
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetKeyCallback(window, glfw_onKey);

	printExtensions();

	// Print the Vulkan version
	uint32_t apiVersion;
	auto FN_vkEnumerateInstanceVersion = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

	if (FN_vkEnumerateInstanceVersion == nullptr)
	{
		fmt::println("vkEnumerateInstanceVersion is not available. Using VK_API_VERSION_1_0\n");
	}
	else
	{
		vkEnumerateInstanceVersion(&apiVersion);
		fmt::println("Vulkan API version: {}.{}.{}\n",
			VK_VERSION_MAJOR(apiVersion),
			VK_VERSION_MINOR(apiVersion),
			VK_VERSION_PATCH(apiVersion));
	}

	// Initialize Vulkan
	VkInstance instance = createInstance();

	// Create a surface
	VkSurfaceKHR surface = createSurface(window, instance);

	printDeviceInfo(instance, surface);

	// Set the physical device
	VkPhysicalDevice physicalDevice = choosePhysicalDevice(instance);

	// Create a logical device
	VkDevice device = createLogicalDevice(physicalDevice, surface);

	// Create a swapchain
	VkSwapchainKHR swapChain = createSwapChain(window, physicalDevice, device, surface);
	fmt::println("Minimum Extent: {}x{}", swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.minImageExtent.height);
	fmt::println("Maximum Extent: {}x{}", swapChainSupport.capabilities.maxImageExtent.width, swapChainSupport.capabilities.maxImageExtent.height);
	fmt::println("Chosen Extent: {}x{}", swapChainExtent.width, swapChainExtent.height);

	// Create image views
	createImageViews(device);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	// Clean up
	cleanupSwapChain(device, swapChain);

	vkDestroyDevice(device, nullptr);

	bool enableValidationLayers = checkValidationLayerSupport();
#ifdef NDEBUG
	enableValidationLayers = false;
	fmt::print("Running in release mode, validation layers disabled\n");
#endif

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	// Destroy the Vulkan instance
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
