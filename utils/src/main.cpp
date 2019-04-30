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

// Header Files
// #include "Bounds.h"
// #include "Object.h"
// #include "Tree.h"
// #include "ModelLoader.h"
// #include "SceneManager.h"

// Config Parser Component headers
// #include "config_parsers/ConfigUtils.h"
// #include "config_parsers/SceneConfig.h"

// SRC Files
// #include "Bounds.cpp"
// #include "Tree.cpp"
// #include "Object.cpp"
// #include "ModelLoader.cpp"
// #include "SceneManager.cpp"

/**
TODOS
  Adjust for the renderer:
     True Model Loading
     True Material Loading
     Create the render array
*/

struct Triangle
{
    glm::vec2 vertA;
    glm::vec2 vertB;
    glm::vec2 vertC;
};

struct Mask
{
    uint32_t mask; // 2D array
    float depth_0;
    float depth_1;
};

/*

*/


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

int main(void) 
{
    int s = 1920*1080;
    // Mask* m l[1920*1080];
    printf("Size: %td\n", sizeof(Mask) * (1920 * 1080));
    printf("Size in kilo: %td\n", (sizeof(Mask) * (1920 * 1080)) / 1024);


    

    return(0);
}
