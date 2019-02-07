// TODO(Matt): Dynamic vulkan library loading.
// TODO(Matt): Replace GLM?
// TODO(Matt): Better timing than chrono?
// TODO(Matt): Separate out the platform layer.
#pragma once
#include "WindowBase.h"
#define VK_USE_PLATFORM_WIN32_KHR
#pragma warning(push, 0)
#include "vulkan/vulkan.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#pragma warning(pop)
#include <iostream>
#include <chrono>
#include <cstring>
#include <cstdlib>

#define MAX_FRAMES_IN_FLIGHT 2

static char *validation_layers[] = {"VK_LAYER_LUNARG_standard_validation"};
static char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
static bool enable_validation = false;
static char *instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
#else
static bool enable_validation = true;
static char *instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#endif

static uint32_t instance_extension_count = ARRAYSIZE(instance_extensions);
static uint32_t device_extension_count = ARRAYSIZE(device_extensions);
static uint32_t validation_layer_count = ARRAYSIZE(validation_layers);

// Holds info about the vulkan instance which is not re-created with the swapchain.
struct VulkanInfo
{
    VkInstance vulkan_instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkSurfaceKHR surface;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_index;
    uint32_t present_index;
    bool use_shared_queue;
    VkCommandPool command_pool;
    VkDescriptorPool descriptor_pool;
};

// TODO(Matt): Figure out something better for buffers.
struct BufferInfo
{
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkBuffer *uniform_buffers;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    VkDeviceMemory *uniform_buffers_memory;
};

// Holds vulkan info which is re-created with the swapchain.
struct SwapchainInfo
{
    VkSwapchainKHR swapchain;
    VkImage *images;
    VkImageView *imageviews;
    VkFramebuffer *framebuffers;
    uint32_t image_count;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    VkSurfaceTransformFlagBitsKHR transform;
    VkRenderPass renderpass;
    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkDescriptorSet *descriptor_sets;
    VkDescriptorSetLayout *descriptor_set_layouts;
    VkCommandBuffer *command_buffers;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkFence *in_flight_fences;
    uint32_t current_frame;
};

// TODO(Matt): Decide on a vertex struct, multiple of 64 bytes.
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

static Vertex vertices[] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
static uint32_t vertex_count = ARRAYSIZE(vertices);

static uint16_t indices[] = {
    0, 1, 2, 2, 3, 0
};
static uint32_t index_count = ARRAYSIZE(indices);

static bool CheckValidationLayerSupport(VkLayerProperties available[], uint32_t available_count)
{
    for (uint32_t i = 0; i < validation_layer_count; ++i) {
        bool found = false;
        for (uint32_t j = 0; j < available_count; ++j) {
            if (strcmp(validation_layers[i], available[j].layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

static bool CheckInstanceExtensionSupport(VkExtensionProperties available[], uint32_t available_count)
{
    for (uint32_t i = 0; i < instance_extension_count; ++i) {
        bool found = false;
        for (uint32_t j = 0; j < available_count; ++j) {
            if (strcmp(instance_extensions[i], available[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

static bool CheckDeviceExtensionSupport(VkExtensionProperties available[], uint32_t available_count)
{
    for (uint32_t i = 0; i < device_extension_count; ++i) {
        bool found = false;
        for (uint32_t j = 0; j < available_count; ++j) {
            if (strcmp(device_extensions[i], available[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}



VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* messenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* allocator);


static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

static char *ReadShaderFile(char *path, uint32_t *length)
{
    FILE * file = fopen (path, "rb");
    if (!file) return nullptr;
    fseek (file, 0, SEEK_END);
    *length = ftell(file);
    fseek (file, 0, SEEK_SET);
    char *buffer = (char *)malloc(*length);
    fread (buffer, 1, *length, file);
    fclose (file);
    return buffer;
}

void InitializeVulkan(WindowInfo *window_info, VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info);
// TODO(Matt): Pass struct pointers into these instead of using globals.
static void CreateInstance(VulkanInfo *vulkan_info);
static void CreateDebugMessenger(VulkanInfo *vulkan_info);
static void CreateSurface(WindowInfo *window_info, VulkanInfo *vulkan_info);
static void ChoosePhysicalDevice(VulkanInfo *vulkan_info);
static void CreateLogicalDevice(VulkanInfo *vulkan_info);
static void CreateSwapchain(WindowInfo *window_info, VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
static void CreateImageviews(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
static void CreateRenderpass(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
static void CreateDescriptorSetLayout(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
static void CreatePipeline(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
static void CreateFramebuffers(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
static void CreateCommandPool(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
static void CreateVertexBuffer(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info);
static void CreateIndexBuffer(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info);
static void CreateUniformBuffers(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info);
static void CreateDescriptorPool(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
static void CreateDescriptorSets(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info);
static void CreateCommandBuffers(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info);
static void CreateSyncPrimitives(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);

// TODO(Matt): Fix some of the sloppiness in these next few functions.
void DrawFrame(WindowInfo *window_info, VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info,BufferInfo *buffer_info);
void RecreateSwapchain(WindowInfo *window_info, VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info);
void CleanupSwapchain(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);
void UpdateUniforms(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info, uint32_t image_index);
void CreateBuffer(VulkanInfo *vulkan_info, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);

void CopyBuffer(VulkanInfo *vulkan_info, VkBuffer source, VkBuffer destination, VkDeviceSize size);
uint32_t FindMemoryType(VulkanInfo *vulkan_info, uint32_t type, VkMemoryPropertyFlags properties);