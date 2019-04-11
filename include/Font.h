
#ifndef FONT_H
#define FONT_H

struct BitmapFont
{
    Texture texture; // Bitmap texture for the font.
    stbtt_fontinfo info; // Font info to calculate bounds and spacing.
    stbtt_bakedchar *character_data; // Character data for this font.
    unsigned char first_character; // Index of the first ANSI character.
    unsigned char character_count; // Number of ANSI characters to use.
    // TODO(Matt): Probably some kind of identifier here.
    u32 material_type;
    u32 shader_id;
};

// Creates a font bitmap from a ttf file. Returns an empty font if unable.
BitmapFont LoadBitmapFont(const char *path, u32 material_type, u32 shader_id, u32 first_character = 32, u32 character_count = 96,  u32 resolution = 1024, float character_size = 128.0f, bool generate_mips = true);

// TODO(Matt): Add a static size multiplier (as a cheap hack).
// TODO(Matt): Handle newlines in the input.
// Creates text on screen for a given font. Size is determined by the font.
// Position is in screen space. Newline characters are ignored.
//Model CreateText(const char *text, const BitmapFont *font,  glm::vec2 screen_postion);

// Call only while queue is idle.
void DestroyFont(BitmapFont *font);

#endif