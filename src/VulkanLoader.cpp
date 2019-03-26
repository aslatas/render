
#include "VulkanLoader.h"

bool CheckValidationLayerSupport(VkLayerProperties available[], u32 available_count)
{
    u32 validation_layer_count = sizeof(validation_layers) / sizeof(validation_layers[0]);
    for (u32 i = 0; i < validation_layer_count; ++i) {
        bool found = false;
        for (u32 j = 0; j < available_count; ++j) {
            if (strcmp(validation_layers[i], available[j].layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

bool CheckInstanceExtensionSupport(VkExtensionProperties available[], u32 available_count)
{
    u32 instance_extension_count = sizeof(instance_extensions) / sizeof(instance_extensions[0]);
    for (u32 i = 0; i < instance_extension_count; ++i) {
        bool found = false;
        for (u32 j = 0; j < available_count; ++j) {
            if (strcmp(instance_extensions[i], available[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

bool CheckDeviceExtensionSupport(VkExtensionProperties available[], u32 available_count)
{
    u32 device_extension_count = sizeof(device_extensions) / sizeof(device_extensions[0]);
    for (u32 i = 0; i < device_extension_count; ++i) {
        bool found = false;
        for (u32 j = 0; j < available_count; ++j) {
            if (strcmp(device_extensions[i], available[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

void LoadVulkanGlobalFunctions()
{
#define VK_GLOBAL_FUNCTION(name)                                           \
    if (!(name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name))) {         \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void LoadVulkanInstanceFunctions(VkInstance instance)
{
#define VK_INSTANCE_FUNCTION(name)                                         \
    if (!(name = (PFN_##name)vkGetInstanceProcAddr(instance, #name))) {        \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void LoadVulkanInstanceExtensionFunctions(VkInstance instance)
{
#define VK_INSTANCE_FUNCTION_EXT(name)                                     \
    if (!(name = (PFN_##name)vkGetInstanceProcAddr(instance, #name))) {        \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void LoadVulkanDeviceFunctions(VkDevice device)
{
#define VK_DEVICE_FUNCTION(name)                                           \
    if (!(name = (PFN_##name)vkGetDeviceProcAddr(device, #name))) {          \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void LoadVulkanDeviceExtensionFunctions(VkDevice device)
{
#define VK_DEVICE_FUNCTION_EXT(name)                                       \
    if (!(name = (PFN_##name)vkGetDeviceProcAddr(device, #name))) {            \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}