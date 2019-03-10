
#version 450

struct DirectionalLight
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 view_pos;
    DirectionalLight sun;
} ubo;

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 2) in vec3 in_color;

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(in_color, 1.0f);
}
