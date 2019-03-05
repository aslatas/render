
#include "Texture.h"
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Loads a texture from a PNG file. 
Texture LoadTexture(const VulkanInfo *vulkan_info, const char *path, uint32_t channel_count, bool generate_mips)
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
    
    CreateBuffer(vulkan_info, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info->logical_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, buffer, image_size);
    vkUnmapMemory(vulkan_info->logical_device, staging_buffer_memory);
    stbi_image_free(buffer);
    
    VkFormat format = GetFormatFromChannelCount(texture.channel_count);
    CreateImage(vulkan_info, texture.width, texture.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.image, &texture.device_memory, texture.mip_count, VK_SAMPLE_COUNT_1_BIT);
    
    TransitionImageLayout(vulkan_info, texture.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mip_count);
    CopyBufferToImage(vulkan_info, staging_buffer, texture.image, texture.width, texture.height);
    
    CreateMipmaps(vulkan_info, &texture);
    
    vkDestroyBuffer(vulkan_info->logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, staging_buffer_memory, nullptr);
    texture.image_view = CreateImageView(vulkan_info, texture.image, format, VK_IMAGE_ASPECT_COLOR_BIT, texture.mip_count);
    CreateTextureSampler(vulkan_info, &texture);
    return texture;
}

void CreateTextureSampler(const VulkanInfo *vulkan_info, Texture *texture)
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
    
    VK_CHECK_RESULT(vkCreateSampler(vulkan_info->logical_device, &sampler_create_info, nullptr, &texture->sampler));
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

void DestroyTexture(const VulkanInfo *vulkan_info, Texture *texture)
{
    vkDestroySampler(vulkan_info->logical_device, texture->sampler, nullptr);
    vkDestroyImageView(vulkan_info->logical_device, texture->image_view, nullptr);
    vkDestroyImage(vulkan_info->logical_device, texture->image, nullptr);
    vkFreeMemory(vulkan_info->logical_device, texture->device_memory, nullptr);
    *texture = {};
}

void CreateMipmaps(const VulkanInfo *vulkan_info, Texture *texture)
{
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(vulkan_info->physical_device, GetFormatFromChannelCount(texture->channel_count), &properties);
    if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        std::cerr << "Texture filtering is unsupported!" << std::endl;
        exit(EXIT_FAILURE);
    }
    VkCommandBuffer command_buffer = BeginOneTimeCommand(vulkan_info);
    
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = texture->image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    
    uint32_t mip_width = texture->width;
    uint32_t mip_height = texture->height;
    
    for (uint32_t i = 1; i < texture->mip_count; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { (int32_t)mip_width, (int32_t)mip_height, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { (mip_width > 1) ? (int32_t)mip_width / 2 : 1, (mip_height > 1) ? (int32_t)mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        
        vkCmdBlitImage(command_buffer,
                       texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        
        if (mip_width > 1) mip_width /= 2;
        if (mip_height > 1) mip_height /= 2;
    }
    
    barrier.subresourceRange.baseMipLevel = texture->mip_count - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
    
    EndOneTimeCommand(vulkan_info, command_buffer);
}