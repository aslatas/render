// TODO(Matt): Dynamic vulkan library loading.
// TODO(Matt): Replace GLM?
// TODO(Matt): Better timing than chrono?
// TODO(Matt): Separate out the platform layer.
#pragma once
#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _CRT_SECURE_NO_WARNINGS
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

#define WNDCLASS_NAME L"WindowClass"
#define WINDOW_TITLE L"Rendering Prototype"
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

// TODO(Matt): Command pool - info or swapchain?
// TODO(Matt): Vertex/index buffers, uniforms, etc. - Where?
// TODO(Matt): Sync objects - Where?
struct RenderInfo
{
    HWND window;
    bool is_initialized;
    bool is_minimized;
    bool is_resizing;
    VkInstance vulkan_instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkSurfaceKHR surface;
    VkDebugUtilsMessengerEXT debug_messenger;
    uint32_t graphics_index;
    uint32_t present_index;
    VkQueue graphics_queue;
    VkQueue present_queue;
    bool use_shared_queue;
    VkCommandPool command_pool;
    VkDescriptorPool descriptor_pool;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkBuffer *uniform_buffers;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    VkDeviceMemory *uniform_buffers_memory;
};

struct Swapchain
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

Vertex vertices[] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
uint32_t vertex_count = ARRAYSIZE(vertices);
uint16_t indices[] = {
    0, 1, 2, 2, 3, 0
};
uint32_t index_count = ARRAYSIZE(indices);
LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

void CreateWin32Window(HINSTANCE instance, int command_show);

int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int command_show);

bool CheckValidationLayerSupport(VkLayerProperties available[], uint32_t available_count)
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

bool CheckInstanceExtensionSupport(VkExtensionProperties available[], uint32_t available_count)
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

bool CheckDeviceExtensionSupport(VkExtensionProperties available[], uint32_t available_count)
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

char *ReadShaderFile(char *path, uint32_t *length)
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

// TODO(Matt): Pass struct pointers into these instead of using globals.
void CreateInstance();
void CreateDebugMessenger();
void CreateSurface();
void ChoosePhysicalDevice();
void CreateLogicalDevice();
void CreateSwapchain();
void CreateImageviews();
void CreateRenderpass();
void CreateDescriptorSetLayout();
void CreatePipeline();
void CreateFramebuffers();
void CreateCommandPool();
void CreateVertexBuffer();
void CreateIndexBuffer();
void CreateUniformBuffers();
void CreateDescriptorPool();
void CreateDescriptorSets();
void CreateCommandBuffers();
void CreateSyncPrimitives();
// TODO(Matt): Fix some of the sloppiness in these next few functions.
void DrawFrame();
void RecreateSwapchain();
void CleanupSwapchain();
void UpdateUniforms(uint32_t image_index);
void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);

void CopyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size);
uint32_t FindMemoryType(uint32_t type, VkMemoryPropertyFlags properties);