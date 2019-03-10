
#pragma once

#include "Texture.h"
#include "stb/stb_truetype.h"

struct BitmapFont
{
    Texture texture; // Bitmap texture for the font.
    stbtt_bakedchar *character_data; // Character data for this font.
    unsigned char first_character; // Index of the first ANSI character.
    unsigned char character_count; // Number of ANSI characters to use.
    // TODO(Matt): Probably some kind of identifier here.
    uint32_t material_type;
    uint32_t shader_id;
};

// Creates a font bitmap from a ttf file. Returns an empty font if unable.
BitmapFont LoadBitmapFont(const VulkanInfo *vulkan_info, const char *path, uint32_t material_type, uint32_t shader_id, uint32_t first_character = 32, uint32_t character_count = 96,  uint32_t resolution = 512, float character_size = 64.0f, bool generate_mips = true);

Model CreateText(const char *text, const BitmapFont *font, uint32_t material_type, uint32_t shader_id, uint32_t uniform_count,  glm::vec2 screen_postion, glm::vec2 screen_size);

// Call only while queue is idle.
void DestroyFont(const VulkanInfo *vulkan_info, BitmapFont *font);