/*
TODO List:
-> Hash Tables
  -> Material
  -> Model
-> Lists
-> OctTree
-> Scene Manager

-> Frustum Culling
  -> Frustum Exctraction
  -> Furstum Culling

-> Occlusion Culling


Steps for implementation:
-> Add Model, adds to the Model HashTable
  -> Also add to the ModelData list
-> Add Material, add to the Mat HashTable
-> Adapt QuadTree to OctTree
-> LoadScene populates the OctTree
-> RenderScene travesrses to the leaf nodes, adds each model to the 
  render list. 
-> Integrate work so far into actual program
-> Frustum Culling
  -> Extract Frustum Planes
  -> AABB-Frustum intersection
-> Occlusion Culling
-> Integrate Culling into program
-> Hardware Occlusion Culling

-> Visual Techniques
  -> Second camera to get a high level view of the scene
  -> Build several scene examples
  -> Data gathering for render FPS, objects culled, and so forth

*/

// usefule defines
#define _CRT_SECURE_NO_WARNINGS

// C Lib Files
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <iostream>

// External Lib Files
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#define STBDS_SIPHASH_2_4
#include "stb/stb_truetype.h"
#include <stb/stb_ds.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#pragma warning(push, 0)
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/glm.hpp"
#pragma warning(pop)

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "tinygltf/tiny_gltf.h"

// Some convientent defines
typedef uint8_t u8;
#define u32 unsigned int
#define M_PI 3.14159

float GetSpeedMultiplier();

// Header Files
#include "Bounds.h"
// #include "Object.h"
// #include "Tree.h"
// #include "ModelLoader.h"
// #include "SceneManager.h"
#include "Camera.h"

// Config Parser Component headers
// #include "config_parsers/ConfigUtils.h"
// #include "config_parsers/SceneConfig.h"

// SRC Files
#include "Bounds.cpp"
// #include "Tree.cpp"
// #include "Object.cpp"
// #include "ModelLoader.cpp"
// #include "SceneManager.cpp"
#include "Camera.cpp"

/**
TODOS
  Adjust for the renderer:
     True Model Loading
     True Material Loading
     Create the render array
*/

static const float inchToMm = 25.4f;
enum FitResolutionGate { kFill = 0, kOverscan }; 

struct Triangle
{
    glm::vec2 vertA;
    glm::vec2 vertB;
    glm::vec2 vertC;
};

uint32_t default_mask = 0xFFFFFFFF;

struct Mask
{
    uint32_t mask = 0xFFFFFFFF; // 32bit depth mask for an 8x4
    float depth_0;
    float depth_1;
};

/*

*/

const uint32_t imageWidth = 640;
const uint32_t imageHeight = 480; 
const float nearClippingPLane = .1;
const float farClippingPLane = 1000;
float focalLength = 20; // in mm
// 35mm Full Aperture in inches
float filmApertureWidth = 0.980f;
float filmApertureHeight = 0.735f; 

float GetSpeedMultiplier()
{
  return 1.0f;
}

void print_aabb(AABB_3D &aabb)
{
  printf("Bounding Box:\n   Minimum: (%f, %f, %f)\n   Maximum: (%f, %f, %f)\n   Center: (%f, %f, %f)\n   Extent: (%f, %f, %f)\n", 
                    aabb.min[0], aabb.min[1], aabb.min[2],
                    aabb.max[0], aabb.max[1], aabb.max[2],
                    aabb.center[0], aabb.center[1], aabb.center[2], 
                    aabb.ext[0], aabb.ext[1], aabb.ext[2]);
}

void print_byte_as_bits(char val) {
  for (int i = 7; 0 <= i; i--) {
    printf("%c", (val & (1 << i)) ? '1' : '0');
  }
}

void print_bits(char * ty, char * val, unsigned char * bytes, size_t num_bytes) {
  printf("(%*s) %*s = [ ", 15, ty, 16, val);
  for (size_t i = 0; i < num_bytes; i++) {
    print_byte_as_bits(bytes[i]);
    printf(" ");
  }
  printf("]\n");
}

void print_mask(uint32_t mask)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int i = 7; 0 <= i; i--) {
            printf("%c", (mask & (1 << i)) ? '1' : '0');
        }
        printf("\n");
    }
}


#define SHOW(T,V) do { T x = V; print_bits(#T, #V, (unsigned char*) &x, sizeof(x)); } while(0)


bool CalculateBetween(glm::vec3 ray_max, glm::vec3 ray_min, glm::vec3 point_ray)
{
    float c = abs(dot(ray_max, ray_min));
    float res = abs(dot(ray_min, point_ray));

    // c == 1 is edge case where the ray_min/max are parallel to each other
    // there are rounding point errors
    bool b = c >= 0.99998f && c <= 1.0f || res <= c && res > 0;

    return (ray_min[0] * (-1 * point_ray[1]) + ray_min[1] * point_ray[0] >= 0) &&
           (ray_max[0] * (-1 * point_ray[1]) + ray_max[1] * point_ray[0] <= 0);
}

AABB_3D* Create3DAxisAlignedBoundingBoxFromCenter(float* center, float* ext)
{
    AABB_3D *aabb = (AABB_3D *)malloc(sizeof(AABB_3D));
    aabb->ext = glm::make_vec3(&ext[0]);
    aabb->center = glm::make_vec3(&center[0]);
    aabb->min = aabb->center - aabb->ext;
    aabb->max = aabb->center + aabb->ext; 

    return aabb;
}

void computeScreenCoordinates(
    const float &filmApertureWidth,
    const float &filmApertureHeight,
    const uint32_t &imageWidth,
    const uint32_t &imageHeight,
    const FitResolutionGate &fitFilm,
    const float &nearClippingPLane,
    const float &focalLength,
    float &top, float &bottom, float &left, float &right)
{
    float filmAspectRatio = filmApertureWidth / filmApertureHeight;
    float deviceAspectRatio = imageWidth / (float)imageHeight;

    top = ((filmApertureHeight * inchToMm / 2) / focalLength) * nearClippingPLane;
    right = ((filmApertureWidth * inchToMm / 2) / focalLength) * nearClippingPLane;

    // field of view (horizontal)
    float fov = 2.0f * 180.0f / (float)M_PI * atan((filmApertureWidth * inchToMm / 2.0f) / focalLength);
    // std::cerr << "Field of view " << fov << std::endl;

    float xscale = 1;
    float yscale = 1;

    switch (fitFilm) {
        default:
        case kFill:
            if (filmAspectRatio > deviceAspectRatio) 
            {
                xscale = deviceAspectRatio / filmAspectRatio;
            }
            else {
                yscale = filmAspectRatio / deviceAspectRatio;
            }
        break;
        case kOverscan:
            if (filmAspectRatio > deviceAspectRatio) {
                yscale = filmAspectRatio / deviceAspectRatio;
            }
            else {
                xscale = deviceAspectRatio / filmAspectRatio;
            }
        break;
    }

    right *= xscale;
    top *= yscale;

    bottom = -top;
    left = -right;
} 

glm::vec3 WorldToRaster(Camera::Camera cam, glm::vec3 point,
    const float &l,
    const float &r,
    const float &t,
    const float &b )
{
    float cam_near = cam.near_dist;
    float cam_far = cam.far_dist;
    glm::vec3 cam_loc = cam.location;

    glm::mat4 view = Camera::GetViewTransform(&cam);
    glm::vec4 vert_cam = view * glm::vec4(point, 1.0f);
    Camera::Frustum frust = *Camera::ExtractFrustumPlanes(cam, &view);
    glm::vec3 *points = frust.points;

    glm::vec2 vert_screen;
    vert_screen.x = 2.0f * cam_near * vert_cam.x / -vert_cam.z;
    vert_screen.y = 2.0f * cam_near * vert_cam.y / -vert_cam.z;

    // now convert point from screen space to NDC space (in range [-1,1])
    glm::vec2 vertexNDC;
    vertexNDC.x = 2 * vert_screen.x / (r - l) - (r + l) / (r - l);
    vertexNDC.y = 2 * vert_screen.y / (t - b) - (t + b) / (t - b); 

    // convert to raster space
    glm::vec3 vertexRaster;
    vertexRaster.x = (vertexNDC.x + 1) / 2 * imageWidth;
    // in raster space y is down so invert direction
    vertexRaster.y = (1 - vertexNDC.y) / 2 * imageHeight;
    vertexRaster.z = -vert_cam.z; 

    return vertexRaster;
}

// y is the y coordinate to get the x for
// returns the corresponding x coordinate
float line(glm::vec2 max, glm::vec2 min, float y)
{
    if (max[0] - min[0] == 0) return min[0]; // vertical line
    float m = (max[1] - min[1]) / (max[0] - min[0]);
    if (m == 0) return -1; // horizontal line
    return (y - min[1] + (m * min[0])) / (m);
}

void box_mask_test(glm::vec3 min, glm::vec3 max, Mask &mask, glm::vec2 pixel_start)
{
    // printf("BOX MAX TEST\n");
    Mask temp;
    // print_mask(temp.mask);

    min -= glm::vec3(pixel_start, 0.0f);
    max -= glm::vec3(pixel_start, 0.0f);

    glm::vec2 bl = min;
    glm::vec2 br = glm::vec2(max.x, min.y);
    glm::vec2 tl = glm::vec2(min.x, max.y);;
    glm::vec2 tr = max;

    for (int i = 0; i < 4; ++i)
    {
        temp.mask = (temp.mask >> (i * 8)) | (temp.mask << (32 - ((i) * 8)));

        uint8_t left = default_mask;
        uint8_t right = default_mask;
        uint8_t top = default_mask;
        uint8_t bottom = default_mask;

        int min_x = (int)line(tl, bl, i);
        if (min_x > 8) min_x = 8;
        else if (min_x < 0) min_x = 0;
        left = left >> min_x;

        int max_x = (int)line(tr, br, i);
        if (max_x < 0) max_x = 8;
        else if (max_x > 8) max_x = 0;
        // else if (max_x > (pixel_start[0] + 8)) max_x = 0;
        right = right << max_x;

        int min_y = (int)line(br, bl, i);
        if (min_y == -1)
        {
            // min_y = min[1] - pixel_start[1]; // y value from pixel start
            float tem = min[1] + pixel_start[1];
            if (tem < pixel_start[1]) min_y = 0;
            else if (tem > pixel_start[1])
            {
                if (tem > pixel_start[1] + i) min_y = 8;
                else min_y = 0; 
            }
            // if (min[1] < 0) min_y = 0;        // min y is above tile
            // else if (min[1] > (i)) min_y = 8;   // min y is below tile
            else  min_y = 0;
        }
        else if (min_y < pixel_start[0]) min_y = 0;
        else if (min_y > (pixel_start[1] + 4)) min_y = 8;
        top = top >> min_y;

        int max_y = (int)line(tr, tl, i);
        if (max_y == -1)
        {
            // max_y = tl[1] - pixel_start[1]; // y value from pixel start
            if (max[1] < 0) max_y = 8;        // max y is above tile
            else if ((i) > max[1]) max_y = 8;   // max y is below tile
            else max_y = 0;
        }
        else if (max_y < pixel_start[0]) max_y = 8;
        else if (max_y > (pixel_start[0] + 4)) max_y = 0;
        bottom = bottom << max_y;

        uint32_t c = (left&right&top&bottom)|(default_mask << 8);
        temp.mask &= (c);

        temp.mask = (temp.mask << (i * 8)) | (temp.mask >> (32 - (i * 8)));
    }

    if (temp.mask&default_mask != 0x0)
    {
        float depth = fmax(min[2], max[2]);
        if (depth > 0 && depth < mask.depth_1 && depth < mask.depth_0) 
        {
            mask.depth_1 = depth;
            mask.mask = default_mask;
            mask.mask &= temp.mask;        
        }
    }
    
}

int main(void) 
{
    // int s = 1920*1080;
    // // Mask* m l[1920*1080];
    // printf("Size: %td\n", sizeof(Mask) * (1920 * 1080));
    // printf("Size in kilo: %td\n", (sizeof(Mask) * (1920 * 1080)) / 1024);

    Camera::Camera cam;
    printf("Camera is located at (%f, %f, %f)\n", cam.location.x, cam.location.y, cam.location.z);
    glm::vec3 forw = Camera::GetForwardVector(&cam);
    printf("Camera forward vector at (%f, %f, %f)\n", forw.x, forw.y, forw.z);

    // compute screen coordinates
    float t, b, l, r;

    computeScreenCoordinates(
        filmApertureWidth, filmApertureHeight,
        imageWidth, imageHeight,
        kOverscan,
        nearClippingPLane,
        focalLength,
        t, b, l, r); 

    // x = 0, y = 1, z = 0 ext .5
    printf("In front of camera test\n");
    float center[3] = {1.0f, 0.0f, 0.0f};
    float ext[3] = {0.5f, 0.5f, 0.5f};
    AABB_3D aabb = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);
    print_aabb(aabb);

    glm::mat4 view = Camera::GetViewTransform(&cam);
    glm::mat4 proj = Camera::GetProjectionTransform(&cam);

    glm::vec4 view_space_min = view * glm::vec4(aabb.min, 1.0f);
    printf("U view space min (%f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z);

    // view_space_min /= view_space_min.w; 
    // printf("N view space min (%f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z);

    glm::vec4 clip_space_min = proj * view_space_min;
    printf("U proj space min (%f, %f, %f)\n", clip_space_min.x, clip_space_min.y, clip_space_min.z);

    glm::vec3 ret = WorldToRaster(cam, aabb.min, l, r, t, b);
    printf("Raster space min (%f, %f, %f)\n", ret.x, ret.y, ret.z);

    printf("\n\n");
 
    printf("Behind camera test\n");
    center[0] = -1.0f;
    center[1] = 0.0f;
    center[2] = 0.0f;
    ext[0] = 0.5f;
    ext[1] = 0.5f;
    ext[2] = 0.5f;
    aabb = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);
    print_aabb(aabb);

    view_space_min = view * glm::vec4(aabb.min, 1.0f);
    printf("U view space min (%f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z);

    clip_space_min = proj * view_space_min;
    printf("U proj space min (%f, %f, %f)\n", clip_space_min.x, clip_space_min.y, clip_space_min.z);

    ret = WorldToRaster(cam, aabb.min, l, r, t, b);
    printf("Raster space min (%f, %f, %f)\n", ret.x, ret.y, ret.z);

    printf("\n\n");

    printf("Far right test\n");
    center[0] = 1.0f;
    center[1] = 5.0f;
    center[2] = 0.0f;
    ext[0] = 0.5f;
    ext[1] = 0.5f;
    ext[2] = 0.5f;
    aabb = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);
    print_aabb(aabb);

    view_space_min = glm::vec4(0.0f);
    clip_space_min = glm::vec4(0.0f);

    view_space_min = view * glm::vec4(aabb.min, 1.0f);
    printf("U view space min (%f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z);

    clip_space_min = proj * view_space_min;
    printf("U proj space min (%f, %f, %f)\n", clip_space_min.x, clip_space_min.y, clip_space_min.z);

    ret = WorldToRaster(cam, aabb.min, l, r, t, b);
    printf("Raster space min (%f, %f, %f)\n", ret.x, ret.y, ret.z);

    printf("\n\n");

    printf("Far left test\n");
    center[0] = 1.0f;
    center[1] = -100.0f;
    center[2] = 0.0f;
    ext[0] = 0.5f;
    ext[1] = 0.5f;
    ext[2] = 0.5f;
    aabb = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);
    print_aabb(aabb);

    view_space_min = glm::vec4(0.0f);
    clip_space_min = glm::vec4(0.0f);

    view_space_min = view * glm::vec4(aabb.min, 1.0f);
    printf("U view space min (%f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z);

    clip_space_min = proj * view_space_min;
    printf("U proj space min (%f, %f, %f)\n", clip_space_min.x, clip_space_min.y, clip_space_min.z);

    ret = WorldToRaster(cam, aabb.min, l, r, t, b);
    printf("Raster space min (%f, %f, %f)\n", ret.x, ret.y, ret.z);

    printf("\n\n");

    printf("Depth bounds 999\n");
    center[0] = 999.0f;
    center[1] = 0.0f;
    center[2] = 0.0f;
    ext[0] = 0.5f;
    ext[1] = 0.5f;
    ext[2] = 0.5f;
    aabb = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);
    print_aabb(aabb);

    view_space_min = glm::vec4(0.0f);
    clip_space_min = glm::vec4(0.0f);

    view_space_min = view * glm::vec4(aabb.min, 1.0f);
    printf("U view space min (%f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z);

    clip_space_min = proj * view_space_min;
    printf("U proj space min (%f, %f, %f)\n", clip_space_min.x, clip_space_min.y, clip_space_min.z);

    ret = WorldToRaster(cam, aabb.min, l, r, t, b);
    printf("Raster space min (%f, %f, %f)\n", ret.x, ret.y, ret.z);

    printf("\n\n");

    printf("Depth bounds 1000\n");
    center[0] = 1000.0f;
    center[1] = 0.0f;
    center[2] = 0.0f;
    ext[0] = 0.5f;
    ext[1] = 0.5f;
    ext[2] = 0.5f;
    aabb = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);
    print_aabb(aabb);

    view_space_min = glm::vec4(0.0f);
    clip_space_min = glm::vec4(0.0f);

    view_space_min = view * glm::vec4(aabb.min, 1.0f);
    printf("U view space min (%f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z);

    clip_space_min = proj * view_space_min;
    printf("U proj space min (%f, %f, %f)\n", clip_space_min.x, clip_space_min.y, clip_space_min.z);

    ret = WorldToRaster(cam, aabb.min, l, r, t, b);
    printf("Raster space min (%f, %f, %f)\n", ret.x, ret.y, ret.z);

    printf("\n\n");

    printf("Depth bounds 1001\n");
    center[0] = 1001.0f;
    center[1] = 0.0f;
    center[2] = 0.0f;
    ext[0] = 0.5f;
    ext[1] = 0.5f;
    ext[2] = 0.5f;
    aabb = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);
    print_aabb(aabb);

    view_space_min = glm::vec4(0.0f);
    clip_space_min = glm::vec4(0.0f);

    view_space_min = view * glm::vec4(aabb.min, 1.0f);
    printf("U view space min (%f, %f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z, view_space_min.w);

    clip_space_min = proj * view_space_min;
    printf("U proj space min (%f, %f, %f, %f)\n", clip_space_min.x, clip_space_min.y, clip_space_min.z, view_space_min.w);

    ret = WorldToRaster(cam, aabb.min, l, r, t, b);
    printf("Raster space min (%f, %f, %f)\n", ret.x, ret.y, ret.z);

    printf("\n\n");

    printf("BIT MASKING TESTS\n\n");

    uint32_t test = 0xFFFFFFFF;
    SHOW(uint32_t, test);

    // Create a depth buffer with one tile (maybe just use the size of the screen)
    Mask mask;
    SHOW(uint32_t, mask.mask);
    print_mask(mask.mask);

    printf("\n\n");

    // Test the tile agains a box, pixel coordinates
    glm::vec2 minA = glm::vec2(1.0f, -1.0f);
    glm::vec2 maxA = glm::vec2(3.0f, 3.0f);
    // box_mask_test(minA, maxA, mask, glm::vec2(0));
    print_mask(mask.mask);

    // multiple bit masks
    // 1920 x 1080
    // 8x4 pixels per mask
    // 240 x 270
    //const uint32_t imageWidth = 640;
    //const uint32_t imageHeight = 480; 
    // 80x120
    int mask_row = imageWidth / 8;
    int mask_col = imageHeight / 4;
    Mask* masks = (Mask*)malloc(sizeof(Mask) * mask_row * mask_col);
    for (int i = 0; i < mask_col; ++i)
    {
        for (int j = 0; j < mask_row; ++j)
        {
            masks[i * mask_row + j].mask = default_mask;
            masks[i * mask_row + j].depth_0 = 1000.0f;
            masks[i * mask_row + j].depth_1 = 1000.0f;
        }
    }

    printf("Behind camera test\n");
    center[0] = 15.0f;
    center[1] = 10.0f;
    center[2] = 0.0f;
    ext[0] = 5.0f;
    ext[1] = 5.0f;
    ext[2] = 5.0f;
    aabb = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);
    print_aabb(aabb);

    center[0] = 10.0f;
    center[1] = 5.0f;
    center[2] = 0.0f;
    ext[0] = 5.0f;
    ext[1] = 5.0f;
    ext[2] = 5.0f;
    AABB_3D aabb2 = *Create3DAxisAlignedBoundingBoxFromCenter(center, ext);

    view_space_min = view * glm::vec4(aabb.min, 1.0f);
    printf("U view space min (%f, %f, %f)\n", view_space_min.x, view_space_min.y, view_space_min.z);

    clip_space_min = proj * view_space_min;
    printf("U proj space min (%f, %f, %f)\n", clip_space_min.x, clip_space_min.y, clip_space_min.z);

    glm::vec3 min_raster = WorldToRaster(cam, aabb.min, l, r, t, b);
    glm::vec3 max_raster = WorldToRaster(cam, aabb.max, l, r, t, b);
    printf("min_raster (%f, %f, %f)\n", min_raster.x, min_raster.y, min_raster.z);
    printf("max_raster (%f, %f, %f)\n", max_raster.x, max_raster.y, max_raster.z);

    glm::vec3 min_rasterB = WorldToRaster(cam, aabb2.min, l, r, t, b);
    glm::vec3 max_rasterB = WorldToRaster(cam, aabb2.max, l, r, t, b);

    for (int i = 0; i < mask_col; ++i)
    {
        for (int j = 0; j < mask_row; ++j)
        {
            box_mask_test(glm::vec3(max_rasterB), glm::vec3(min_rasterB), masks[i * mask_row + j], glm::vec2(j*8, i*4));
        }
    }

    for (int i = 0; i < mask_col; ++i)
    {
        for (int j = 0; j < mask_row; ++j)
        {
            box_mask_test(glm::vec3(max_raster), glm::vec3(min_raster), masks[i * mask_row + j], glm::vec2(j*8, i*4));
        }
    }

    

    int m_col = 0;
    int m_row = 0;
    int adj_pixel_x = 0;
    int adj_pixel_y = 0;
    std::ofstream ofs;
    ofs.open( "test.ppm", std::ios::out | std::ios::binary );
    ofs << "P6\n" << imageWidth << " " << imageHeight << "\n255\n";
    for (int i = 0; i < imageHeight; ++i)
    {
        m_col = i / 4;
        adj_pixel_y = i - (m_col * 4);
        for (int j = 0; j < imageWidth; ++j)
        {
            
            m_row = j / 8;
            adj_pixel_x = j - (m_row * 8);

            uint32_t m = masks[m_col * mask_row + m_row].mask;
            // LSR
            m = (m << (adj_pixel_y * 8 + adj_pixel_x))|(m >> (32 - (adj_pixel_y * 8 + adj_pixel_x))); 
            unsigned char t;
            if (m & (1))
            {
                printf("Depth: %d Depth char: %c\n", masks[m_col * mask_row + m_row].depth_1,
                    255 * (unsigned char)((masks[m_col * mask_row + m_row].depth_0 - masks[m_col * mask_row + m_row].depth_1) / masks[m_col * mask_row + m_row].depth_0));
               
                t = 255 * ((masks[m_col * mask_row + m_row].depth_0 - masks[m_col * mask_row + m_row].depth_1 )/ masks[m_col * mask_row + m_row].depth_0);
            }
            else 
            {
                t = 0x00;
            }
            // if (t == 1) ofs << 0 << 0 << 255;
            // else ofs << 255 << 0 << 0;
            ofs << t << t << t;

            // RSR
            m = (m >> (adj_pixel_y * 8 + adj_pixel_x))|(m << (32 - (adj_pixel_y * 8 + adj_pixel_x)));
        }
        // printf("\n");
    }

    // for (int i = 0; i < imageHeight * imageWidth; ++i)
    // {
    //     unsigned char n = 0xFF;
    //     ofs << n << n << n;
    // }
    ofs.close();




    // Add in depth consideration


    // print depth buffer for visuals

    // Tests for multiple depths of different objects



    // Use raster results to guess the start and end row for depth buffer


    
    
    return(0);
}
