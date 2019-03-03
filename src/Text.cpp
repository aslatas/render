
#include "Text.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

static stbtt_bakedchar cdata[96];
Texture LoadFontTexture(char *path, uint32_t resolution, bool generate_mips, const VulkanInfo *vulkan_info)
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
    unsigned char *bitmap = (unsigned char *)malloc(sizeof(unsigned char) * resolution * resolution);
    stbtt_BakeFontBitmap(buffer, 0, 64.0f, bitmap, resolution, resolution, 32, 96, cdata);
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

Model CreateText(char *text, float x, float y, float screen_width, float screen_height)
{
    Model model;
    uint32_t length = (uint32_t)strlen(text);
    model.vertex_count = length * 4;
    model.index_count = length * 6;
    model.vertices = (Vertex *)calloc(model.vertex_count, sizeof(Vertex));
    model.indices = (uint32_t *)calloc(model.index_count, sizeof(uint32_t));
    screen_width /= 2;
    screen_height /= 2;
    x /= screen_width;
    y /= screen_height;
    for (uint32_t i = 0; i < length; ++i) {
        if (text[i] >= 32 && text[i] < 128) {
            stbtt_aligned_quad quad;
            stbtt_GetBakedQuad(cdata, 512,512, text[i]-32, &x, &y, &quad, 1);
            model.vertices[(i * 4) + 0].position = glm::vec3(quad.x0 / screen_width, quad.y0 / screen_height, 0.0f);
            model.vertices[(i * 4) + 1].position = glm::vec3(quad.x0 / screen_width, quad.y1 / screen_height, 0.0f);
            model.vertices[(i * 4) + 2].position = glm::vec3(quad.x1 / screen_width, quad.y1 / screen_height, 0.0f);
            model.vertices[(i * 4) + 3].position = glm::vec3(quad.x1 / screen_width, quad.y0 / screen_height, 0.0f);
            
            model.vertices[(i * 4) + 0].normal = glm::vec3(0.0f, -1.0f, 0.0f);
            model.vertices[(i * 4) + 1].normal = glm::vec3(0.0f, -1.0f, 0.0f);
            model.vertices[(i * 4) + 2].normal = glm::vec3(0.0f, -1.0f, 0.0f);
            model.vertices[(i * 4) + 3].normal = glm::vec3(0.0f, -1.0f, 0.0f);
            
            model.vertices[(i * 4) + 0].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            model.vertices[(i * 4) + 1].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            model.vertices[(i * 4) + 2].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            model.vertices[(i * 4) + 3].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            
            model.vertices[(i * 4) + 0].uv0 = glm::vec2(quad.s0, quad.t0);
            model.vertices[(i * 4) + 1].uv0 = glm::vec2(quad.s0, quad.t1);
            model.vertices[(i * 4) + 2].uv0 = glm::vec2(quad.s1, quad.t1);
            model.vertices[(i * 4) + 3].uv0 = glm::vec2(quad.s1, quad.t0);
            
            model.vertices[(i * 4) + 0].uv1 = glm::vec2(quad.s0, quad.t0);
            model.vertices[(i * 4) + 1].uv1 = glm::vec2(quad.s0, quad.t1);
            model.vertices[(i * 4) + 2].uv1 = glm::vec2(quad.s1, quad.t1);
            model.vertices[(i * 4) + 3].uv1 = glm::vec2(quad.s1, quad.t0);
            
            model.vertices[(i * 4) + 0].uv2 = glm::vec2(quad.s0, quad.t0);
            model.vertices[(i * 4) + 1].uv2 = glm::vec2(quad.s0, quad.t1);
            model.vertices[(i * 4) + 2].uv2 = glm::vec2(quad.s1, quad.t1);
            model.vertices[(i * 4) + 3].uv2 = glm::vec2(quad.s1, quad.t0);
            
            model.indices[(i * 6) + 0] = (i * 4) + 0;
            model.indices[(i * 6) + 1] = (i * 4) + 1;
            model.indices[(i * 6) + 2] = (i * 4) + 2;
            model.indices[(i * 6) + 3] = (i * 4) + 0;
            model.indices[(i * 6) + 4] = (i * 4) + 2;
            model.indices[(i * 6) + 5] = (i * 4) + 3;
        }
    }
    model.shader_id = 4;
    model.pos = {0.0f, 0.0f, 0.0f};
    model.rot = glm::vec3(0.0f);
    model.scl = glm::vec3(1.0f);
    model.bounds.min = glm::vec3(0.0f);
    model.bounds.max = glm::vec3(1.0f);
    model.ubo.model = glm::mat4(1.0f);
    model.ubo.view_position = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    model.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
    model.ubo.projection[1][1] *= -1;
    
    return model;
}