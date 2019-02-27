
#version 450
#extension GL_ARB_separate_shader_objects : enable

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

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_uv0;

layout(location = 0) out vec4 out_color;

void main()
{ 	
    // diffuse 
    vec3 norm = normalize(in_normal);
    vec3 light_dir = normalize(-ubo.sun.direction.rgb);  
    vec3 diffuse = max(dot(norm, light_dir), 0.0) * ubo.sun.diffuse.rgb;  
    
    // specular
    vec3 view_dir = normalize(ubo.view_pos.rgb - in_position);
    vec3 reflect_dir = reflect(-light_dir, norm);  
    vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), 64.0f) * ubo.sun.specular.rgb;  
        
    vec3 result = ubo.sun.ambient.rgb + diffuse + specular;
    out_color = vec4(result * texture(texture_sampler, in_uv0).rgb, 1.0f);

}

