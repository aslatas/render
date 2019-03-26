
#ifndef VULKANINIT_H

#define APPLICATION_NAME "NYCEngine Prototype"
#define ENGINE_NAME "NYCE"
// Macro checks the result of a vulkan call, to verify success.
// TODO(Matt): Maybe redefine to do nothing when not validating?
#define VK_CHECK_RESULT(function)                                              \
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

// Initializes all of the static vulkan state.
void InitializeVulkan(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);

// Recreates the swapchain, usually because of a window resize.
void RecreateSwapchain(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);

// Frees vulkan objects, shuts down renderer.
void ShutdownVulkan(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);

// Destroys all scene objects.
void DestroyScene();

// Creates a Vulkan buffer, backed by device memory.
void CreateBuffer(const VulkanInfo *vulkan_info, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);

// Copies the contents of one buffer to another, usually to move from a
// staging buffer to a device-local one.
void CopyBuffer(const VulkanInfo *vulkan_info, VkBuffer source, VkBuffer destination, VkDeviceSize size);

// Finds a valid device memory type for the given properties.
u32 FindMemoryType(const VulkanInfo *vulkan_info, u32 type, VkMemoryPropertyFlags properties);

// Allocates and starts a transient command buffer.
VkCommandBuffer BeginOneTimeCommand(const VulkanInfo *vulkan_info);

// Ends and destroys a transient command buffer.
void EndOneTimeCommand(const VulkanInfo *vulkan_info, VkCommandBuffer command_buffer);

// Copies the contents of a buffer into an image.
void CopyBufferToImage(const VulkanInfo *vulkan_info, VkBuffer buffer, VkImage image, u32 width, u32 height);

// Transitions an image from one layout to another.
void TransitionImageLayout(const VulkanInfo *vulkan_info, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, u32 mip_count);

// Creates a Vulkan image, given its properties. Used for textures and
// also swapchain attachments.
void CreateImage(const VulkanInfo *vulkan_info, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *image_memory, u32 mips, VkSampleCountFlagBits samples);

// Creates a Vulkan imageview, given an image and format.
VkImageView CreateImageView(const VulkanInfo *vulkan_info, VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, u32 mip_count);

// Finds a supported image format, given a preferential list of candidates.
VkFormat FindSupportedFormat(const VulkanInfo *vulkan_info, VkFormat *acceptable_formats, u32 acceptable_count, VkImageTiling tiling, VkFormatFeatureFlags features);

// Destroys the swapchain. Should be called before shutting down the rest of the renderer.
void DestroySwapchain(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info);

#define VULKANINIT_H
#endif