
#pragma once

#include "RenderBase.h"

struct Texture
{
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory device_memory;
    VkSampler sampler;
    uint32_t mip_count;
};

Texture LoadFont(char *path, uint32_t resolution, bool generate_mips, const VulkanInfo *vulkan_info);