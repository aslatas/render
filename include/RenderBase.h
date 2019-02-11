
#pragma once
#pragma warning(push, 0)
#define _CRT_SECURE_NO_WARNINGS
#define VK_USE_PLATFORM_WIN32_KHR
#include "WindowBase.h"
#include "vulkan/vulkan.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma warning(pop)

// TODO(Matt): Move most of these vars into an ini file or something.
#define MAX_FRAMES_IN_FLIGHT 2
static char *validation_layers[] = {"VK_LAYER_LUNARG_standard_validation"};
static char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// NOTE(Matt): Validation is enabled/disabled based on the NDEBUG flag.
// This requires loading different instance/device extensions too.
// TODO(Matt): Platform Specific - move the win32 surface extension.
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

// Stores vulkan instance/device information not dependent on swapchain.
struct VulkanInfo
{
    VkInstance vulkan_instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkSurfaceKHR surface;
    VkDebugUtilsMessengerEXT debug_messenger;
    uint32_t graphics_index;
    uint32_t present_index;
    VkQueue graphics_queue;
    VkQueue present_queue;
    // NOTE(Matt): Likely, graphics and present are the same queue. If so,
    // calls to Vulkan cannot treat them as separate, hence this flag.
    bool use_shared_queue;
    VkCommandPool command_pool;
    VkDescriptorPool descriptor_pool;
    
};

// Stores vulkan information that must be recreated with the swapchain.
struct SwapchainInfo
{
    VkSwapchainKHR swapchain;
    uint32_t image_count;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    VkSurfaceTransformFlagBitsKHR transform;
    VkRenderPass renderpass;
    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    uint32_t current_frame;
    
    // Heap allocated (make sure they get freed):
    VkFramebuffer *framebuffers;
    VkImage *images;
    VkImageView *imageviews;
    VkCommandBuffer *command_buffers;
    VkDescriptorSet *descriptor_sets;
    VkDescriptorSetLayout *descriptor_set_layouts;
    VkFence *in_flight_fences;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
};

// Stores info about geometry to draw, including UBOs.
// TODO(Matt): Extend this to handle multiple objects in the trivial case.
struct BufferInfo
{
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    
    // Heap allocated:
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
};

// TODO(Matt): Decide on a vertex struct, multiple of 64 bytes.
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

// TODO(Matt): Decide on a standard UBO for opaque objects, or choose a way to link objects with their UBOs.
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* messenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* allocator);

// Debug callback, prints validation layer messages to the log file.
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

// Reads a shader file as a heap allocated byte array.
// TODO(Matt): Merge me with the shader module creation, to simplify the usage code.
char *ReadShaderFile(char *path, uint32_t *length);

// Initialize renderer and attach to the window.
void InitializeVulkan();

// Draw the next frame.
void DrawFrame();

// Shutdown vulkan and free associated memory.
void ShutdownVulkan();
// Helpers to initialize vulkan. 
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
void RecreateSwapchain();
void CleanupSwapchain();
void UpdateUniforms(uint32_t image_index);
void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);

void CopyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size);
uint32_t FindMemoryType(uint32_t type, VkMemoryPropertyFlags properties);
bool CheckValidationLayerSupport(VkLayerProperties available[], uint32_t available_count);
bool CheckInstanceExtensionSupport(VkExtensionProperties available[], uint32_t available_count);
bool CheckDeviceExtensionSupport(VkExtensionProperties available[], uint32_t available_count);
