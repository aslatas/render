
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

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec2 in_uv0;
layout(location = 4) in vec2 in_uv1;
layout(location = 5) in vec2 in_uv2;

layout(location = 2) out vec3 out_color;

void main() {
    out_color = in_color.rgb;
    gl_Position = vec4(in_position, 1.0f);
}
