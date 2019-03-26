
#version 450
#include "FragmentLayout.glsl"

void main()
{
    out_color = vec4(in_color.rgb, 1.0f);
}
