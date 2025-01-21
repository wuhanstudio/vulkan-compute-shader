#include <vulkan/vulkan.h>

void CreateDeviceAndComputeQueue(void);
void CreateCommandPool(void);
void DestroyCommandPoolAndLogicalDevice(void);
void CreateDescriptorPool(void);
 
extern VkDevice LogicalDevice;
extern VkQueue ComputingQueue;
extern VkCommandPool ComputeCmdPool;
extern VkDescriptorPool DescriptorPool;
