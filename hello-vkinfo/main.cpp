#include <vulkan/vulkan.h>

#include <fmt/core.h>
#include <vector>

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

	// Create a Vulkan instance
	VkInstance instance;
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		fmt::print("Failed to create Vulkan instance\n");
		return -1;
	}

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

	fmt::println("Available devices:\n");
	for (const auto& device : devices) {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Print device information
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
				fmt::print("| Protected");
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
			fmt::print("\n");
		}
		fmt::print("\n");

		// Print compute capabilities
		fmt::println("Max Dimension 1D: {}", deviceProperties.limits.maxImageDimension1D);
		fmt::println("Max Dimension 2D: {}", deviceProperties.limits.maxImageDimension2D);
		fmt::println("Max Dimension 3D: {}", deviceProperties.limits.maxImageDimension3D);

		fmt::println("Max Compute Work Group Count: [{}, {}, {}]", deviceProperties.limits.maxComputeWorkGroupCount[0], deviceProperties.limits.maxComputeWorkGroupCount[1], deviceProperties.limits.maxComputeWorkGroupCount[2]);
		fmt::println("Max Compute Work Group Size: [{}, {}, {}]", deviceProperties.limits.maxComputeWorkGroupSize[0], deviceProperties.limits.maxComputeWorkGroupSize[1], deviceProperties.limits.maxComputeWorkGroupSize[2]);
		fmt::println("Max Compute Work Group Invocations: {}", deviceProperties.limits.maxComputeWorkGroupInvocations);
		fmt::println("Max Compute Shared Memory Size: {}", deviceProperties.limits.maxComputeSharedMemorySize);
		fmt::print("\n");

		// Print available extensions
		//uint32_t extensionCount = 0;
		//vkEnumerateDeviceExtensionProperties(device, nullptr , &extensionCount, nullptr);

		//std::vector<VkExtensionProperties> vk_extensions(extensionCount);
		//vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, vk_extensions.data());

		//fmt::println("Available extensions:");
		//for (const auto& extension : vk_extensions) {
			//fmt::println("{}", extension.extensionName);
		//}
		//fmt::print("\n");
	}

	// Destroy the Vulkan instance
	vkDestroyInstance(instance, nullptr);

	return 0;
}
