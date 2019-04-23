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
#define STB_DS_IMPLEMENTATION
#define STBDS_SIPHASH_2_4
#include <stb/stb_ds.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#pragma warning(push, 0)
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/glm.hpp"
#pragma warning(pop)

// Some convientent defines
#define u32 unsigned int 

// Header Files
#include "Bounds.h"
#include "Object.h"
#include "Tree.h"
#include "SceneManager.h"

// SRC Files
#include "Bounds.cpp"
#include "Tree.cpp"
#include "Object.cpp"
#include "SceneManager.cpp"

/**
TODOS
  Increase Bin size as depth increases?

*/

int main(void) 
{
    SceneManager* sm = new SceneManager();

    float min[3] = {0, 5, 0};
    float max[3] = {5, 10, 5};

    // Create the heirarchy and load the octtree
    min[0] = 0;
    min[1] = 0;
    min[2] = 0;
    max[0] = 100;
    max[1] = 100;
    max[2] = 100;
    sm->CreateSpatialHeirarchy(min, max);

    for (auto i = 0.0f; i < 100.0f; i += 1.f) {
        min[0] = i;
        max[0] = i+.9f;
        for (auto j = 0.0f; j < 100.0f; j += 1.f) {
            min[1] = j;
            max[1] = j+.9f;
            for (auto k = 0.0f; k < 100.0f; k += 1.f) {
                min[2] = k;
                max[2] = k+.9f;
                sm->AddModel(min, max, -1, i+j+k);
            }
        }
    }

    sm->LoadOctTree();
    sm->PrintScene();


    // stbds_rand_seed(174769);
    //char* test = (char*)malloc(6 * sizeof(char));
    //char* o = "rtyhg";
    //strncpy(test, o, 5);
    //test[5] = '\0';

    //std::string *str = new std::string("this is another test");

    
    // printf("Model was loaded at index %zd\n", sm->LoadModel("randomfilename", 0));
    // printf("Model was loaded at index %zd\n", sm->LoadModel("yoyoyoyo", 2));
    // printf("Model was loaded at index %zd\n", sm->LoadModel("tee", 1));

    // printf("Model was loaded at index %zd\n", sm->LoadModel("filename", 2));
    // printf("Model was loaded at index %zd\n", sm->LoadModel("name", 3));
    // printf("Model was loaded at index %zd\n", sm->LoadModel("file", 4));

    // printf("\n");
    // printf("\n");
    // printf("\n");

    // printf("Model is string key %s hash the index %zd\n", "file", sm->GetModelIndex("file"));
    // printf("Model is string key %s hash the index %zd\n", "yoyoyoyo", sm->GetModelIndex("yoyoyoyo"));
    // printf("Model is string key %s hash the index %zd\n", "randomfilename", sm->GetModelIndex("randomfilename"));
    // printf("Model is string key %s hash the index %zd\n", "name", sm->GetModelIndex("name"));
    // printf("Model is string key %s hash the index %zd\n", "tee", sm->GetModelIndex("tee"));
    // printf("Model is string key %s hash the index %zd\n", "filename", sm->GetModelIndex("filename"));

    // printf("\n");
    // printf("\n");
    // printf("\n");

  
    //sm->PrintModelTable();

    sm->Shutdown();
    //delete sm;

    //free(test);
    //delete str;

    return(0);
}
