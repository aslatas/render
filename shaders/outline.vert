
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

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_tangent;
layout(location = 3) in vec4 in_color;
layout(location = 4) in vec2 in_uv0;
layout(location = 5) in vec2 in_uv1;
layout(location = 6) in vec2 in_uv2;

void main() {
    UniformBufferObject ubo = uniform_data.uniforms[push_block.draw_index];
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position + in_normal * 0.01, 1.0f);
}