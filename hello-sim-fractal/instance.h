#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

void CreateInstance(void);
void GetPhysicalDevice(void);

extern VkInstance Instance;
extern VkPhysicalDevice PhysicalDevice;

#ifdef __cplusplus
}
#endif
