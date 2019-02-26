
#pragma once

#include "Win32PlatformLayer.h"

#define VK_EXPORTED_FUNCTION(name) extern PFN_##name name;
#define VK_GLOBAL_FUNCTION(name) extern PFN_##name name;
#define VK_INSTANCE_FUNCTION(name) extern PFN_##name name;
#define VK_INSTANCE_FUNCTION_EXT(name) extern PFN_##name name;
#define VK_DEVICE_FUNCTION(name) extern PFN_##name name;
#define VK_DEVICE_FUNCTION_EXT(name) extern PFN_##name name;
#include "VulkanFunctions.inl"