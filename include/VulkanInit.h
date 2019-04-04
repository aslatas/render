
#ifndef VULKANINIT_H
#define VULKANINIT_H

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

#define APPLICATION_NAME "NYCEngine Prototype"
#define ENGINE_NAME "NYCE"
// Macro checks the result of a vulkan call, to verify success.
// TODO(Matt): Maybe redefine to do nothing when not validating?
#define VK_CHECK_RESULT(function)                                      \
{                                                                      \
	VkResult result = (function);                                      \
	if (result != VK_SUCCESS)                                          \
	{                                                                  \
        std::cerr << "Error in " << __FILE__ << ", line " <<           \
        __LINE__ << "! message: \"" << "VkResult was " <<              \
        result << "\"." << std::endl;                                  \
        exit(EXIT_FAILURE);                                            \
	}                                                                  \
}

// Stores vulkan information that must be recreated with the swapchain.
struct SwapchainInfo
{
    VkSwapchainKHR swapchain; // Swapchain object.
    u32 image_count; // Swapchain image count (also uniform count).
    VkSurfaceFormatKHR format; // Swapchain surface format.
    VkPresentModeKHR present_mode; // Swapchain present mode.
    VkExtent2D extent; // Swapchain extent.
    VkRenderPass renderpass; // TODO(Matt): Multiple render passes.
    u32 current_frame; // Current image being processed.
    
    // TODO(Matt): Extract image, memory, and view to a struct.
    VkImage color_image; // Color attachment.
    VkDeviceMemory color_image_memory; // Color attachment memory.
    VkImageView color_image_view; // Color image view.
    VkImage depth_image; // Depth attachment.
    VkDeviceMemory depth_image_memory; // Depth attachment memory.
    VkImageView depth_image_view; // Depth image view.
    VkFormat depth_format; // Format of the depth buffer (uses stencil).
    
    // Heap allocated (make sure they get freed):
    VkFramebuffer *framebuffers; // Swapchain framebuffers.
    VkImage *images; // Swapchain images.
    VkImageView *imageviews; // Swapchain image views.
    VkCommandBuffer *primary_command_buffers; // Draw command buffers.
    
    VkDescriptorSet *descriptor_sets;
};

struct DescriptorLayout
{
    VkSampler samplers[MATERIAL_SAMPLER_COUNT];
    VkDescriptorSetLayout *descriptor_layouts;
};

// Holds info about a material, and the list of models that use it.
struct Material
{
    VkPipeline pipeline; // Pipeline object.
    u32 type; // Material type (by index).
    Model_Separate_Data *models; // Model list.
};

// Holds info about a material layout. Keeps a list of materials of its'
// type.
struct MaterialLayout
{
    VkPipelineLayout pipeline_layout; // Pipeline layout.
    Material *materials;
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
    u32 stage_count; // Number of shader stages (usually 2).
    
    // Heap allocated.
    VkPipelineShaderStageCreateInfo *shader_stages;
    VkShaderModule *shader_modules;
};

// Push block has 128 bytes (the maximum guaranteed size):
//   -  4 byte Draw Index (for indexing into the uniform buffer).
//   - 48 byte user data (3 vectors of 4 channels).
//   - 32 byte texture index array (for accessing up to 16 textures).
//   - 60 byte user data (7 scalars at 4 bytes each).
// NOTE(Matt): If a material uses 8 or fewer textures, it can access texture
// indices directly. Otherwise, 16 bit indices can be packed instead, but
// must be manually unpacked in the shader.
struct PushConstantBlock
{
    u32 draw_index;
    s32 scalar_parameters[7];
    union 
    {
        uint16_t thin[MATERIAL_SAMPLER_COUNT];
        u32 wide[MATERIAL_SAMPLER_COUNT / 2];
    } texture_indices;
    glm::vec4 vector_parameters[4];
};

// Initializes all of the static vulkan state.
void InitializeVulkan();

// Recreates the swapchain, usually because of a window resize.
void RecreateSwapchain();

// Frees vulkan objects, shuts down renderer.
void ShutdownVulkan();

// Destroys all scene objects.
void DestroyScene();

// Creates a Vulkan buffer, backed by device memory.
void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);

// Copies the contents of one buffer to another, usually to move from a
// staging buffer to a device-local one.
void CopyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size);

// Finds a valid device memory type for the given properties.
u32 FindMemoryType(u32 type, VkMemoryPropertyFlags properties);

// Allocates and starts a transient command buffer.
VkCommandBuffer BeginOneTimeCommand();

// Ends and destroys a transient command buffer.
void EndOneTimeCommand(VkCommandBuffer command_buffer);

// Copies the contents of a buffer into an image.
void CopyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height);

// Transitions an image from one layout to another.
void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, u32 mip_count);

// Creates a Vulkan image, given its properties. Used for textures and
// also swapchain attachments.
void CreateImage(u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *image_memory, u32 mips, VkSampleCountFlagBits samples);

// Creates a Vulkan imageview, given an image and format.
VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, u32 mip_count);

// Finds a supported image format, given a preferential list of candidates.
VkFormat FindSupportedFormat(VkFormat *acceptable_formats, u32 acceptable_count, VkImageTiling tiling, VkFormatFeatureFlags features);

// Destroys the swapchain. Should be called before shutting down the rest of the renderer.
void DestroySwapchain();

// TODO(Matt): Document theses.
VkShaderModule CreateShaderModule(char *code, u32 length);
void CreateDescriptorSets(VkBuffer *buffers);

// TODO(Matt): Swapchain recreation should be asynchronous if possible.
// Waits for the current frame fence and acquires a swapchain image.
// If the result is out of date, recreates the swapchain.
// Returns the index of the acquired image, to be used for uniform
// updates, command submission, and image presentation.
u32 WaitForNextImage();
void SubmitRenderCommands(u32 image_index);
void PresentFrame(u32 image_index);

void UpdateDeviceMemory(void *data, VkDeviceSize size, VkDeviceMemory memory);
MaterialLayout CreateMaterialLayout();
void CreateDescriptorLayout(DescriptorLayout *layout);
MaterialCreateInfo CreateDefaultMaterialInfo(const char *vert_file, const char *frag_file);
Material CreateMaterial(MaterialCreateInfo *material_info, VkPipelineLayout layout, u32 type, VkRenderPass renderpass, u32 sub_pass);
void DestroyDeviceBuffer(VkBuffer buffer);
void FreeDeviceMemory(VkDeviceMemory memory);
void DestroyPipeline(VkPipeline pipeline);
void DestroyPipelineLayout(VkPipelineLayout layout);
void DestroyDescriptorSetLayout(VkDescriptorSetLayout layout);

void DestroySampler(VkSampler sampler);
void DestroyImageView(VkImageView view);
void DestroyImage(VkImage image);
VkFormatProperties GetFormatProperties(VkFormat format);
void UpdateDescriptorSet(VkWriteDescriptorSet descriptor_write);
void CommandBeginRenderPass(u32 image_index);
void CommandBindPipeline(VkPipeline pipeline, u32 image_index);
void CommandBindVertexBuffer(VkBuffer buffer, size_t *offsets, u32 image_index);
void CommandBindIndexBuffer(VkBuffer buffer, VkIndexType type, u32 image_index);
void CommandPushConstants(VkPipelineLayout pipeline_layout, u32 stages, const PushConstantBlock *push_block, u32 image_index);
void CommandDrawIndexed(u32 image_index, u32 index_count);
void CommandEndRenderPass(u32 image_index);
void GetSwapchainExtent(u32 *width, u32 *height);
u32 GetSwapchainImageCount();
VkRenderPass GetSwapchainRenderPass();
void PresentNextFrame(u32 image_index);
#endif