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
#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>

#define QUAD_TREE_CHILDREN 4
#define QUAD_TREE_BIN_SIZE 4

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
  Model* model= NULL; // list of models in this bin
  uint8_t count = 0;
};

struct Node {
  Node *parent   = NULL;
  Bin  *bin      = NULL;
  Node *children = NULL;

  // Bounding Box for this node
  AABB_2D* bounding_box;
};

struct QuadTree {
  size_t size_element_per_bin; // keep? Helps to determine the required space for a bin element
  uint8_t num_levels = 0; // depth of the tree, default is 0
  Node** tree = NULL; // array representation of the tree
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

void Split() {

}

static bool
helper_add(Node** tree, int position, Model* model) {

    Node* node = tree[position];
    if (node->children) // this node has children, need to go further down the tree
    {
        bool at_least_one_node_intersection_found = false;
        for (int i = 0; i < arrlen(node->children); ++i)
        {
            if (CheckBoundingBoxCollision2D(*node->children[i].bounding_box, model->aabb))
            {
                at_least_one_node_intersection_found = true;
                assert((position + i) * QUAD_TREE_CHILDREN < arrlen(tree)); // the child node should be within bounds of the tree
                helper_add(tree, (position + i) * QUAD_TREE_CHILDREN, model);
            }
        }

        return at_least_one_node_intersection_found;
    }
    else // we are at a leaf
    {
        assert(node->bin); // the bin should be allocated
        if (node->bin->count + 1 > QUAD_TREE_BIN_SIZE)
        {
            Split();
        }
        else 
        {
            arrput(node->bin->model, *model);
            ++node->bin->count;
        }

        return true;
    }
}

bool Add(QuadTree& qt, Model* model) {

    // Preliminary check to verify the model exists within bounds of the tree
    if (!CheckBoundingBoxCollision2D(*qt.tree[0]->bounding_box, model->aabb)) 
        return false;    

    return helper_add(qt.tree, 0, model);
}

Node* CreateNode(float* s_min, float* s_max, size_t per_element_bin_size, Node* parent) {
  // printf("MIN: %f %f\nMAX: %f %f\n", s_min[0], s_min[1], s_max[0], s_max[1]);

  size_t node_size = sizeof(Node);

  AABB_2D* aabb = (AABB_2D*)malloc(sizeof(AABB_2D));;
  aabb->min[0] = s_min[0];
  aabb->min[1] = s_min[1];
  aabb->max[0] = s_max[0];
  aabb->max[1] = s_max[1];

  Node *n = (Node*)malloc(sizeof(Node));
  n->bounding_box = aabb;
  n->parent       = parent;
  n->children     = nullptr;
  n->bin          = (Bin*)malloc(sizeof(Bin));

  // By default a node will have bins instead of children
//   for (int i = 0; i < QUAD_TREE_BIN_SIZE; ++i)
//   {
//     Bin b;
//     b.count = 0;
//     b.model = nullptr;
//     arrput(n->bin, b);
//   }

  return n;
}

// Creates a Quad tree given a min and max bounds
// min and max should be a 2 element array
QuadTree* CreateQuadTree(float* min, float* max, size_t element_size_bin) 
{
  QuadTree* qt = (QuadTree*)malloc(sizeof(QuadTree));
  qt->num_levels = 0;
  qt->size_element_per_bin = element_size_bin;
  qt->tree = nullptr;
  Node* node = CreateNode(min, max, element_size_bin, nullptr);
  arrput(qt->tree, node);

  return qt;
}

void PrintQuadTree(QuadTree& qt) {

  for (int i = 0; i < arrlen(qt.tree); ++i) 
  {
    Node *node = qt.tree[i];

    printf("Node %d at level %d.\n", i, i / 4);
    if (!node->parent)
    {
      printf("  This node is the root.\n");
    }
    if (node->children)
    {
      printf("  This node has the following children at the indices:\n");
      printf("    Child 1: %d\n", (i + 0) * 4);
      printf("    Child 2: %d\n", (i + 1) * 4);
      printf("    Child 3: %d\n", (i + 2) * 4);
      printf("    Child 4: %d\n", (i + 3) * 4);
    }
    else
    {
      printf("  This node has no children.\n");
    }

    if (!node->bin) 
    {
      printf("  This node does not have a bin.\n");
    }
    else 
    {
      if (node->bin->count == 0)
      {
        printf("  This node has a bin, but contains no models.\n");
      }
      else 
      {
        for (int j = 0; j < node->bin->count; ++j)
        {
          Model* m = node->bin->model;
          printf("  This node's bin contains:\n");
          printf("    Bounding Box:\n      minimum: (%f, %f)\n      maximum: (%f, %f)\n", m->aabb.min[0], m->aabb.min[1], m->aabb.max[0], m->aabb.max[1]);
          printf("    Data: %d\n", m->val);
        }
        //printf("  ");
      }
    }

    printf("\n");
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

  Model modelC;
  modelC.aabb.min[0] = 6;
  modelC.aabb.min[1] = 0;
  modelC.aabb.max[0] = 1;
  modelC.aabb.max[1] = 7;
  modelC.val = 30;

  Model modelD;
  modelD.aabb.min[0] = 0;
  modelD.aabb.min[1] = 6;
  modelD.aabb.max[0] = 1;
  modelD.aabb.max[1] = 7;
  modelD.val = 40;

  Model modelE;
  modelE.aabb.min[0] = 6;
  modelE.aabb.min[1] = 6;
  modelE.aabb.max[0] = 7;
  modelE.aabb.max[1] = 7;
  modelE.val = 50;

//   Model modelF;
//   modelF.aabb.min[0] = 11;
//   modelF.aabb.min[1] = 11;
//   modelF.aabb.max[0] = 12;
//   modelF.aabb.max[1] = 12;
//   modelF.val = 60;

  assert(Add(*qt, &modelA));
  assert(!Add(*qt, &modelB));
  assert(Add(*qt, &modelC));
  assert(Add(*qt, &modelD));
  assert(Add(*qt, &modelE));
//   assert(Add(*qt, &modelF));

  PrintQuadTree(*qt);

  return(0);
}