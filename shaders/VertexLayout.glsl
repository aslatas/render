
// Vertex position.
layout(location = 0) in vec3 in_position;
// Vertex normal.
layout(location = 1) in vec3 in_normal;
// Vertex tangent.
layout(location = 2) in vec4 in_tangent;
// Vertex color.
layout(location = 3) in vec4 in_color;
// Primary vertex UV coordinate.
layout(location = 4) in vec2 in_uv0;
// Secondary (lightmap) UV coordinate.
layout(location = 5) in vec2 in_uv1;
// User Secondary UV coordinate.
layout(location = 6) in vec2 in_uv2;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec4 out_color;
layout(location = 4) out vec2 out_uv0;
layout(location = 5) out vec2 out_uv1;
layout(location = 6) out vec2 out_uv2;