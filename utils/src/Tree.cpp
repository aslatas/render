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
#include <exception>

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

/*
Layout of Node:

pointer to the parent
type 
count elements in the bin



*/

static bool
helper_add(Node** tree, int position, Model* model);
Node* CreateNode(float* s_min, float* s_max, size_t per_element_bin_size, Node* parent);

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

Node** Split(Node** tree, int position) {
    Node* node = tree[position];

    AABB_2D* aabb = node->bounding_box;
    float* min = aabb->min;
    float* max = aabb->max;

    float tl_min[2] = {0, max[1] / 2};
    float tl_max[2] = {max[0] / 2, max[1]}; 

    float tr_min[2] = {max[0] / 2, max[1] / 2};
    float tr_max[2] = {max[0], max[1]}; 

    float bl_min[2] = {0, 0};
    float bl_max[2] = {max[0] / 2, max[1] / 2}; 

    float br_min[2] = {max[0] / 2, 0};
    float br_max[2] = {max[0], max[1] / 2};

    // Resize the tree if necessary
    int len = arrlen(tree);
    int o = (position * QUAD_TREE_CHILDREN) + 4;
    if ( (position * QUAD_TREE_CHILDREN) + 4 >= arrlen(tree))
    {
        Node** temp = nullptr;
        arrsetlen(temp, (arrlen(tree) + 1) * 4);
        for (int i = 0; i < arrlen(tree); ++i) 
        {
            temp[i] = tree[i];
            // memcpy(temp[i], tree[i], sizeof(temp[i]));
        }
        arrfree(tree);

        tree = temp;
        temp = nullptr;
    }

    tree[(position * QUAD_TREE_CHILDREN) + 1] = CreateNode(tl_min, tl_max, sizeof(int), node); 
    tree[(position * QUAD_TREE_CHILDREN) + 2] = CreateNode(tr_min, tr_max, sizeof(int), node); 
    tree[(position * QUAD_TREE_CHILDREN) + 3] = CreateNode(bl_min, bl_max, sizeof(int), node); 
    tree[(position * QUAD_TREE_CHILDREN) + 4] = CreateNode(br_min, br_max, sizeof(int), node);

    node->isLeaf = false;

    for (int i = 0; i < node->bin->count; ++i) 
    {
        Model* model = node->bin->model[i];
        helper_add(tree, position, model);
    }

    // No longer should have a bin attached to the node
    arrfree(node->bin->model);
    free(node->bin);
    node->bin = nullptr;

    return tree;
}

static bool
helper_add(Node** tree, int position, Model* model) {

    Node* node = tree[position];
    if (!node->isLeaf) // this node has children, need to go further down the tree
    {
        bool at_least_one_node_intersection_found = false;
        for (int i = 0; i < QUAD_TREE_CHILDREN; ++i)
        {
            int l = arrlen(tree);
            assert((position * QUAD_TREE_CHILDREN) + (i + 1) < arrlen(tree)); // the child node should be within bounds of the tree
            if (CheckBoundingBoxCollision2D(*tree[(position * QUAD_TREE_CHILDREN) + (i + 1)]->bounding_box, model->aabb))
            {
                at_least_one_node_intersection_found = true;
                helper_add(tree, (position * QUAD_TREE_CHILDREN) + (i + 1), model);
            }
        }

        return at_least_one_node_intersection_found;
    }
    else // we are at a leaf
    {
        assert(node->bin); // the bin should be allocated
        if (node->bin->count + 1 > QUAD_TREE_BIN_SIZE)
        {
            tree = Split(tree, position);

            // Verify the split was successful
            assert(!tree[position]->isLeaf);
            // assert(!tree[position]->bin);

            for (int i = 0; i < QUAD_TREE_CHILDREN; ++i)
            {
                assert((position * QUAD_TREE_CHILDREN) + i < arrlen(tree)); // the child node should be within bounds of the tree
                if (CheckBoundingBoxCollision2D(*tree[(position * QUAD_TREE_CHILDREN) + i]->bounding_box, model->aabb))
                {
                    helper_add(tree, (position * QUAD_TREE_CHILDREN) + i, model);
                }
            }
        }
        else 
        {
            arrput(node->bin->model, model);
            ++node->bin->count;
        }

        return true;
    }
}

bool Add(QuadTree* qt, Model* model) {

    // Preliminary check to verify the model exists within bounds of the tree
    if (!CheckBoundingBoxCollision2D(*qt->tree[0]->bounding_box, model->aabb)) 
        return false;    

    return helper_add(qt->tree, 0, model);
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
  n->isLeaf       = true;
  n->bin          = (Bin*)malloc(sizeof(Bin));
  n->bin->count   = 0;
  n->bin->model   = nullptr;

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

  arrsetlen(qt->tree, 1);
  qt->tree[0] = CreateNode(min, max, element_size_bin, nullptr);

  return qt;
}

static void
helper_print_quad_tree(Node** tree, int position) 
{
    Node *node = tree[position];


    printf("Node %d at level %d.\n", position, position / 4);
    if (!node->parent)
    {
      printf("  This node is the root.\n");
    }
    if (!node->isLeaf)
    {
        printf("  This node has the following children at the indices:\n");
        helper_print_quad_tree(tree, (position * QUAD_TREE_CHILDREN) + 1);
        helper_print_quad_tree(tree, (position * QUAD_TREE_CHILDREN) + 2);
        helper_print_quad_tree(tree, (position * QUAD_TREE_CHILDREN) + 3);
        helper_print_quad_tree(tree, (position * QUAD_TREE_CHILDREN) + 4);
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
            Model* m = node->bin->model[j];
            printf("  This node's bin contains:\n");
            printf("    Bounding Box:\n      minimum: (%f, %f)\n      maximum: (%f, %f)\n", m->aabb.min[0], m->aabb.min[1], m->aabb.max[0], m->aabb.max[1]);
            printf("    Data: %d\n", m->val);
        }
        //printf("  ");
      }
    }
}

void PrintQuadTree(QuadTree& qt) 
{
  helper_print_quad_tree(qt.tree, 0);
}

static void
helper_free_quad_tree(Node** tree, int position)
{
    if (!tree[position]->isLeaf)
    {
        for (int i = 1; i <= QUAD_TREE_CHILDREN; ++i)
        {
            helper_free_quad_tree(tree, (position * QUAD_TREE_CHILDREN) + i);
        }
    }
    else
    {       
        arrfree(tree[position]->bin->model);
    }

    free(tree[position]->bounding_box);
    free(tree[position]);
}

void FreeQuadTree(QuadTree* qt)
{
    helper_free_quad_tree(qt->tree, 0);
    arrfree(qt->tree);
    free(qt);
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

  Model modelF;
  modelF.aabb.min[0] = 4;
  modelF.aabb.min[1] = 4;
  modelF.aabb.max[0] = 6;
  modelF.aabb.max[1] = 6;
  modelF.val = 60;

  assert(Add(qt, &modelA));
  assert(!Add(qt, &modelB));
  assert(Add(qt, &modelC));
  assert(Add(qt, &modelD));
  assert(Add(qt, &modelE));
  assert(Add(qt, &modelF));

  PrintQuadTree(*qt);

  FreeQuadTree(qt);

  return(0);
}