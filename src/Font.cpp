
#include "Font.h"

BitmapFont LoadBitmapFont(const char *path, u32 material_type, u32 shader_id, u32 first_character, u32 character_count, u32 resolution, float character_size, bool generate_mips)
{
    BitmapFont font = {};
    FILE *file = fopen(path, "rb");
    if (!file) {
        std::cerr << "Unable to load font! (" << path << ")" << std::endl;
        return font;
    }
    fseek(file, 0, SEEK_END);
    u32 length = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * length);
    fread(buffer, 1, length, file);
    fclose(file);
    
    font.material_type = material_type;
    font.shader_id = shader_id;
    font.first_character = first_character;
    font.character_count = character_count;
    font.texture.width = resolution;
    font.texture.height = resolution;
    font.texture.channel_count = 1;
    unsigned char *bitmap = (unsigned char *)malloc(sizeof(unsigned char) * resolution * resolution);
    font.character_data = (stbtt_bakedchar *)malloc(sizeof(stbtt_bakedchar) * character_count);
    stbtt_BakeFontBitmap(buffer, 0, character_size, bitmap, resolution, resolution, first_character, character_count, font.character_data);
    font.texture.mip_count = (generate_mips) ?  1 + (u32)log2(resolution) : 1;
    VkDeviceSize image_size = resolution * resolution;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    UpdateDeviceMemory(bitmap, image_size, staging_buffer_memory);
    CreateImage(resolution, resolution, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &font.texture.image, &font.texture.device_memory, font.texture.mip_count, VK_SAMPLE_COUNT_1_BIT);
    TransitionImageLayout(font.texture.image, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, font.texture.mip_count);
    CopyBufferToImage(staging_buffer, font.texture.image, resolution, resolution);
    
    CreateMipmaps(&font.texture);
    
    DestroyDeviceBuffer(staging_buffer);
    FreeDeviceMemory(staging_buffer_memory);
    font.texture.image_view = CreateImageView(font.texture.image, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, font.texture.mip_count);
    free(bitmap);
    free(buffer);
    return font;
}
/*
// TODO(Matt): Font sizing via stb_truetype font attributes.
Model CreateText(const char *text, const BitmapFont *font,  glm::vec2 pos)
{
    Model model = {};
    model.material_type = font->material_type;
    model.shader_id = font->shader_id;
    model.uniform_count = GetUniformCount();
    u32 length = (u32)strlen(text);
    model.vertex_count = length * 4;
    model.index_count = length * 6;
    model.vertices = (Vertex *)malloc(sizeof(Vertex) * model.vertex_count);
    model.indices = (u32 *)malloc(sizeof(u32) * model.index_count);
    const SwapchainInfo *swapchain_info = GetSwapchainInfo();
    float half_width = (float)swapchain_info->extent.width;
    float half_height = (float)swapchain_info->extent.height;
    pos.x = pos.x * half_width * 2.0f;
    pos.y = pos.y * half_height * 2.0f;
    
    for (u32 i = 0; i < length; ++i) {
        if (text[i] >= font->first_character && text[i] < font->first_character + font->character_count) {
            stbtt_aligned_quad quad;
            stbtt_GetBakedQuad(font->character_data, font->texture.width,font->texture.height, text[i] - font->first_character, &pos.x, &pos.y, &quad, 1);
            quad.x0 = (quad.x0 - half_width) / half_width;
            quad.x1 = (quad.x1 - half_width) / half_width;
            quad.y0 = (quad.y0 - half_height) / half_height;
            quad.y1 = (quad.y1 - half_height) / half_height;
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
*/
void DestroyFont(BitmapFont *font)
{
    free(font->character_data);
    *font = {};
}