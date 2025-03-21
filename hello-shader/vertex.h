#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <stdexcept>

#include "device.h"
#include "command.h"
#include "memory.h"

extern VkBuffer vertexBuffer;
extern VkDeviceMemory vertexBufferMemory;

extern VkBuffer indexBuffer;
extern VkDeviceMemory indexBufferMemory;

struct Vertex {
    glm::vec2 pos;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() 
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};


const std::vector<Vertex> vertices = {
    {{-1.0f, -1.0f},  {1.0f, 0.0f}},
    {{ 1.0f, -1.0f},  {0.0f, 0.0f}},
    {{ 1.0f,  1.0f} , {0.0f, 1.0f}},
    {{-1.0f,  1.0f},  {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

void vk_create_vertex_buffer(VkPhysicalDevice vk_physical_device, VkDevice vk_device);
void vk_create_index_buffer(VkPhysicalDevice vk_physical_device, VkDevice vk_device, VkQueue vk_graphics_queue, VkCommandPool vk_command_pool);
void vk_create_buffer(VkPhysicalDevice vk_physical_device, VkDevice vk_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
