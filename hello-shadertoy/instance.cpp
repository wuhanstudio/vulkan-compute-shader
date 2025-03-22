#include "instance.h"
#include "validation.h"

#include <stdexcept>

#include <GLFW/glfw3.h>
#include <fmt/core.h>

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

std::vector<const char*> vk_get_required_extensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (vk_check_validation_layer()) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VkInstance vk_create_instance(VkDebugUtilsMessengerEXT& vk_debug_messenger)
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Shader";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = vk_get_required_extensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vk_check_validation_layer()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        vk_populate_debug_messenger_createInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VkInstance instance = VK_NULL_HANDLE;
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance!");
    }

	// Set up debug messenger
    if (vk_check_validation_layer()) {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        vk_populate_debug_messenger_createInfo(debugCreateInfo);

        if (vk_create_debug_utils_messenger_ext(instance, &debugCreateInfo, nullptr, &vk_debug_messenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    return instance;

}
