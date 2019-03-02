
#include "Text.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
Texture LoadFont(char *path, uint32_t resolution, bool generate_mips, const VulkanInfo *vulkan_info)
{
    FILE *file = fopen(path, "rb");
    if (!file) {
        std::cerr << "Unable to load font file!" << std::endl;
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    uint32_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * length);
    fread(buffer, 1, length, file);
    fclose(file);
    Texture texture;
    stbtt_bakedchar cdata[96];
    unsigned char *bitmap = (unsigned char *)malloc(sizeof(unsigned char) * resolution * resolution);
    stbtt_BakeFontBitmap(buffer, 0, 32.0, bitmap, resolution, resolution, 32, 96, cdata);
    texture.mip_count = (generate_mips) ?  1 + (uint32_t)log2(resolution) : 1;
    VkDeviceSize image_size = resolution * resolution;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    void *data;
    vkMapMemory(vulkan_info->logical_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, bitmap, image_size);
    vkUnmapMemory(vulkan_info->logical_device, staging_buffer_memory);
    CreateImage(resolution, resolution, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.image, &texture.device_memory, texture.mip_count, VK_SAMPLE_COUNT_1_BIT);
    TransitionImageLayout(texture.image, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mip_count);
    CopyBufferToImage(staging_buffer, texture.image, resolution, resolution);
    
    if (generate_mips) GenerateMipmaps(texture.image, VK_FORMAT_R8_UNORM, resolution, resolution, texture.mip_count);
    
    vkDestroyBuffer(vulkan_info->logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, staging_buffer_memory, nullptr);
    free(bitmap);
    free(buffer);
    
    VkImageViewCreateInfo view_create_info = {};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = texture.image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = VK_FORMAT_R8_UNORM;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = texture.mip_count;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;
    
    if (vkCreateImageView(vulkan_info->logical_device, &view_create_info, nullptr, &texture.image_view) != VK_SUCCESS)
    {
        std::cerr << "Unable to create image view!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
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
    sampler_create_info.maxLod = (generate_mips) ? (float)texture.mip_count : 0.0f;
    
    if (vkCreateSampler(vulkan_info->logical_device, &sampler_create_info, nullptr, &texture.sampler) != VK_SUCCESS)
    {
        std::cerr << "Unable to create texture sampler!" << std::endl;
        exit(EXIT_FAILURE);
    }
    return texture;
}