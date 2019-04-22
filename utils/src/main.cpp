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

static void test_octtree()
{
    float min[3] = {0, 0, 0};
    float max[3] = {10, 10, 10};

    OctTree *ot = new OctTree(min, max);

    Model modelA;
    modelA.aabb.min[0] = 0;
    modelA.aabb.min[1] = 0;
    modelA.aabb.min[2] = 0;
    modelA.aabb.max[0] = 1;
    modelA.aabb.max[1] = 1;
    modelA.aabb.max[2] = 1;
    modelA.val = 10;
    
    Model modelB;
    modelB.aabb.min[0] = 150;
    modelB.aabb.min[1] = 150;
    modelB.aabb.min[2] = 150;
    modelB.aabb.max[0] = 200;
    modelB.aabb.max[1] = 200;
    modelB.aabb.max[2] = 200;
    modelB.val = 20;
    
    Model modelC;
    modelC.aabb.min[0] = 6;
    modelC.aabb.min[1] = 0;
    modelC.aabb.min[2] = 0;
    modelC.aabb.max[0] = 7;
    modelC.aabb.max[1] = 1;
    modelC.aabb.max[2] = 1;
    modelC.val = 30;
    
    Model modelD;
    modelD.aabb.min[0] = 0;
    modelD.aabb.min[1] = 6;
    modelD.aabb.min[2] = 0;
    modelD.aabb.max[0] = 1;
    modelD.aabb.max[1] = 7;
    modelD.aabb.max[2] = 1;
    modelD.val = 40;
    
    Model modelE;
    modelE.aabb.min[0] = 6;
    modelE.aabb.min[1] = 6;
    modelE.aabb.min[2] = 0;
    modelE.aabb.max[0] = 7;
    modelE.aabb.max[1] = 7;
    modelE.aabb.max[2] = 1;
    modelE.val = 50;
    
    Model modelF;
    modelF.aabb.min[0] = 4;
    modelF.aabb.min[1] = 4;
    modelF.aabb.min[2] = 0;
    modelF.aabb.max[0] = 6;
    modelF.aabb.max[1] = 6;
    modelF.aabb.max[2] = 1;
    modelF.val = 60;
    
    assert(ot->Add(&modelA));
    assert(!ot->Add(&modelB));
    assert(ot->Add(&modelC));
    assert(ot->Add(&modelD));
    assert(ot->Add(&modelE));
    
    // First recursive split
    // assert(ot->Add(&modelF));
    
    // // Second recursive split test
    // assert(ot->Add(&modelA));
    // assert(ot->Add(&modelA));
    // assert(ot->Add(&modelA));
    // assert(ot->Add(&modelA));


    ot->Print();

    // FreeQuadTree(qt);
}


int main(void) 
{
    test_octtree();

    stbds_rand_seed(174769);
    //char* test = (char*)malloc(6 * sizeof(char));
    //char* o = "rtyhg";
    //strncpy(test, o, 5);
    //test[5] = '\0';

    //std::string *str = new std::string("this is another test");

    const SceneManager* sm = SceneManager::GetInstance();
    printf("Model was loaded at index %d\n", sm->LoadModel("randomfilename", 0));
    printf("Model was loaded at index %td\n", sm->LoadModel("yoyoyoyo", 2));
    printf("Model was loaded at index %td\n", sm->LoadModel("tee", 1));

    printf("Model was loaded at index %td\n", sm->LoadModel("filename", 2));
    printf("Model was loaded at index %td\n", sm->LoadModel("name", 3));
    printf("Model was loaded at index %td\n", sm->LoadModel("file", 4));

    printf("\n");
    printf("\n");
    printf("\n");

    printf("Model is string key %s hash the index %d\n", "file", sm->GetModelIndex("file"));
    printf("Model is string key %s hash the index %d\n", "yoyoyoyo", sm->GetModelIndex("yoyoyoyo"));
    printf("Model is string key %s hash the index %d\n", "randomfilename", sm->GetModelIndex("randomfilename"));
    printf("Model is string key %s hash the index %d\n", "name", sm->GetModelIndex("name"));
    printf("Model is string key %s hash the index %d\n", "tee", sm->GetModelIndex("tee"));
    printf("Model is string key %s hash the index %d\n", "filename", sm->GetModelIndex("filename"));

    printf("\n");
    printf("\n");
    printf("\n");

  
    //sm->PrintModelTable();

    sm->Shutdown();
    //delete sm;

    //free(test);
    //delete str;

    return(0);
}
