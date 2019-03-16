
#version 450

struct DirectionalLight
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
};

struct UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 view_pos;
    DirectionalLight sun;
};

layout(push_constant) uniform PushBlock
{
    uint draw_index;
    int scalar_parameters[7];
    uint texture_indices[8];
    vec4 vector_parameters[4];
} push_block;

layout(binding = 0) uniform UniformData {
    UniformBufferObject uniforms[64];
} uniform_data;

layout(set = 0, binding = 1) uniform sampler samplers[16];
layout(set = 0, binding = 2) uniform texture2D textures[8];

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_uv0;

layout(location = 0) out vec4 out_color;

void main()
{
    //if (texture(texture_sampler, in_uv0).r < 0.333) discard;
    float alpha = texture(sampler2D(textures[push_block.texture_indices[0]], samplers[0]), in_uv0).r;
    out_color = vec4(1.0, 1.0, 1.0, alpha);    
}
