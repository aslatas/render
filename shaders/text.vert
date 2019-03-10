
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

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec2 out_uv0;
void main() {
    out_position = in_position;
    out_normal = in_normal;
    out_color = in_color.rgb;
	out_uv0 = in_uv0;
    gl_Position = vec4(in_position, 1.0f);
}
