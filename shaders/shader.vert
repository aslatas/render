
#version 450
#include "Uniforms.glsl"
#include "VertexLayout.glsl"

void main() {
    mat4 model = GetModelTransform();
    vec4 pos = model * vec4(in_position, 1.0f);
    out_position = pos.rgb;
    out_normal = mat3(transpose(inverse(model))) * in_normal;
    out_tangent = mat3(transpose(inverse(model))) * in_tangent.rgb;
    out_color = in_color;
    out_uv0 = in_uv0;
    out_uv1 = in_uv1;
    out_uv2 = in_uv2;
    
    gl_Position = GetProjectionTransform() * GetViewTransform() * pos;
}

