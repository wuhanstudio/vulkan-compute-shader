#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <fmt/core.h>
#include <vector>

#include "validation.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* window;
VkSurfaceKHR surface;

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

const char* vkPhysicalDeviceType_as_string(VkPhysicalDeviceType type) {
	switch (type)
	{
	case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		return "Other";

	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return "Integrated GPU";

	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return "Discrete GPU";

	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return "Virtualized GPU";

	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		return "CPU";

	default:
		return "Unknown";
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

	printExtensions();

	bool enableValidationLayers = checkValidationLayerSupport();
	if (not enableValidationLayers) {
		fmt::println("Validation layers not available");
	}
	else {
		fmt::print("Validation layers available\n");
	}

#ifdef NDEBUG
	enableValidationLayers = false;
	fmt::print("Running in release mode, validation layers disabled\n");
#endif

	// Initialize Vulkan
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		// Enable debug extension
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Create a Vulkan instance
	VkInstance instance;
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		fmt::print("Failed to create Vulkan instance\n");
		return -1;
	}

	// Set up debug messenger
	if (enableValidationLayers) {
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		populateDebugMessengerCreateInfo(debugCreateInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("Failed to set up debug messenger!");
		}
	}

	fmt::println("");

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


	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}

	// Query the number of Vulkan physical devices
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	fmt::println("Number of physical devices: {}\n", physicalDeviceCount);

	std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());

	fmt::println("Available devices:");
	for (const auto& device : devices) {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		fmt::println("Device name: {}", deviceProperties.deviceName);
		fmt::println("Device type: {}", vkPhysicalDeviceType_as_string(deviceProperties.deviceType));
		fmt::println("API version: {}.{}.{}\n",
			VK_VERSION_MAJOR(deviceProperties.apiVersion),
			VK_VERSION_MINOR(deviceProperties.apiVersion),
			VK_VERSION_PATCH(deviceProperties.apiVersion));

		// Print queue families
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		fmt::println("Queue families: {}", queueFamilyCount);
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			fmt::print("  Queue count: {}", queueFamily.queueCount);
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				fmt::print("| Graphics ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
				fmt::print("| Compute ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
				fmt::print("| Transfer ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
				fmt::print("| Sparse binding ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) {
				fmt::print("| Protected ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
				fmt::print("| Video Decode ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
				fmt::print("| Video Encode ");
			}
			if (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
				fmt::print("| Optical Flow ");
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				fmt::print(" [ Present Support ]");
			}
			fmt::print("\n");
			i++;
		}
		fmt::print("\n");
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

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
