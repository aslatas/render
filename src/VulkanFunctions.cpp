
#include "VulkanFunctions.h"

#define VK_EXPORTED_FUNCTION(name) PFN_##name name;
#define VK_GLOBAL_FUNCTION(name) PFN_##name name;
#define VK_INSTANCE_FUNCTION(name) PFN_##name name;
#define VK_INSTANCE_FUNCTION_EXT(name) PFN_##name name;
#define VK_DEVICE_FUNCTION(name) PFN_##name name;
#define VK_DEVICE_FUNCTION_EXT(name) PFN_##name name;
#include "VulkanFunctions.inl"
