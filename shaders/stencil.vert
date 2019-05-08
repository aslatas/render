
#version 450
#include "Uniforms.glsl"
#include "VertexLayout.glsl"

void main() {
    gl_Position = GetProjectionTransform() * GetViewTransform() * GetModelTransform() * vec4(in_position, 1.0f);
}