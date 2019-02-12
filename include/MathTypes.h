
#pragma once

struct Vec2
{
    union
    {
        struct
        {
            float x, y;
        };
        struct
        {
            float r, g;
        };
        float data[2];
    };
};

// Example of operator overloading. Should probably have at least
// scalar add, sub, mult, div, vector add, sub, mult (dot), div?, stream.
inline Vec2 operator*(float scalar, Vec2 vector)
{
    Vec2 result;
    result.x = vector.x * scalar;
    result.y = vector.y * scalar;
    return result;
}
inline Vec2 operator*(Vec2 vector, float scalar) {return scalar * vector;}

struct Vec3
{
    union
    {
        struct
        {
            float x, y, z;
        };
        struct
        {
            float r, g, b;
        };
        float data[3];
    };
};

struct Vec4
{
    union
    {
        struct
        {
            float x, y, z, w;
        };
        struct
        {
            float r, g, b, a;
        };
        float data[4];
    };
};

// TODO(Matt): Find a way to ensure that this doesn't get padded. It doesn't on my platform but I imagine there are some ways to force this.
// Vertex struct, 64 bytes. (To be renamed once GLM is out)
struct VertexX
{
    Vec3 position;
    Vec3 normal;
    Vec3 tangent;
    Vec3 color;
    Vec2 uv0;
    Vec2 uv1;
};