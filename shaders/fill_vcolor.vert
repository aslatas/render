
#version 450
#include "Uniforms.glsl"
#include "VertexLayout.glsl"

void main() {
    out_color = in_color;
    gl_Position = vec4(in_position, 1.0f);
}
