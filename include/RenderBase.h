
#pragma once

#include "Win32PlatformLayer.h"
#include "RenderTypes.h"

// TODO(Matt): Move most of these vars into an ini file or something.
// TODO(Matt): Whip up a heap alloc'd array, or grab stb stretchybuffer.
// There's a lot of easy to leak heap memory in here.
#define MAX_FRAMES_IN_FLIGHT 2
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

// Stores vulkan instance/device information not dependent on swapchain.
struct VulkanInfo
{
    VkInstance instance;
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
    VkPipeline *pipelines;
    uint32_t pipeline_count;
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
void CreatePipeline(VkPipeline *pipeline, char *vert_code, char *frag_code);
void CreateFramebuffers();
void CreateCommandPool();
void CreateDescriptorPool();
void CreateVertexBuffer(Model *model);
void CreateIndexBuffer(Model *model);
void CreateUniformBuffers(Model *model);
void CreateDescriptorSets(Model *model);
void CreateCommandBuffers();
void CreateSyncPrimitives();

// TODO(Matt): Fix some of the sloppiness in these next few functions.
void RecreateSwapchain();
void CleanupSwapchain();
void UpdateUniforms(uint32_t image_index, Model *model);
void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
void CopyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size);
uint32_t FindMemoryType(uint32_t type, VkMemoryPropertyFlags properties);
bool CheckValidationLayerSupport(VkLayerProperties available[], uint32_t available_count);
bool CheckInstanceExtensionSupport(VkExtensionProperties available[], uint32_t available_count);
bool CheckDeviceExtensionSupport(VkExtensionProperties available[], uint32_t available_count);

// Loads vulkan functions dynamically.
void LoadVulkanGlobalFunctions();
void LoadVulkanInstanceFunctions(VkInstance instance);
void LoadVulkanInstanceExtensionFunctions(VkInstance instance);
void LoadVulkanDeviceFunctions(VkDevice device);
void LoadVulkanDeviceExtensionFunctions(VkDevice device);