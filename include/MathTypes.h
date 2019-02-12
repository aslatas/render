// NOTE(Matt): This file isn't being used. I've toyed with the idea of using
// custom matrix and vector types, but I'm leaning instead towards stripping 
// out all the un-used parts of GLM. The math isn't too bad, but there are
// so many ways to access and modify a matrix that I figure letting someone
// else do all the C++ type tomfoolery might be the best plan.
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

// Tricky to get initializers and accessors here - by row/column, plus
// special matrices like identity.
struct Mat4
{
    union
    {
        Vec4 rows[4];
        float data[4][4];
    };
};
