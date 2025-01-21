#include "pipeline.h"
#include <stdio.h>
#include <string.h>
#include "device.h"

VkPipeline Pipeline = VK_NULL_HANDLE;
VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;

VkShaderModule CreateComputeShader(void)
{
    uint8_t shaderData[20000];

    FILE *f = fopen("shader/fractal.spv", "rb");
    if (f == NULL)
    {
        printf("Failed to open the shader file.\n");
        return VK_NULL_HANDLE;
    }

    size_t size = fread(shaderData, 1, sizeof(shaderData), f);
    fclose(f);

    VkShaderModuleCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = (uint32_t*)shaderData;

    VkShaderModule handle;

    if (vkCreateShaderModule(LogicalDevice, &createInfo, NULL, &handle) != VK_SUCCESS)
    {
        printf("Failed to create the shader module.\n");
        return VK_NULL_HANDLE;
    }
    return handle;
}

void CreateDescriptorSetLayout(void)
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

    if (vkCreateDescriptorSetLayout(LogicalDevice, &createInfo, NULL, &DescriptorSetLayout) != VK_SUCCESS)
    {
        printf("Failed to create a descriptor set layout handle.'n");
    }
}

void CreatePipelineLayout(void)
{
    CreateDescriptorSetLayout();

    VkPipelineLayoutCreateInfo createLayout;
    memset(&createLayout, 0, sizeof(createLayout));
    createLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createLayout.pSetLayouts = &DescriptorSetLayout;
    createLayout.setLayoutCount = 1;

    if (vkCreatePipelineLayout(LogicalDevice, &createLayout, NULL, &PipelineLayout) != VK_SUCCESS)
    {
        printf("Failed to create the pipeline layout.\n");
        return;
    }
}
 
void CreatePipeline(void)
{
    CreatePipelineLayout();

    VkComputePipelineCreateInfo createPipeline;
    memset(&createPipeline, 0, sizeof(createPipeline));
    createPipeline.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createPipeline.layout = PipelineLayout;
    createPipeline.basePipelineIndex = -1;
    createPipeline.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createPipeline.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createPipeline.stage.pName = "main";
    createPipeline.stage.module = CreateComputeShader();

    if (vkCreateComputePipelines(LogicalDevice, VK_NULL_HANDLE, 1, &createPipeline, NULL, &Pipeline) != VK_SUCCESS)
    {
        printf("Failed to create a pipeline.\n");
        return;
    }

    vkDestroyShaderModule(LogicalDevice, createPipeline.stage.module, NULL);
}

void DestroyPipeline(void)
{
    if (PipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, NULL);
    }
    if (DescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(LogicalDevice, DescriptorSetLayout, NULL);
    }
    if (Pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(LogicalDevice, Pipeline, NULL);
    }
}