#ifdef __cplusplus
extern "C" {
#endif

#ifndef VULKAN_COMPTE_H
#define VULKAN_COMPTE_H

#include <vulkan/vulkan.h>

extern VkDescriptorSet DescriptorSet;

void PrepareCommandBuffer(void);
int Compute(void);
void CreateDescriptorSet(void);

#endif // VULKAN_COMPTE_H

#ifdef __cplusplus
}
#endif