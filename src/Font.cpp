
#include "Font.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

BitmapFont LoadBitmapFont(const char *path, const VulkanInfo *vulkan_info, uint32_t shader_id, uint32_t first_character, uint32_t character_count, uint32_t resolution, float character_size, bool generate_mips)
{
    FILE *file = fopen(path, "rb");
    if (!file) {
        std::cerr << "Unable to load font! (" << path << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    uint32_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * length);
    fread(buffer, 1, length, file);
    fclose(file);
    
    BitmapFont font = {};
    font.shader_id = shader_id;
    font.first_character = first_character;
    font.character_count = character_count;
    font.texture.width = resolution;
    font.texture.height = resolution;
    font.texture.channel_count = 1;
    unsigned char *bitmap = (unsigned char *)malloc(sizeof(unsigned char) * resolution * resolution);
    font.character_data = (stbtt_bakedchar *)malloc(sizeof(stbtt_bakedchar) * character_count);
    stbtt_BakeFontBitmap(buffer, 0, character_size, bitmap, resolution, resolution, first_character, character_count, font.character_data);
    font.texture.mip_count = (generate_mips) ?  1 + (uint32_t)log2(resolution) : 1;
    VkDeviceSize image_size = resolution * resolution;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    void *data;
    vkMapMemory(vulkan_info->logical_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, bitmap, image_size);
    vkUnmapMemory(vulkan_info->logical_device, staging_buffer_memory);
    CreateImage(resolution, resolution, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &font.texture.image, &font.texture.device_memory, font.texture.mip_count, VK_SAMPLE_COUNT_1_BIT);
    TransitionImageLayout(font.texture.image, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, font.texture.mip_count);
    CopyBufferToImage(staging_buffer, font.texture.image, resolution, resolution);
    
    // TODO(Matt): I think there's a layout transition that happens in here,
    // which should happen all the time.
    if (generate_mips) GenerateMipmaps(font.texture.image, VK_FORMAT_R8_UNORM, resolution, resolution, font.texture.mip_count);
    
    vkDestroyBuffer(vulkan_info->logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, staging_buffer_memory, nullptr);
    font.texture.image_view = CreateImageView(font.texture.image, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, font.texture.mip_count);
    CreateTextureSampler(&font.texture, vulkan_info);
    free(bitmap);
    free(buffer);
    return font;
}

// TODO(Matt): Font sizing via stb_truetype font attributes.
Model CreateText(const char *text, const BitmapFont *font, glm::vec2 screen_position, glm::vec2 screen_size)
{
    Model model = {};
    uint32_t length = (uint32_t)strlen(text);
    model.vertex_count = length * 4;
    model.index_count = length * 6;
    model.vertices = (Vertex *)malloc(sizeof(Vertex) * model.vertex_count);
    model.indices = (uint32_t *)malloc(sizeof(uint32_t) * model.index_count);
    
    // NOTE(Matt): Convert to NDC.
    screen_size /= 2.0f;
    screen_position.x /= screen_size.x;
    screen_position.y /= screen_size.y;
    for (uint32_t i = 0; i < length; ++i) {
        if (text[i] >= font->first_character && text[i] < font->first_character + font->character_count) {
            stbtt_aligned_quad quad;
            stbtt_GetBakedQuad(font->character_data, font->texture.width,font->texture.height, text[i] - font->first_character, &screen_position.x, &screen_position.y, &quad, 1);
            quad.x0 /= screen_size.x;
            quad.x1 /= screen_size.x;
            quad.y0 /= screen_size.y;
            quad.y1 /= screen_size.y;
            model.vertices[(i * 4) + 0].position = glm::vec3(quad.x0, quad.y0, 0.0f);
            model.vertices[(i * 4) + 1].position = glm::vec3(quad.x0, quad.y1, 0.0f);
            model.vertices[(i * 4) + 2].position = glm::vec3(quad.x1, quad.y1, 0.0f);
            model.vertices[(i * 4) + 3].position = glm::vec3(quad.x1, quad.y0, 0.0f);
            
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
    // TODO(Matt): Remove all this transform business once UI objects
    // are separate from models.
    model.shader_id = font->shader_id;
    model.pos = {0.0f, 0.0f, 0.0f};
    model.rot = glm::vec3(0.0f);
    model.scl = glm::vec3(1.0f);
    model.hit_test_enabled = false;
    model.bounds = {};
    model.ubo.model = glm::mat4(1.0f);
    model.ubo.view_position = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    model.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
    model.ubo.projection[1][1] *= -1;
    
    return model;
}

void DestroyFont(BitmapFont *font, const VulkanInfo *vulkan_info)
{
    free(font->character_data);
    DestroyTexture(&font->texture, vulkan_info);
    *font = {};
}