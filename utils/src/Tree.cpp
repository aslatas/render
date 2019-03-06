/*

Material Grouping
Instance Grouping
Frustum Grouping

Pipeline


CreateNewMaterial()
  Contains the Pipeline
  Other material properties
  Model List
CreateModel()
  Geometry data and other required stuff
  Instance List
CreateNewInstance()
  ModelProjection


Material
  Model List
    Instance List (set of matrices)
    

*/

// 

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define QUAD_TREE_CHILDREN 4
#define QUAD_TREE_BIN_SIZE 10

#define OCT_TREE_CHILDREN 8
#define OCT_TREE_BIN_SIZE 20

struct Node; // Pre-declare node because reasons...
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

enum {
  SPACE = 0x00,
  BIN = 0x01
} PARTITION_TYPE;

struct Model {
  // General representation of a model in a scene
  AABB_2D aabb;
  int val; // For now a model is simply an integer type
};

// TODO(Dustin): In the future this will probably change to a differnet data structure
// so that we can 
struct Bin {
  Model* model= nullptr; // list of models in this bin
  uint8_t count = 0;
};

struct Node {
  char* memory_block; // block of memeroy for bins and child pointers

  Node* parent;

  // by default, mode is set to store in the bin
  // Essentially a lookup array to determine if an element in the list of
  // children is a branch or leaf node
  uint8_t partition_type[QUAD_TREE_CHILDREN];
  // Either point to the next Child node, or to a list of Bins
  union {
    Node* node;
    Bin*  bin;
  } child[QUAD_TREE_CHILDREN];

  //Node* space_partitions[QUAD_TREE_CHILDREN];
  Bin* bin[QUAD_TREE_BIN_SIZE];

  // char* memory_block;
  AABB_2D* bounding_box;
};

struct QuadTree {
  size_t size_element_per_bin; // keep? Helps to determine the required space for a bin element
  uint8_t num_levels = 0; // depth of the tree, default is 0
  Node* root; // First node in the list
};

/*
Layout of Node:

pointer to the parent
type 
count elements in the bin



*/

// TODO(Dustin): Change to bit shifting for better efficiency
static bool
CheckAxisPointOverlapp(float min_x, float max_x, float min_y, float max_y) {
  // printf("Minx: %f Maxx: %f Miny: %f Maxy: %f\n", min_x, max_x, min_y, max_y);
  // printf("Bounds collision returns for 1: %d\n", 
  //   (min_x <= min_y && min_y <= max_x && max_x >= max_y && max_y >= min_x) ? 1 : 0 );
  // printf("Bounds collision returns for 2: %d\n", 
  //   (min_x >= min_y && min_y <= max_x && max_x >= max_y && max_y >= min_x) ? 1 : 0 );
  // printf("Bounds collision returns for 3: %d\n", 
  //   (min_x <= min_y && min_y <= max_x && max_x <= max_y && max_y >= min_x) ? 1 : 0 );
  // printf("Bounds collision returns for 4: %d\n", 
  //   (min_x >= min_y && min_y <= max_x && max_x <= max_y && max_y >= min_x) ? 1 : 0 );


  return (min_x <= min_y && min_y <= max_x && max_x >= max_y && max_y >= min_x) || // X--y------y--X
         (min_x >= min_y && min_y <= max_x && max_x >= max_y && max_y >= min_x) || // y--X------y--X
         (min_x <= min_y && min_y <= max_x && max_x <= max_y && max_y >= min_x) || // X--y------X--y
         (min_x >= min_y && min_y <= max_x && max_x <= max_y && max_y >= min_x);   // y--X------X--y
}

// TODO(Dustin): Change to bit shifting for better efficiency
bool CheckBoundingBoxCollision2D(const AABB_2D& boxA, const AABB_2D& boxB) {

  // printf("Bounds collision returns for x axis: %d\n", 
  //   (CheckAxisPointOverlapp(boxA.min[0], boxA.max[0], boxB.min[0], boxB.max[0])) ? 1 : 0 );
  // printf("Bounds collision returns for y axis: %d\n", 
  //   CheckAxisPointOverlapp(boxA.min[1], boxA.max[1], boxB.min[1], boxB.max[1]));

  // printf("Return val 1: %d\n", CheckAxisPointOverlapp(boxA.min[0], boxA.max[0], boxB.min[0], boxB.max[0]));
  // printf("Return val 2: %d\n", CheckAxisPointOverlapp(boxA.min[1], boxA.max[1], boxB.min[1], boxB.max[1]));

  unsigned int bit = 0x00;
  bit = bit | CheckAxisPointOverlapp(boxA.min[0], boxA.max[0], boxB.min[0], boxB.max[0]);
  bit = bit & CheckAxisPointOverlapp(boxA.min[1], boxA.max[1], boxB.min[1], boxB.max[1]);
  return bit > 0;
}

// TODO(Dustin): Change to bit shifting for better efficiency
bool CheckBoundingBoxCollision3D(AABB_3D& boxA, AABB_3D& boxB) {
  return CheckAxisPointOverlapp(boxA.min[0], boxA.max[0], boxB.min[0], boxB.max[0]) &&
         CheckAxisPointOverlapp(boxA.min[1], boxA.max[1], boxB.min[1], boxB.max[1]) &&
         CheckAxisPointOverlapp(boxA.min[2], boxA.max[2], boxB.min[2], boxB.max[2]);
}

static bool
helper_add(Node& node, Model& model) {
  // printf("ERROR: Helper add was called, and should not have been!!\n");

  return false;
}

bool Add(QuadTree& qt, Model& model) {
  // printf("Attempting to add model with bounsds:\nMin: (%f, %f)\nMax: (%f, %f)\n",
    // model.aabb.min[0], model.aabb.min[1], model.aabb.max[0], model.aabb.max[1]);
  // For now, simply add to the root
  Node* root = qt.root;

  AABB_2D aabb = *root->bounding_box;
  // printf("Node min: %f %f\nNode max: %f %f\n", aabb.min[0], aabb.min[1], aabb.max[0], aabb.max[1]);
  if (CheckBoundingBoxCollision2D(aabb, model.aabb)) {
    // printf("Collistion!!!\n");
    return true;
  }
  else {
    return helper_add(*root, model);
  }
}

void Split() {

}

Node* CreateNode(float* s_min, float* s_max, size_t per_element_bin_size) {
  // printf("MIN: %f %f\nMAX: %f %f\n", s_min[0], s_min[1], s_max[0], s_max[1]);

  size_t node_size = sizeof(Node);

  AABB_2D* aabb = (AABB_2D*)malloc(sizeof(AABB_2D));;
  aabb->min[0] = s_min[0];
  aabb->min[1] = s_min[1];
  aabb->max[0] = s_max[0];
  aabb->max[1] = s_max[1];


  // Node Memory Layout:
  //   Pointer to parent Node
  //   Array

  // Array Memory Layout
  //   Set array size: (Bin element size * total number of bins * total number of children) 
  //                    + (size of a Node * number of children)
  //   Array structure:
  //     Child 1
  //     Child 2
  //     ...
  //     Child QUAD_TREE_CHILDREN
  //     Bin 1
  //     Bin 2
  //     ...
  //     Bin QUAD_TREE_CHILDREN * QUAD_TREE_BIN_SIZE
  //   A Child element in the array will either point to a Bin START or to another Node

  // Child memory layout:
  //   AABB
  //   type (uint8_t): Does it point to a bin or does it point to a Node?
  //   pointer (void*): the pointer

  // Bin Element Memory Layout:
  //   This is essentially a model. The specifics can be determined in the future...

  // This memory layout is easily scalable, so adding struct elements shouldn't break anything in terms
  // of allocating memory. Cache coherency should be maintain amongst a Node's children as bins are stored
  // No longer use the idea of a "node" but instead have pointers to arrays that represent the immediate
  // layer below. 

  size_t bin_size = per_element_bin_size * QUAD_TREE_BIN_SIZE;

  assert(sizeof(Node*) == sizeof(Bin*));
  size_t total_array_size = bin_size * QUAD_TREE_CHILDREN + sizeof(Node*) * QUAD_TREE_CHILDREN;

  //
  // printf("Size of element per bin: %lu\n", per_element_bin_size);
  // printf("Size of Bins Total: %lu\n", per_element_bin_size * QUAD_TREE_BIN_SIZE * QUAD_TREE_CHILDREN);
  // printf("Size of Pointers: %lu\n", sizeof(Node*));
  // printf("Size of Pointers Array: %lu\n", sizeof(Node*) * QUAD_TREE_CHILDREN);
  // printf("Size of Memory Block: %lu\n", per_element_bin_size * QUAD_TREE_BIN_SIZE * QUAD_TREE_CHILDREN +
  //                           sizeof(Node*) * QUAD_TREE_CHILDREN);

  Node* n = (Node*)malloc(sizeof(Node));
  n->memory_block = (char*)malloc(total_array_size);
  n->bounding_box = aabb;

  for (int i = 0; i < QUAD_TREE_CHILDREN; ++i) {
    // Set partition type
    n->partition_type[i] = BIN;

    // Set pointer of each child to the correct bin
    n->child[i].bin = (Bin*)(n->memory_block + (bin_size * i));
  }

  // printf("MIN AFTER: %f %f\nMAX AFTER: %f %f\n", 
  //   n->bounding_box->min[0], n->bounding_box->min[1], n->bounding_box->max[0], n->bounding_box->max[1]);


  return n;
}

// Creates a Quad tree given a min and max bounds
// min and max should be a 2 element array
QuadTree* CreateQuadTree(float* min, float* max, size_t element_size_bin) {

  QuadTree* qt = (QuadTree*)malloc(sizeof(QuadTree));
  qt->num_levels = 0;
  qt->size_element_per_bin = element_size_bin;
  qt->root = CreateNode(min, max, element_size_bin);

  return qt;
}

void PrintQuadTree(QuadTree& qt) {

  Node* node = qt.root;

  printf("Node has bounds:\nMin: (%f,%f)\nMax: (%f,%f)\n", node->bounding_box->min[0],
    node->bounding_box->min[1], node->bounding_box->max[0], node->bounding_box->max[1]);

  for (int i = 0; i < QUAD_TREE_CHILDREN; ++i) {
    assert((node->partition_type[i] & BIN) == BIN);

    if ((node->partition_type[i] & BIN) == BIN) {
      printf("Child %d: Bin\n", i + 1);
    }
    else if ((node->partition_type[i] & SPACE) == SPACE) {
      printf("Child %d: Space\n", i + 1);
    }
  }

}

void FreeQuadTree(QuadTree& qt) {

}

// Entry point
int main(void) {

  float min[2] = {0, 0};
  float max[2] = {10, 10};

  // printf("MIN BEFORE: %f %f\nMAX BEFORE: %f %f\n", min[0], min[1], max[0], max[1]);

  QuadTree* qt = CreateQuadTree(min, max, sizeof(int));

  Model modelA;
  modelA.aabb.min[0] = 0;
  modelA.aabb.min[1] = 0;
  modelA.aabb.max[0] = 1;
  modelA.aabb.max[1] = 1;
  modelA.val = 10;

  Model modelB;
  modelB.aabb.min[0] = 11;
  modelB.aabb.min[1] = 11;
  modelB.aabb.max[0] = 12;
  modelB.aabb.max[1] = 12;
  modelB.val = 20;

  assert(Add(*qt, modelA));
  assert(!Add(*qt, modelB));

  PrintQuadTree(*qt);

  return(0);
}