
#pragma once
#include "RenderBase.h"
#include "VulkanInit.h"

// Holds information about an image texture.
struct Texture
{
    VkImage image; // Vulkan image object.
    VkImageView image_view; // Vulkan imageview object. Must be recreated
    // on swapchain resize.
    VkDeviceMemory device_memory; // Vulkan device memory.
    uint32_t channel_count; // Number of channels (R/G/B/A)
    uint32_t mip_count; // Number of mip levels, 1 indicates no mipmaps.
    uint32_t width; // Texture width.
    uint32_t height; // Texture height.
    // TODO(Matt): Some kind of identifier would be good.
};

// Loads a texture from a PNG file. 
Texture LoadTexture(const VulkanInfo *vulkan_info, const char *path, uint32_t channel_count, bool generate_mips);

// Destroys a texture, freeing device memory and Vulkan objects.
// Only call while queue is idle.
void DestroyTexture(const VulkanInfo *vulkan_info, Texture *texture);

// Gets a VkFormat for an image from the number of channels it uses.
VkFormat GetFormatFromChannelCount(uint32_t channel_count);

void CreateMipmaps(const VulkanInfo *vulkan_info, Texture *texture);