
#include "Texture.h"

// Loads a texture from a PNG or JPG file. 
Texture LoadTexture(const char *path, u32 channel_count, bool generate_mips)
{
    Texture texture = {};
    int width, height, channels;
    stbi_uc *buffer = stbi_load(path, &width, &height, &channels, channel_count);
    texture.width = (u32)width;
    texture.height = (u32)height;
    texture.channel_count = (u32)channel_count;
    VkDeviceSize image_size = texture.width * texture.height * texture.channel_count;
    texture.mip_count = (generate_mips) ? 1 + (u32)log2(fmax(texture.width, texture.height)) : 1;
    if (!buffer)
    {
        std::cerr << "Unable to load texture! (" << path << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    
    CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    UpdateDeviceMemory(buffer, image_size, staging_buffer_memory);
    stbi_image_free(buffer);
    
    VkFormat format = GetFormatFromChannelCount(texture.channel_count);
    CreateImage(texture.width, texture.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.image, &texture.device_memory, texture.mip_count, VK_SAMPLE_COUNT_1_BIT);
    
    TransitionImageLayout(texture.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mip_count);
    CopyBufferToImage(staging_buffer, texture.image, texture.width, texture.height);
    
    CreateMipmaps(&texture);
    
    DestroyDeviceBuffer(staging_buffer);
    FreeDeviceMemory(staging_buffer_memory);
    texture.image_view = CreateImageView(texture.image, format, VK_IMAGE_ASPECT_COLOR_BIT, texture.mip_count);
    return texture;
}

VkFormat GetFormatFromChannelCount(u32 channel_count)
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

void DestroyTexture(Texture *texture)
{
    DestroyImageView(texture->image_view);
    DestroyImage(texture->image);
    FreeDeviceMemory(texture->device_memory);
    *texture = {};
}

void CreateMipmaps(Texture *texture)
{
    VkFormatProperties properties = GetFormatProperties(GetFormatFromChannelCount(texture->channel_count));
    if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        std::cerr << "Texture filtering is unsupported!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkCommandBuffer command_buffer = BeginOneTimeCommand();
    
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = texture->image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    
    u32 mip_width = texture->width;
    u32 mip_height = texture->height;
    
    for (u32 i = 1; i < texture->mip_count; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { (s32)mip_width, (s32)mip_height, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { (mip_width > 1) ? (s32)mip_width / 2 : 1, (mip_height > 1) ? (s32)mip_height / 2 : 1, 1 };
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
    
    EndOneTimeCommand(command_buffer);
}