#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

extern VkPipeline Pipeline;
extern VkPipelineLayout PipelineLayout;
extern VkDescriptorSetLayout DescriptorSetLayout;

void CreatePipeline(void);
void DestroyPipeline(void);

#ifdef __cplusplus
}
#endif
