
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

Texture LoadFontTexture(char *path, uint32_t resolution, bool generate_mips, const VulkanInfo *vulkan_info);

Model CreateText(char *text, float x, float y, float screen_width, float screen_height);