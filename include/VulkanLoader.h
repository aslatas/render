
#ifndef VULKANLOADER_H
global char *validation_layers[] = {"VK_LAYER_LUNARG_standard_validation"};
global char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// NOTE(Matt): Validation is enabled/disabled based on the NDEBUG flag.
// This requires loading different instance/device extensions too.
// TODO(Matt): Platform Specific - move the win32 surface extension.
#ifdef NDEBUG
global bool enable_validation = false;
global char *instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
global PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
#else
global bool enable_validation = true;
global char *instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#endif

bool CheckValidationLayerSupport(VkLayerProperties available[], u32 available_count);
bool CheckInstanceExtensionSupport(VkExtensionProperties available[], u32 available_count);
bool CheckDeviceExtensionSupport(VkExtensionProperties available[], u32 available_count);

// Loads vulkan functions dynamically.
void LoadVulkanGlobalFunctions();
void LoadVulkanInstanceFunctions(VkInstance instance);
void LoadVulkanInstanceExtensionFunctions(VkInstance instance);
void LoadVulkanDeviceFunctions(VkDevice device);
void LoadVulkanDeviceExtensionFunctions(VkDevice device);

#define VULKANLOADER_H
#endif