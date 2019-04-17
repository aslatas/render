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
  

*/

// C Lib Files
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

// External Lib Files
#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>

// Some convientent defines
#define u32 unsigned int 

// Header Files
#include "Tree.h"
#include "Object.h"
#include "SceneManager.h"

// SRC Files
#include "Tree.cpp"
#include "Object.cpp"
#include "SceneManager.cpp"


int main(void) 
{
  float min[2] = {0, 0};
  float max[2] = {10, 10};

  QuadTree *qt = CreateQuadTree(min, max, sizeof(int));
  FreeQuadTree(qt);


  return(0);
}
