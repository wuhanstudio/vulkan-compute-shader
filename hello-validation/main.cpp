#include <vulkan/vulkan.h>

#include <fmt/core.h>
#include <vector>

#include "validation.h"

// Need to be global
std::vector<const char*> extensions;

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

	printExtensions();

	bool enableValidationLayers = checkValidationLayerSupport();
	if (not enableValidationLayers) {
		fmt::println("Validation layers not available");
	}
	else {
		fmt::print("Validation layers available\n");
	}

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

	if (enableValidationLayers) {
		// Enable debug extension
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		createInfo.enabledExtensionCount = 1;
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
	}

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	// Destroy the Vulkan instance
	vkDestroyInstance(instance, nullptr);

	return 0;
}
