
#pragma once

#include "VulkanFunctions.h"

static char *validation_layers[] = {"VK_LAYER_LUNARG_standard_validation"};
static char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// NOTE(Matt): Validation is enabled/disabled based on the NDEBUG flag.
// This requires loading different instance/device extensions too.
// TODO(Matt): Platform Specific - move the win32 surface extension.
#ifdef NDEBUG
static bool enable_validation = false;
static char *instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
#else
static bool enable_validation = true;
static char *instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#endif


static uint32_t instance_extension_count = ARRAYSIZE(instance_extensions);
static uint32_t device_extension_count = ARRAYSIZE(device_extensions);
static uint32_t validation_layer_count = ARRAYSIZE(validation_layers);

bool CheckValidationLayerSupport(VkLayerProperties available[], uint32_t available_count);
bool CheckInstanceExtensionSupport(VkExtensionProperties available[], uint32_t available_count);
bool CheckDeviceExtensionSupport(VkExtensionProperties available[], uint32_t available_count);

// Loads vulkan functions dynamically.
void LoadVulkanGlobalFunctions();
void LoadVulkanInstanceFunctions(VkInstance instance);
void LoadVulkanInstanceExtensionFunctions(VkInstance instance);
void LoadVulkanDeviceFunctions(VkDevice device);
void LoadVulkanDeviceExtensionFunctions(VkDevice device);