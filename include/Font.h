
#pragma once

#include "Texture.h"
#include "stb/stb_truetype.h"

struct BitmapFont
{
    Texture texture; // Bitmap texture for the font.
    stbtt_fontinfo info; // Font info to calculate bounds and spacing.
    stbtt_bakedchar *character_data; // Character data for this font.
    unsigned char first_character; // Index of the first ANSI character.
    unsigned char character_count; // Number of ANSI characters to use.
    // TODO(Matt): Probably some kind of identifier here.
    uint32_t material_type;
    uint32_t shader_id;
};

// Creates a font bitmap from a ttf file. Returns an empty font if unable.
BitmapFont LoadBitmapFont(const VulkanInfo *vulkan_info, const char *path, uint32_t material_type, uint32_t shader_id, uint32_t first_character = 32, uint32_t character_count = 96,  uint32_t resolution = 1024, float character_size = 128.0f, bool generate_mips = true);

// TODO(Matt): Add a static size multiplier (as a cheap hack).
// TODO(Matt): Handle newlines in the input.
// Creates text on screen for a given font. Size is determined by the font.
// Position is in screen space. Newline characters are ignored.
//Model CreateText(const char *text, const BitmapFont *font,  glm::vec2 screen_postion);

// Call only while queue is idle.
void DestroyFont(const VulkanInfo *vulkan_info, BitmapFont *font);