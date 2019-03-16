
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

layout(set = 0, binding = 1) uniform sampler samp;
layout(set = 0, binding = 2) uniform texture2D textures[8];

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;

layout(location = 0) out vec4 out_color;

void main()
{
    UniformBufferObject ubo = uniform_data.uniforms[push_block.draw_index];
    // diffuse 
    vec3 norm = normalize(in_normal);
    vec3 light_dir = normalize(-ubo.sun.direction.rgb);  
    vec3 diffuse = max(dot(norm, light_dir), 0.0) * ubo.sun.diffuse.rgb;  
    
    // specular
    vec3 view_dir = normalize(ubo.view_pos.rgb - in_position);
    vec3 reflect_dir = reflect(-light_dir, norm);  
    vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), 64.0f) * ubo.sun.specular.rgb;  
    
    vec3 result = ubo.sun.ambient.rgb + diffuse + specular;
    out_color = vec4(result * in_color, 1.0);
    
}

