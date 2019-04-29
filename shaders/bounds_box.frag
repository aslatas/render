
#version 450

// Instance color.
layout(location = 0) in vec4 in_color;
// Instance user data.
layout(location = 1) in vec4 in_user;

// Output color (unmodified).
layout(location = 0) out vec4 out_color;
void main()
{
    out_color = in_color;
}

