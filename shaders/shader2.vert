
#version 450
#extension GL_ARB_separate_shader_objects : enable
struct DirectionalLight
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
};

layout(push_constant) uniform PushBlock
{
    uint draw_index;
    int scalar_parameters[7];
    uint texture_indices[8];
    vec4 vector_parameters[4];
} push_block;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 view_pos;
    DirectionalLight sun;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_tangent;
layout(location = 3) in vec4 in_color;
layout(location = 4) in vec2 in_uv0;
layout(location = 5) in vec2 in_uv1;
layout(location = 6) in vec2 in_uv2;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec2 out_uv0;
void main() {
    vec4 pos = ubo.model * vec4(in_position, 1.0f);
    out_position = pos.rgb;
    out_normal = mat3(transpose(inverse(ubo.model))) * in_normal;
    out_color = in_color.rgb;
	out_uv0 = in_uv0;
    gl_Position = ubo.proj * ubo.view * pos;
}

