
// Describes parameters of a directional (sun) light.
struct DirectionalLight
{
    vec4 direction; // Normalized light direction.
    vec4 diffuse; // Diffuse light color/intensity.
    vec4 specular; // Specular light color/intensity.
    vec4 ambient; // Ambieng light color/intensity.
};

// Uniform data which varies every frame.
struct PerFrame
{
    vec4 view_pos; // World location of the camera.
    mat4 view; // View matrix.
    mat4 proj; // Perspective projection matrix.
};

// Uniform data which varies for every render pass.
struct PerPass
{
    DirectionalLight sun; // Directional light.
};

// Uniform data which varies for every object.
struct PerDraw
{
    mat4 model; // Model matrix of the object.
};

// Push constant block, varies per object.
layout(push_constant) uniform PushBlock
{
    uint draw_index; // Index of the object.
    int scalar_parameters[7]; // Scalar material parameters.
    uint texture_indices[8]; // Texture indices, 8 or 16.
    vec4 vector_parameters[4]; // Vector material parameters.
} push_block;

// Uniform information.
layout(set = 0, binding = 0) uniform UniformData {
    PerFrame per_frame; // Per-frame data.
    PerPass per_pass; // Per-pass data.
    PerDraw uniforms[64]; // Object uniforms. Used with push-constant to get
    // per-draw uniform data.
} uniform_data;

layout(set = 0, binding = 1) uniform sampler samplers[16];
layout(set = 0, binding = 2) uniform texture2D textures[8];

// Gets the model (world) transform of the current object.
mat4 GetModelTransform()
{
    return uniform_data.uniforms[push_block.draw_index].model;
}

// Gets the view transform of the current frame.
mat4 GetViewTransform()
{
    return uniform_data.per_frame.view;
}

// Gets the projection matrix for the current frame.
mat4 GetProjectionTransform()
{
    return uniform_data.per_frame.proj;
}

// Gets the location of the view camera, in world space.
vec3 GetCameraLocation()
{
    return uniform_data.per_frame.view_pos.rgb;
}

// Gets the directional light for the scene, if there is one.
DirectionalLight GetDirectionalLight()
{
    return uniform_data.per_pass.sun;
}

vec4 GetTextureValue(uint index, vec2 uv)
{
    return texture(sampler2D(textures[push_block.texture_indices[index]], samplers[index]), uv);
}