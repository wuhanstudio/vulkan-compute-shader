#ifdef __cplusplus
extern "C" {
#endif

#include "pipeline.h"
#include <stdio.h>
#include <string.h>
#include "device.h"

VkShaderModule CreateComputeShader(VkDevice vk_device)
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

    if (vkCreateShaderModule(vk_device, &createInfo, NULL, &handle) != VK_SUCCESS)
    {
        printf("Failed to create the shader module.\n");
        return VK_NULL_HANDLE;
    }
    return handle;
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

VkPipelineLayout vk_create_pipeline_layout(VkDevice vk_device, VkDescriptorSetLayout vk_descriptor_set_layout)
{
	VkPipelineLayout vk_pipeline_layout;

    VkPipelineLayoutCreateInfo createLayout;
    memset(&createLayout, 0, sizeof(createLayout));

    createLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createLayout.pSetLayouts = &vk_descriptor_set_layout;
    createLayout.setLayoutCount = 1;

    if (vkCreatePipelineLayout(vk_device, &createLayout, NULL, &vk_pipeline_layout) != VK_SUCCESS)
    {
        printf("Failed to create the pipeline layout.\n");
        return VK_NULL_HANDLE;
    }
	return vk_pipeline_layout;
}
 
VkPipeline vk_create_pipline(VkDevice vk_device, VkPipelineLayout* vk_pipeline_layout, VkDescriptorSetLayout vk_descriptor_set_layout)
{
    VkPipeline vk_pipeline;
    *vk_pipeline_layout = vk_create_pipeline_layout(vk_device, vk_descriptor_set_layout);

    VkComputePipelineCreateInfo createPipeline;
    memset(&createPipeline, 0, sizeof(createPipeline));
    createPipeline.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createPipeline.layout = *vk_pipeline_layout;
    createPipeline.basePipelineIndex = -1;
    createPipeline.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createPipeline.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createPipeline.stage.pName = "main";
    createPipeline.stage.module = CreateComputeShader(vk_device);

    if (vkCreateComputePipelines(vk_device, VK_NULL_HANDLE, 1, &createPipeline, NULL, &vk_pipeline) != VK_SUCCESS)
    {
        printf("Failed to create a pipeline.\n");
        return VK_NULL_HANDLE;
    }

    vkDestroyShaderModule(vk_device, createPipeline.stage.module, NULL);

	return vk_pipeline;
}

void vk_destroy_pipeline(VkDevice vk_device, VkPipeline vk_pipeline, VkPipelineLayout vk_pipeline_layout, VkDescriptorSetLayout vk_descriptor_set_layout)
{
    if (vk_pipeline_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, NULL);
    }
    if (vk_descriptor_set_layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(vk_device, vk_descriptor_set_layout, NULL);
    }
    if (vk_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(vk_device, vk_pipeline, NULL);
    }
}

#ifdef __cplusplus
}
#endif
