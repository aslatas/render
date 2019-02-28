
#pragma once

#include "RenderTypes.h"

// TODO(Matt): Move most of these vars into an ini file or something.
// TODO(Matt): Whip up a heap alloc'd array, or grab stb stretchybuffer.
// There's a lot of easy to leak heap memory in here.
#define MAX_FRAMES_IN_FLIGHT 2


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
    VkImage texture_image;
    VkImageView texture_image_view;
    VkDeviceMemory texture_memory;
    VkSampler texture_sampler;
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
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView depth_image_view;
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

void CreateTextureImage(char *file);

// TODO(Matt): Fix some of the sloppiness in these next few functions.
void RecreateSwapchain();
void CleanupSwapchain();
void UpdateUniforms(uint32_t image_index, Model *model);
void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
void CopyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size);
uint32_t FindMemoryType(uint32_t type, VkMemoryPropertyFlags properties);

VkCommandBuffer BeginOneTimeCommand();
void EndOneTimeCommand(VkCommandBuffer command_buffer);


void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *image_memory);
void CreateTextureImageView();
VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask);
void CreateTextureSampler();


VkFormat FindSupportedFormat(VkFormat *acceptable_formats, uint32_t acceptable_count, VkImageTiling tiling, VkFormatFeatureFlags features);
void CreateDepthResources();
VkFormat FindDepthFormat();