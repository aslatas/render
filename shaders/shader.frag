
#version 450
#include "Uniforms.glsl"
#include "FragmentLayout.glsl"

void main()
{
    DirectionalLight sun = GetDirectionalLight();
    // diffuse 
    vec3 norm = normalize(in_normal);
    vec3 light_dir = normalize(-sun.direction.rgb);  
    vec3 diffuse = max(dot(norm, light_dir), 0.0) * sun.diffuse.rgb;  
    
    // specular
    vec3 view_dir = normalize(GetCameraLocation().rgb - in_position);
    vec3 reflect_dir = reflect(-light_dir, norm);  
    vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), 64.0f) * sun.specular.rgb;  
    
    vec3 result = sun.ambient.rgb + diffuse + specular;
    out_color = vec4(result * in_color.rgb, 1.0);
}

