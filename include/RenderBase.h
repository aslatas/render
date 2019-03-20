
#pragma once

#include "RenderTypes.h"

// TODO(Matt): Move most of these vars into an ini file or something.
// TODO(Matt): Whip up a heap alloc'd array, or grab stb stretchybuffer.
// There's a lot of easy to leak heap memory in here.
#define MAX_FRAMES_IN_FLIGHT 2
// Number of samplers allowed by a material. Note that this 16 is the
// guaranteed minimum, and the push constant block size assumes this as
// well.
#define MATERIAL_SAMPLER_COUNT 16

// Maximum number of loaded textures.
#define MAX_TEXTURES 8

#define MAX_OBJECTS 64
// Max LOD for texture samplers. 16 Levels allows for unreasonably large
// textures.
#define MAX_SAMPLER_LOD 16

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

// Push block has 128 bytes (the maximum guaranteed size):
//   -  4 byte Draw Index (for indexing into the uniform buffer).
//   - 48 byte user data (3 vectors of 4 channels).
//   - 32 byte texture index array (for accessing up to 16 textures).
//   - 60 byte user data (7 scalars at 4 bytes each).
// NOTE(Matt): If a material uses 8 or fewer textures, it can access texture
// indices directly. Otherwise, 16 bit indices can be packed instead, but must
// be manually unpacked in the shader.
struct PushConstantBlock
{
    uint32_t draw_index;
    int32_t scalar_parameters[7];
    union 
    {
        uint16_t thin[MATERIAL_SAMPLER_COUNT];
        uint32_t wide[MATERIAL_SAMPLER_COUNT / 2];
    } texture_indices;
    glm::vec4 vector_parameters[4];
};

struct DescriptorLayout
{
    VkSampler samplers[MATERIAL_SAMPLER_COUNT];
    VkDescriptorSetLayout *descriptor_layouts;
};
// Holds info about a material layout. Keeps a list of materials of its'
// type.
struct MaterialLayout
{
    VkPipelineLayout pipeline_layout; // Pipeline layout.
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
//void CreateVertexBuffer(Model *model);
// Creates the index buffer for a given model, from its index array.
//void CreateIndexBuffer(Model *model);

// Creates descriptor sets for a given model, from its material type and
// uniform info.
void CreateDescriptorSets();

void CreateModelBuffer(VkDeviceSize buffer_size, void* buffer_data, VkBuffer* buffer, VkDeviceMemory* buffer_memory, VkBufferUsageFlagBits flags);

// Callback for window resize. Recreates the swapchain.
void OnWindowResized();
// Updates the uniform buffers for a model to reflect its current state.
void UpdateUniforms(uint32_t image_index);

// Performs a raycast in world space from the mouse location, selecting the
// first object intersected. If accumulate is true, objects are added to a
// list of currently selected. If accumulate is true AND the first object
// hit is already selected, that object is de-selected. Only models with the
// "hit test visible" flag set are tested against.
void SelectObject(int32_t mouse_x, int32_t mouse_y, bool accumulate);

// Records draw calls for the current frame.
void RecordPrimaryCommand(uint32_t image_index);
// Updates the models in the scene with a new time.
void UpdatePrePhysics(double delta);
// Performs a physics update step.
void UpdatePhysics(double delta);
void UpdatePostPhysics(double delta);
void UpdatePostRender(double delta);
// Sets up a pipeline create info with defaults. Pass nullptr for fragment
// code if not needed.
MaterialCreateInfo CreateDefaultMaterialInfo(const char *vert_file, const char *frag_file);

// Creates a graphics pipeline from create info generated by a call
// to CreateDefaultPipelineInfo().
void AddMaterial(MaterialCreateInfo *material_info, uint32_t material_type, VkRenderPass render_pass, uint32_t sub_pass);

// Creates the material layout.
// TODO(Matt): Parameterize this, so that multiple layouts are possible.
MaterialLayout CreateMaterialLayout();
void CreateDescriptorLayout();

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

// Updates a descriptor set with the texture bindings.
void UpdateTextureDescriptors(VkDescriptorSet descriptor_set);

// Create the immutable samplers for a material layout.
void CreateSamplers(DescriptorLayout *layout);

void CreateGlobalUniformBuffers();
PerDrawUniformObject *GetPerDrawUniform(uint32_t object_index);
PerFrameUniformObject *GetPerFrameUniform();
PerPassUniformObject *GetPerPassUniform();