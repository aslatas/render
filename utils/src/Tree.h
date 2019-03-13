#pragma once

#include <stdlib.h>
#include <stdint.h>

// Bounding region for 2D space
struct AABB_2D {
  float min[2];
  float max[2];
};
// Bounding region for 3D space
struct AABB_3D {
  float min[3];
  float max[3];
};

struct Model {
  // General representation of a model in a scene
  AABB_2D aabb;
  int val; // For now a model is simply an integer type
};

// TODO(Dustin): In the future this will probably change to a differnet data structure
// so that we can 
struct Bin {
  Model** model= NULL; // list of models in this bin
  uint8_t count = 0;
};

struct Node {
  Node *parent   = NULL;
  Bin  *bin      = NULL;
  //Node *children = NULL;

  // Bounding Box for this node
  AABB_2D* bounding_box;
  bool isLeaf = true;
};

struct QuadTree {
  size_t size_element_per_bin; // keep? Helps to determine the required space for a bin element
  uint8_t num_levels = 0; // depth of the tree, default is 0
  Node** tree = NULL; // array representation of the tree
};

bool CheckBoundingBoxCollision2D(AABB_2D& boxA, AABB_2D& boxB);
bool CheckBoundingBoxCollision3D(AABB_3D& boxA, AABB_3D& boxB);

QuadTree* CreateQuadTree(float* min, float* max, size_t element_size_bin);

bool Add(QuadTree* qt, Model* model);

void FreeQuadTree(QuadTree* qt);