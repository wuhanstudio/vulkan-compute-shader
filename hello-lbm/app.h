#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <random>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int NX = 480;		// solver grid resolution
const int NY = 360;

const uint32_t PARTICLE_COUNT = 8192;

const int MAX_FRAMES_IN_FLIGHT = 2;

/*--------------------- LBM -----------------------------------------------------------------------------*/
#define NUMR 20
#define NUM_VECTORS 9	// lbm basis vectors (d2q9 model)

/*--------------------- Particles -----------------------------------------------------------------------*/
const float dt = 0.1;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsAndComputeFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsAndComputeFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct LBMUniformBufferObject {
    float deltaTime = 1.0f;
};

struct Particle {
    glm::vec2 position;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Particle);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Particle, position);

        return attributeDescriptions;
    }
};

class VulkanParticleApp {
public:
    void run() {
        vk_init_window();

        vk_init();

        lastTime = glfwGetTime();

        vk_main_loop();

        vk_cleanup();
    }

private:
    GLFWwindow* gWindow;

    int F_cpu[NX * NY];
    float xMouse, yMouse;

    std::vector<Particle> vertices;

    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT vk_debug_messenger;

    VkSurfaceKHR vk_surface;

    VkPhysicalDevice vk_physical_device = VK_NULL_HANDLE;
    VkDevice vk_device;

    VkQueue vk_graphics_queue;
    VkQueue vk_compute_queue;
    VkQueue vk_present_queue;

    VkSwapchainKHR vk_swapchain;

    VkExtent2D vk_swapchain_extent;
    VkFormat vk_swapchain_image_format;

    std::vector<VkImage> vk_swapchain_images;
    std::vector<VkImageView> vk_swapchain_imageviews;
    std::vector<VkFramebuffer> vk_swapchain_framebuffers;

    VkRenderPass vk_render_pass;

    VkPipeline vk_graphics_pipeline;
    VkPipelineLayout vk_graphics_pipeline_layout;

    VkDescriptorPool vk_lbm_descriptor_pool;
    VkDescriptorSetLayout vk_lbm_compute_descriptor_set_layout;
    std::vector<VkDescriptorSet> vk_lbm_compute_descriptor_sets;

    VkPipeline vk_lbm_compute_pipeline;
    VkPipelineLayout vk_lbm_compute_pipeline_layout;

    VkCommandPool vk_command_pool;

    std::vector<VkBuffer> vk_df0_storage_buffers;
    std::vector<VkDeviceMemory> vk_df0_storage_buffers_memory;

    std::vector<VkBuffer> vk_df1_storage_buffers;
    std::vector<VkDeviceMemory> vk_df1_storage_buffers_memory;

    std::vector<VkBuffer> vk_dcf_storage_buffers;
    std::vector<VkDeviceMemory> vk_dcf_storage_buffers_memory;

    std::vector<VkBuffer> vk_dcu_storage_buffers;
    std::vector<VkDeviceMemory> vk_dcu_storage_buffers_memory;

    std::vector<VkBuffer> vk_dcv_storage_buffers;
    std::vector<VkDeviceMemory> vk_dcv_storage_buffers_memory;

    std::vector<VkBuffer> vk_uniform_buffers;
    std::vector<VkDeviceMemory> vk_uniform_buffers_memory;
    std::vector<void*> vk_uniform_buffers_mapped;

    VkBuffer vk_obstacle_vertex_buffer;
    VkDeviceMemory vk_obstacle_vertex_buffer_memory;

    std::vector<VkCommandBuffer> vk_graphics_command_buffers;
    std::vector<VkCommandBuffer> vk_compute_command_buffers;

    std::vector<VkSemaphore> vk_image_available_semaphores;
    std::vector<VkSemaphore> vk_render_finished_semaphores;
    std::vector<VkSemaphore> vk_compute_finished_semaphores;

    std::vector<VkFence> vk_in_flight_fences;
    std::vector<VkFence> vk_compute_in_flight_fences;

    uint32_t currentFrame = 0;

    double lastTime = 0.0f;
    float lastFrameTime = 0.0f;
    

    void vk_init();
    void vk_main_loop();

    void vk_init_window();

    void vk_cleanup_swapchain();

    void vk_cleanup();

    void vk_recreate_swapchain();

    void vk_create_instance();

    void vk_pick_physical_device();

    void vk_populate_debug_messenger_createInfo(VkDebugUtilsMessengerCreateInfoEXT& vk_createInfo);

    VkResult vk_create_debug_utils_messenger_ext(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    void vk_destroy_debug_utils_messenger_ext(const VkAllocationCallbacks * pAllocator);

    void vk_setup_debug_messenger();

    void vk_create_surface();

    void vk_create_logical_device();

    void vk_create_swapchain();

    void vk_create_imageviews();

    void vk_create_render_pass();

    void vk_create_lbm_compute_descriptor_set_layout();

    void vk_create_graphics_pipeline(const char* f_vert, const char* f_frag);

    void vk_create_compute_pipeline(const char* f_compute);

    void vk_create_framebuffers();

    void vk_create_command_pool();

    void vk_create_obstacle_vertex_buffer();

    void vk_create_lbm_shader_storage_buffers();

    void vk_create_uniform_buffers();

    void vk_create_descriptor_pool();

    void vk_create_compute_descriptor_sets();

    void vk_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void vk_copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t vk_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void vk_create_command_buffers();

    void vk_create_compute_command_buffers();

    void vk_record_graphics_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void vk_record_compute_command_buffer(VkCommandBuffer commandBuffer);

    void vk_create_sync_objects();

    void vk_update_uniform_buffer(uint32_t currentImage);

    void vk_draw_frame();

    VkShaderModule vk_create_shader_module(const std::vector<char>& code);

    VkSurfaceFormatKHR vk_choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR vk_choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D vk_choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

    SwapChainSupportDetails vk_query_swapchain_support(VkPhysicalDevice device);

    bool vk_is_device_suitable(VkPhysicalDevice device);

    bool vk_check_device_extension_support(VkPhysicalDevice device);

    QueueFamilyIndices vk_find_queue_families(VkPhysicalDevice device);

    std::vector<const char*> vk_get_required_extensions();

    bool vk_check_validation_layer_support();

    std::vector<char> read_file(const std::string& filename);
    void lbm_update_obstacle(void);
    void lbm_init_ssb(void);
};
