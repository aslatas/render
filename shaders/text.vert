
#version 450
#include "Uniforms.glsl"
#include "VertexLayout.glsl"

void main() {
    out_position = in_position;
    out_normal = in_normal;
    out_tangent = in_tangent.rgb;
    out_color = in_color;
	out_uv0 = in_uv0;
    out_uv1 = in_uv1;
    out_uv2 = in_uv2;
    gl_Position = vec4(in_position, 1.0f);
}
