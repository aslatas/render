
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
    VkCommandPool primary_command_pool;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_set_layout;
    VkSampleCountFlagBits msaa_samples;
    VkFence *in_flight_fences;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
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
    uint32_t current_frame;
    uint32_t pipeline_count;
    VkImage color_image;
    VkDeviceMemory color_image_memory;
    VkImageView color_image_view;
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView depth_image_view;
    VkFormat depth_format;
    
    // Heap allocated (make sure they get freed):
    VkPipelineLayout *pipeline_layouts;
    VkPipeline *pipelines;
    VkFramebuffer *framebuffers;
    VkImage *images;
    VkImageView *imageviews;
    VkCommandBuffer *primary_command_buffers;
};

// Reads a shader file as a heap allocated byte array.
// TODO(Matt): Merge me with the shader module creation, to simplify the usage code.
char *ReadShaderFile(char *path, uint32_t *length);

void InitializeRenderer();
void ShutdownRenderer();
// Draw the next frame.
void DrawFrame();

void CreatePipeline(VkPipeline *pipeline, VkPipelineLayout *pipeline_layout, char *vert_code, char *frag_code);
void CreateStencilPipeline(VkPipeline *pipeline, VkPipelineLayout *pipeline_layout, char *vert_code);
void CreateOutlinePipeline(VkPipeline *pipeline, VkPipelineLayout *pipeline_layout, char *vert_code, char *frag_code);
void CreateVertexBuffer(Model *model);
void CreateIndexBuffer(Model *model);
void CreateUniformBuffers(Model *model);
void CreateDescriptorSets(Model *model);

void OnWindowResized();
void UpdateUniforms(uint32_t image_index, Model *model);

void SelectObject(int32_t mouse_x, int32_t mouse_y, bool accumulate);

void RecordPrimaryCommand(uint32_t image_index);
void UpdateModels(double frame_delta);