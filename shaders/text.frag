
#version 450
#include "Uniforms.glsl"
#include "FragmentLayout.glsl"

void main()
{
    //if (texture(texture_sampler, in_uv0).r < 0.333) discard;
    float alpha = GetTextureValue(0, in_uv0).r;
    out_color = vec4(1.0, 1.0, 1.0, alpha);    
}
