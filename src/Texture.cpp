
#include "Texture.h"
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Loads a texture from a PNG file. 
Texture LoadTexture(const char *path, const VulkanInfo *vulkan_info, uint32_t channel_count, bool generate_mips)
{
    Texture texture = {};
    int width, height, channels;
    stbi_uc *buffer = stbi_load(path, &width, &height, &channels, channel_count);
    texture.width = (uint32_t)width;
    texture.height = (uint32_t)height;
    texture.channel_count = (uint32_t)channel_count;
    VkDeviceSize image_size = texture.width * texture.height * texture.channel_count;
    texture.mip_count = (generate_mips) ? 1 + (uint32_t)log2(fmax(texture.width, texture.height)) : 1;
    if (!buffer)
    {
        std::cerr << "Unable to load texture! (" << path << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    
    CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info->logical_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, buffer, image_size);
    vkUnmapMemory(vulkan_info->logical_device, staging_buffer_memory);
    stbi_image_free(buffer);
    
    VkFormat format = GetFormatFromChannelCount(texture.channel_count);
    CreateImage(texture.width, texture.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.image, &texture.device_memory, texture.mip_count, VK_SAMPLE_COUNT_1_BIT);
    
    TransitionImageLayout(texture.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mip_count);
    CopyBufferToImage(staging_buffer, texture.image, texture.width, texture.height);
    
    GenerateMipmaps(texture.image, format, texture.width, texture.height, texture.mip_count);
    
    vkDestroyBuffer(vulkan_info->logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, staging_buffer_memory, nullptr);
    texture.image_view = CreateImageView(texture.image, format, VK_IMAGE_ASPECT_COLOR_BIT, texture.mip_count);
    CreateTextureSampler(&texture, vulkan_info);
    return texture;
}

void CreateTextureSampler(Texture *texture, const VulkanInfo *vulkan_info)
{
    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy = 16;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0;
    sampler_create_info.maxLod = (texture->mip_count > 1) ? (float)texture->mip_count : 0.0f;
    
    if (vkCreateSampler(vulkan_info->logical_device, &sampler_create_info, nullptr, &texture->sampler) != VK_SUCCESS)
    {
        std::cerr << "Unable to create texture sampler!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

VkFormat GetFormatFromChannelCount(uint32_t channel_count)
{
    switch (channel_count) {
        case 1: return VK_FORMAT_R8_UNORM;
        case 2: return VK_FORMAT_R8G8_UNORM;
        case 3: return VK_FORMAT_R8G8B8_UNORM;
        case 4: return VK_FORMAT_R8G8B8A8_UNORM;
        default: 
        std::cerr << "Illegal channel count! (" << channel_count << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    return VK_FORMAT_R8G8B8A8_UNORM;
}

void DestroyTexture(Texture *texture, const VulkanInfo *vulkan_info)
{
    vkDestroySampler(vulkan_info->logical_device, texture->sampler, nullptr);
    vkDestroyImageView(vulkan_info->logical_device, texture->image_view, nullptr);
    vkDestroyImage(vulkan_info->logical_device, texture->image, nullptr);
    vkFreeMemory(vulkan_info->logical_device, texture->device_memory, nullptr);
    *texture = {};
}