
#ifndef TEXTURE_H
// Holds information about an image texture.
struct Texture
{
    VkImage image; // Vulkan image object.
    VkImageView image_view; // Vulkan imageview object. Must be recreated
    // on swapchain resize.
    VkDeviceMemory device_memory; // Vulkan device memory.
    u32 channel_count; // Number of channels (R/G/B/A)
    u32 mip_count; // Number of mip levels, 1 indicates no mipmaps.
    u32 width; // Texture width.
    u32 height; // Texture height.
    // TODO(Matt): Some kind of identifier would be good.
};

// Loads a texture from a PNG file. 
Texture LoadTexture(const VulkanInfo *vulkan_info, const char *path, u32 channel_count, bool generate_mips);

// Destroys a texture, freeing device memory and Vulkan objects.
// Only call while queue is idle.
void DestroyTexture(const VulkanInfo *vulkan_info, Texture *texture);

// Gets a VkFormat for an image from the number of channels it uses.
VkFormat GetFormatFromChannelCount(u32 channel_count);

void CreateMipmaps(const VulkanInfo *vulkan_info, Texture *texture);

#define TEXTURE_H
#endif