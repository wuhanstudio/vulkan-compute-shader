#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <fmt/core.h>
#include <iostream>
#include <vector>

#include "instance.h"
#include "validation.h"
#include "surface.h"
#include "swapchain.h"
#include "imageview.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* gWindow;

// Print available extensions
void vk_print_extensions() {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> vk_extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vk_extensions.data());

	fmt::println("Available extensions:\n");
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

	gWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetKeyCallback(gWindow, glfw_onKey);

	vk_print_extensions();

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
	VkDebugUtilsMessengerEXT debugMessenger;
	VkInstance instance = vk_create_instance(&debugMessenger);

	// Create a surface
	VkSurfaceKHR surface = vk_create_surface(gWindow, instance);

	vk_print_device_info(instance, surface);

	// Set the physical device
	VkPhysicalDevice physicalDevice = vk_pick_physical_device(instance);

	// Create a logical device
	VkDevice device = vk_create_logical_device(physicalDevice, surface);
	QueueFamilyIndices indices = vk_find_queue_families(physicalDevice, surface);

	VkQueue graphicsQueue, presentQueue;
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

	// Create a swapchain
	SwapChainSupportDetails swapChainSupport = vk_query_swapchain_support(physicalDevice, surface);
	fmt::println("Minimum Extent: {}x{}", swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.minImageExtent.height);
	fmt::println("Maximum Extent: {}x{}", swapChainSupport.capabilities.maxImageExtent.width, swapChainSupport.capabilities.maxImageExtent.height);

	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkSwapchainKHR swapChain = vk_create_swapchain(gWindow, physicalDevice, device, surface, swapChainImages, &swapChainImageFormat, &swapChainExtent);
	fmt::println("Chosen Extent: {}x{}", swapChainExtent.width, swapChainExtent.height);
	fmt::print("\n");

	// Create image views
	std::vector<VkImageView> swapChainImageViews;
	vk_create_image_views(device, swapChainImageFormat, swapChainImageViews, swapChainImages);
	while (!glfwWindowShouldClose(gWindow)) {
		glfwPollEvents();
	}

	// Clean up
	vk_cleanup_swapchain(device, swapChain, swapChainImageViews);

	vkDestroyDevice(device, nullptr);

	bool enableValidationLayers = vk_check_validation_layer();
#ifdef NDEBUG
	enableValidationLayers = false;
	fmt::print("Running in release mode, validation layers disabled\n");
#endif

	if (enableValidationLayers) {
		vk_destroy_debug_utils_messenger_ext(instance, debugMessenger, nullptr);
	}


	// Destroy the Vulkan instance
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(gWindow);
	glfwTerminate();

	return 0;
}
