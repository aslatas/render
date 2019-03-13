
#pragma once

#include "RenderTypes.h"

// TODO(Matt): Move most of these vars into an ini file or something.
// TODO(Matt): Whip up a heap alloc'd array, or grab stb stretchybuffer.
// There's a lot of easy to leak heap memory in here.
#define MAX_FRAMES_IN_FLIGHT 2

// Stores vulkan information that must be recreated with the swapchain.
struct SwapchainInfo
{
    VkSwapchainKHR swapchain; // Swapchain object.
    uint32_t image_count; // Swapchain image count (also uniform count).
    VkSurfaceFormatKHR format; // Swapchain surface format.
    VkPresentModeKHR present_mode; // Swapchain present mode.
    VkExtent2D extent; // Swapchain extent.
    VkSurfaceTransformFlagBitsKHR transform; // Transform (always identity).
    VkRenderPass renderpass; // TODO(Matt): Multiple render passes.
    uint32_t current_frame; // Current image being processed.
    //uint32_t pipeline_count;
    VkImage color_image; // Color attachment.
    VkDeviceMemory color_image_memory; // Color attachment memory.
    VkImageView color_image_view; // Color image view.
    VkImage depth_image; // Depth attachment.
    VkDeviceMemory depth_image_memory; // Depth attachment memory.
    VkImageView depth_image_view; // Depth image view.
    VkFormat depth_format; // Format of the depth buffer (uses stencil).
    
    // Heap allocated (make sure they get freed):
    //VkPipelineLayout *pipeline_layouts;
    //VkPipeline *pipelines;
    VkFramebuffer *framebuffers; // Swapchain framebuffers/
    VkImage *images; // Swapchain images.
    VkImageView *imageviews; // Swapchain image views.
    VkCommandBuffer *primary_command_buffers; // Draw command buffers.
};

// Holds information necessary to create a material. To be filled in with
// defaults and then adjusted by the user.
struct MaterialCreateInfo
{
    VkPipelineVertexInputStateCreateInfo input_info;
    VkVertexInputBindingDescription binding_description[7];
    VkVertexInputAttributeDescription attribute_descriptions[7];
    VkPipelineInputAssemblyStateCreateInfo assembly_info;
    VkViewport viewport; // TODO(Matt): Dynamic?
    VkPipelineViewportStateCreateInfo viewport_info;
    VkRect2D scissor; // TODO(Matt): Dynamic?
    VkPipelineRasterizationStateCreateInfo raster_info;
    VkPipelineMultisampleStateCreateInfo multisample_info;
    VkPipelineColorBlendAttachmentState blend;
    VkPipelineColorBlendStateCreateInfo blend_info;
    VkPipelineDepthStencilStateCreateInfo depth_stencil;
    uint32_t stage_count; // Number of shader stages (usually 2).
    
    // Heap allocated.
    VkPipelineShaderStageCreateInfo *shader_stages;
    VkShaderModule *shader_modules;
};

// Holds info about a material, and the list of models that use it.
struct Material
{
    VkPipeline pipeline; // Pipeline object.
    uint32_t type; // Material type (by index).
    Model_Separate_Data *models; // Model list.
};

// Holds info about a material layout. Keeps a list of materials of its'
// type.
struct MaterialLayout
{
    VkPipelineLayout pipeline_layout; // Pipeline layout.
    
    // Heap allocated.
    VkDescriptorSetLayout *descriptor_layouts; // Descriptor layouts.
    Material *materials;
};


// Reads a shader file as a heap allocated byte array.
// TODO(Matt): Merge me with the shader module creation, to simplify the usage code.
char *ReadShaderFile(const char *path, uint32_t *length);

// Initializes the renderer, creating vulkan instance, device, etc.
void InitializeRenderer();
// Shuts down the renderer, freeing associated memory. Do not call while
// work is active on the GPU.
void ShutdownRenderer();

// Draws the next frame.
void DrawFrame();

// Creates the vertex buffer for a given model, from its vertex array.
void CreateVertexBuffer(Model *model);
// Creates the index buffer for a given model, from its index array.
void CreateIndexBuffer(Model *model);
// Creates uniform buffers for a given model, from its material type and
// uniform info.
void CreateUniformBuffers(Model *model);
// Creates descriptor sets for a given model, from its material type and
// uniform info.
void CreateDescriptorSets(Model *model);

void CreateModelBuffer(VkDeviceSize buffer_size, void* buffer_data, VkBuffer* buffer, VkDeviceMemory* buffer_memory, VkBufferUsageFlagBits flags);
void CreateModelUniformBuffers(VkDeviceSize buffer_size, 
                               VkBuffer* uniform_buffers, 
                               VkDeviceMemory* uniform_buffers_memory, 
                               uint32_t uniform_count);
void CreateModelDescriptorSets(uint32_t uniform_count, 
                               uint32_t material_type, 
                               uint32_t shader_id, 
                               VkBuffer* uniform_buffers, 
                               VkDescriptorSet *descriptor_sets);

// Callback for window resize. Recreates the swapchain.
void OnWindowResized();
// Updates the uniform buffers for a model to reflect its current state.
void UpdateUniforms(uint32_t image_index, Model_Separate_Data *model);

// Performs a raycast in world space from the mouse location, selecting the
// first object intersected. If accumulate is true, objects are added to a
// list of currently selected. If accumulate is true AND the first object
// hit is already selected, that object is de-selected. Only models with the
// "hit test visible" flag set are tested against.
void SelectObject(int32_t mouse_x, int32_t mouse_y, bool accumulate);

// Records draw calls for the current frame.
void RecordPrimaryCommand(uint32_t image_index);
// Updates the models in the scene with a new time.
void UpdateModels(double frame_delta);

// Sets up a pipeline create info with defaults. Pass nullptr for fragment
// code if not needed.
MaterialCreateInfo CreateDefaultMaterialInfo(const char *vert_file, const char *frag_file);

// Creates a graphics pipeline from create info generated by a call
// to CreateDefaultPipelineInfo().
void AddMaterial(MaterialCreateInfo *material_info, uint32_t material_type, VkRenderPass render_pass, uint32_t sub_pass);

// Creates the material layout.
// TODO(Matt): Parameterize this, so that multiple layouts are possible.
MaterialLayout CreateMaterialLayout();

// Creates the materials used by the scene.
void CreateMaterials();
// Destroys materials used by the scene. Performed on swapchain recreation.
void DestroyMaterials();
// Initializes models and other resources in the scene.
void InitializeScene();
// Adds a given model to the scene, using its material type and shader ID.
void AddToScene(Model_Separate_Data model);
// Destroys objects and materials in the scene.
void DestroyScene();

// Gets the renderer VulkanInfo struct.
const VulkanInfo *GetRenderInfo();
// Gets the renderer SwapchainInfo struct.
const SwapchainInfo *GetSwapchainInfo();
uint32_t GetUniformCount();
void InitializeSceneResources();
void DestroySceneResources();