
#version 450

#include "Uniforms.glsl"

// Vertex position.
layout(location = 0) in vec3 in_position;
// Instance color.
layout(location = 1) in vec4 in_color;
// Instance user data.
layout(location = 2) in vec4 in_user;
// Instance transform.
layout(location = 3) in mat4 in_transform;

// Output color (unmodified).
layout(location = 0) out vec4 out_color;

// Output userdata (unmodified).
layout(location = 1) out vec4 out_user;

void main() {
    out_color = in_color;
    out_user = in_user;
    vec4 pos = in_transform * vec4(in_position, 1.0f);
    gl_Position = GetProjectionTransform() * GetViewTransform() * pos;
	//gl_Position = GetProjectionTransform() * pos;
}