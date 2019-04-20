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

// Some convientent defines
#define u32 unsigned int 

// Header Files
#include "Object.h"
#include "Tree.h"
#include "SceneManager.h"

// SRC Files
#include "Tree.cpp"
#include "Object.cpp"
#include "SceneManager.cpp"

int main(void) 
{
    stbds_rand_seed(174769);
    char* test = (char*)malloc(6 * sizeof(char));
    char* o = "rtyhg";
    strncpy(test, o, 5);
    test[5] = '\0';

    const std::string *str = new std::string("this is another test");

  float min[2] = {0, 0};
  float max[2] = {10, 10};

  QuadTree *qt = CreateQuadTree(min, max, sizeof(int));
  FreeQuadTree(qt);

  SceneManager* sm = new SceneManager();
  // printf("Model was loaded at index %td\n", sm->LoadModel("randomfilename", 0));
  printf("Model was loaded at index %td\n", sm->LoadModel(&str->c_str()[0], 2));
  printf("Model was loaded at index %td\n", sm->LoadModel(test, 1));

  // printf("Model was loaded at index %td\n", sm->LoadModel("filename", 2));
  // printf("Model was loaded at index %td\n", sm->LoadModel("name", 3));
  // printf("Model was loaded at index %td\n", sm->LoadModel("file", 4));

  printf("\n");
  printf("\n");
  printf("\n");


  // printf("Model %s at index: %td\n", "randomfilename", sm->GetModelIndex("randomfilename"));
  // printf("Model %s has key: %s\n", "randomfilename", sm->GetModelStruct("randomfilename").key);

  // printf("Model %s at index: %td\n", test, sm->GetModelIndex(test));
  // printf("Model %s has key: %s\n", test, sm->GetModelStruct(test).key);

  // printf("Model %s at index: %td\n", "filename", sm->GetModelIndex("filename"));
  // printf("Model %s has key: %s\n", "filename", sm->GetModelStruct("filename").key);

  // printf("Model %s at index: %td\n", "name", sm->GetModelIndex("name"));
  // printf("Model %s has key: %s\n", "name", sm->GetModelStruct("name").key);

  // printf("Model %s at index: %td\n", "file", sm->GetModelIndex("file"));
  // printf("Model %s has key: %s\n", "file", sm->GetModelStruct("file").key);

    printf("\n");
  printf("\n");
  printf("\n");

  
  sm->PrintModelTable();

  delete sm;

  return(0);
}
