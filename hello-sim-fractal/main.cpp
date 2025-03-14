#include <chrono>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "instance.h"
#include "device.h"
#include "compute.h"
#include "memory.h"
#include "pipeline.h"

#define width 1000
#define height 1000

uint32_t vk_input_data[width];
uint32_t vk_output_data[height][width];

double getTime()
{
     // Get the current time from the system clock
     auto now = std::chrono::high_resolution_clock::now();

     // Convert the current time to time since epoch
     auto duration = now.time_since_epoch();

     // Convert duration to microseconds
     auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

	 return microseconds;
}

void generate_fractal_cpu()
{
    for (uint32_t row = 0; row < height; row++)
    {
        for (uint32_t col = 0; col < width; col++)
        {
            float r = (float)row / 500.0 - 1.0;
            float i = (float)col / 500.0 - 1.0;

            uint32_t cnt = 0;
            while (((r * r + i * i) < 4.0) && (cnt < 63))
            {
                float temp = r * r - i * i + 0.17;
                i = 2 * r * i + 0.57;
                r = temp;
                cnt++;
            }
            vk_output_data[row][col] = (cnt << 10) | 0xff000000;
        }
    }
}


VkDescriptorSetLayout vk_create_descriptor_set_layout(VkDevice vk_device)
{
    VkDescriptorSetLayoutBinding bindings[2];
    memset(&bindings, 0, sizeof(bindings));

    bindings[0].binding = 0;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;

    bindings[1].binding = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 2;
    createInfo.pBindings = bindings;

    VkDescriptorSetLayout vk_descriptor_set_layout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(vk_device, &createInfo, NULL, &vk_descriptor_set_layout) != VK_SUCCESS)
    {
        printf("Failed to create a descriptor set layout handle.'n");
    }
    return vk_descriptor_set_layout;
}

int main(int argc, char* argv[])
{
	// Create a Vulkan instance and select a physical device
    VkInstance vk_instance = vk_create_instance();
    VkPhysicalDevice vk_phy_device = vk_create_physical_device(vk_instance);

	// Create a logical device and a compute queue
    VkQueue  vk_queue_compute = VK_NULL_HANDLE;
	uint32_t vk_queue_family_index = 0;
    VkDevice vk_device = vk_create_device_and_compute_queue(vk_phy_device, &vk_queue_family_index, &vk_queue_compute);

	// Define bindings for the descriptor set layout
	VkDescriptorPool vk_descriptor_pool = vk_create_descriptor_pool(vk_device);
    VkDescriptorSetLayout vk_descriptor_set_layout = vk_create_descriptor_set_layout(vk_device);

    VkDescriptorSet vk_descriptor_set = vk_create_descriptor_set(vk_device, vk_descriptor_set_layout, vk_descriptor_pool);

	// Create buffers for the input and output data
    VkDeviceMemory vk_input_buffer_memory = VK_NULL_HANDLE;
    VkDeviceMemory vk_output_buffer_emory = VK_NULL_HANDLE;

    VkBuffer vk_input_buffer = vk_create_buffer_and_memory(vk_phy_device, vk_device, sizeof(vk_input_data), &vk_input_buffer_memory);
    VkBuffer vk_output_buffer = vk_create_buffer_and_memory(vk_phy_device, vk_device, sizeof(vk_output_data), &vk_output_buffer_emory);

    vk_update_descriptor_set(vk_device, vk_descriptor_set, sizeof(vk_input_data), sizeof(vk_output_data), vk_input_buffer, vk_output_buffer);

	// Create a pipeline and a command pool
    VkPipelineLayout vk_pipeline_layout = vk_create_pipeline_layout(vk_device, vk_descriptor_set_layout);
    VkPipeline vk_pipeline = vk_create_pipline(vk_device, vk_pipeline_layout, vk_descriptor_set_layout);

    VkCommandPool vk_compute_cmd_pool = vk_create_command_pool(vk_device, vk_queue_family_index);
    VkCommandBuffer vk_command_buffer = vk_prepare_command_buffer(vk_device, vk_pipeline, vk_pipeline_layout, vk_descriptor_set, vk_compute_cmd_pool);
 
	// Copy the input data to the GPU
    vk_copy_to_input_buffer(vk_device, vk_input_data, sizeof(vk_input_data), vk_input_buffer_memory);

    double time = getTime();
    generate_fractal_cpu();
    time = getTime() - time;
    printf("CPU fractal: %f ms.\n", time / 1000.0f);

    // Allocate memory for the image data in RGBA format (4 channels per pixel)
    unsigned char* image_data = (unsigned char*)malloc(width * height * 4);

    // Convert the uint32_t color data into RGBA format
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            uint32_t color = vk_output_data[row][col];

            // Extract each channel from the 32-bit packed color
            image_data[(row * width + col) * 4 + 0] = (color >> 16) & 0xFF;  // Red
            image_data[(row * width + col) * 4 + 1] = (color >> 8) & 0xFF;   // Green
            image_data[(row * width + col) * 4 + 2] = color & 0xFF;          // Blue
            image_data[(row * width + col) * 4 + 3] = (color >> 24) & 0xFF;  // Alpha
        }
    }
    stbi_write_png("fractal_cpu.png", width, height, 4, image_data, width * 4);

	// Clear the output data
    memset(vk_output_data, 0, sizeof(vk_output_data));

    time = getTime();
    vk_compute(vk_device, vk_queue_compute, vk_command_buffer);
    time = getTime() - time;

    printf("GPU fractal: %f ms.\n", time / 1000.0f);

	// Copy the output data from the GPU to the CPU
    vk_copy_from_output_buffer(vk_device, vk_output_data, sizeof(vk_output_data), vk_output_buffer_emory);

    memset(image_data, 0, sizeof(image_data));

    // Convert the uint32_t color data into RGBA format
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            uint32_t color = vk_output_data[row][col];

            // Extract each channel from the 32-bit packed color
            image_data[(row * width + col) * 4 + 0] = (color >> 16) & 0xFF;  // Red
            image_data[(row * width + col) * 4 + 1] = (color >> 8) & 0xFF;   // Green
            image_data[(row * width + col) * 4 + 2] = color & 0xFF;          // Blue
            image_data[(row * width + col) * 4 + 3] = (color >> 24) & 0xFF;  // Alpha
        }
    }
    stbi_write_png("fractal_gpu.png", width, height, 4, image_data, width * 4);

    free(image_data);

    vk_destroy_pipeline(vk_device, vk_pipeline, vk_pipeline_layout, vk_descriptor_set_layout);

    vkDestroyCommandPool(vk_device, vk_compute_cmd_pool, NULL);
    vkDestroyDescriptorPool(vk_device, vk_descriptor_pool, NULL);

    vk_destroy_buffers(vk_device, vk_input_buffer, vk_output_buffer, vk_input_buffer_memory, vk_output_buffer_emory);

    vkDestroyDevice(vk_device, NULL);

    return 0;
}
