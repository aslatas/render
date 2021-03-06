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

int main(void) 
{
    
    return(0);
}
