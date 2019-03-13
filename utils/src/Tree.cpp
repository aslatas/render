

#include <stdio.h>
#include <assert.h>
#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>

#include "Tree.h"

#define QUAD_TREE_CHILDREN 4
#define QUAD_TREE_BIN_SIZE 4

#define OCT_TREE_CHILDREN 8
#define OCT_TREE_BIN_SIZE 20

static bool helper_add(QuadTree *qt, int position, Model* model);
Node* CreateNode(float* s_min, float* s_max, size_t per_element_bin_size, Node* parent);

// TODO(Dustin): Change to bit shifting for better efficiency
static bool
CheckAxisPointOverlapp(float min_x, float max_x, float min_y, float max_y) {
  return (min_x <= min_y && min_y <= max_x && max_x >= max_y && max_y >= min_x) || // X--y------y--X
         (min_x >= min_y && min_y <= max_x && max_x >= max_y && max_y >= min_x) || // y--X------y--X
         (min_x <= min_y && min_y <= max_x && max_x <= max_y && max_y >= min_x) || // X--y------X--y
         (min_x >= min_y && min_y <= max_x && max_x <= max_y && max_y >= min_x);   // y--X------X--y
}

bool CheckBoundingBoxCollision2D(AABB_2D& boxA, AABB_2D& boxB) {
    return CheckAxisPointOverlapp(boxA.min[0], boxA.max[0], boxB.min[0], boxB.max[0]) &&
           CheckAxisPointOverlapp(boxA.min[1], boxA.max[1], boxB.min[1], boxB.max[1]);
}

bool CheckBoundingBoxCollision3D(AABB_3D& boxA, AABB_3D& boxB) {
  return CheckAxisPointOverlapp(boxA.min[0], boxA.max[0], boxB.min[0], boxB.max[0]) &&
         CheckAxisPointOverlapp(boxA.min[1], boxA.max[1], boxB.min[1], boxB.max[1]) &&
         CheckAxisPointOverlapp(boxA.min[2], boxA.max[2], boxB.min[2], boxB.max[2]);
}

static void 
Split(QuadTree* qt, int position) {
    Node* node = qt->tree[position];

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

    if ( (position * QUAD_TREE_CHILDREN) + 4 >= arrlen(qt->tree))
    {
        arrsetlen(qt->tree, (arrlen(qt->tree) + 1) * 4);
    }

    qt->tree[(position * QUAD_TREE_CHILDREN) + 1] = CreateNode(tl_min, tl_max, sizeof(int), node); 
    qt->tree[(position * QUAD_TREE_CHILDREN) + 2] = CreateNode(tr_min, tr_max, sizeof(int), node); 
    qt->tree[(position * QUAD_TREE_CHILDREN) + 3] = CreateNode(bl_min, bl_max, sizeof(int), node); 
    qt->tree[(position * QUAD_TREE_CHILDREN) + 4] = CreateNode(br_min, br_max, sizeof(int), node);

    node->isLeaf = false;

    for (int i = 0; i < node->bin->count; ++i) 
    {
        Model* model = node->bin->model[i];
        helper_add(qt, position, model);
    }

    // No longer should have a bin attached to the node
    arrfree(node->bin->model);
    free(node->bin);
    node->bin = nullptr;
}

static bool
helper_add(QuadTree* qt, int position, Model* model) {

    Node* node = qt->tree[position];
    if (!node->isLeaf) // this node has children, need to go further down the tree
    {
        bool at_least_one_node_intersection_found = false;
        for (int i = 0; i < QUAD_TREE_CHILDREN; ++i)
        {
            int l = arrlen(qt->tree);
            assert((position * QUAD_TREE_CHILDREN) + (i + 1) < arrlen(qt->tree)); // the child node should be within bounds of the tree
            if (CheckBoundingBoxCollision2D(*qt->tree[(position * QUAD_TREE_CHILDREN) + (i + 1)]->bounding_box, model->aabb))
            {
                at_least_one_node_intersection_found = true;
                helper_add(qt, (position * QUAD_TREE_CHILDREN) + (i + 1), model);
            }
        }

        return at_least_one_node_intersection_found;
    }
    else // we are at a leaf
    {
        assert(node->bin); // the bin should be allocated
        if (node->bin->count + 1 > QUAD_TREE_BIN_SIZE)
        {
            Split(qt, position);

            // Verify the split was successful
            assert(!qt->tree[position]->isLeaf);

            for (int i = 0; i < QUAD_TREE_CHILDREN; ++i)
            {
                assert((position * QUAD_TREE_CHILDREN) + (i + 1) < arrlen(qt->tree)); // the child node should be within bounds of the tree
                if (CheckBoundingBoxCollision2D(*qt->tree[(position * QUAD_TREE_CHILDREN) + i]->bounding_box, model->aabb))
                {
                    int p = (position * QUAD_TREE_CHILDREN) + (i + 1);
                    helper_add(qt, p, model);
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

    return helper_add(qt, 0, model);
}

static Node* 
CreateNode(float* s_min, float* s_max, size_t per_element_bin_size, Node* parent) {

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
    printf("This node has this bounding box.\n");
    printf("  Bounding Box:\n      minimum: (%f, %f)\n      maximum: (%f, %f)\n", node->bounding_box->min[0], node->bounding_box->min[1], node->bounding_box->max[0], node->bounding_box->max[1]);
    
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
          printf("  This node's bin contains %d models:\n", node->bin->count);
        for (int j = 0; j < node->bin->count; ++j)
        {
            Model* m = node->bin->model[j];
            printf("    Bounding Box:\n      minimum: (%f, %f)\n      maximum: (%f, %f)\n", m->aabb.min[0], m->aabb.min[1], m->aabb.max[0], m->aabb.max[1]);
            printf("    Data: %d\n", m->val);
        }
        //printf("  ");
      }
    }
    printf("\n");
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

void TestBoundingBoxCollisions()
{

    AABB_2D boundsA;
    boundsA.min[0] = 0;
    boundsA.min[1] = 0;
    boundsA.max[0] = 10;
    boundsA.max[1] = 10;

    AABB_2D boundsB; // tl
	boundsB.min[0] = 0;
	boundsB.min[1] = 5;
	boundsB.max[0] = 5;
	boundsB.max[1] = 10;

    AABB_2D boundsC; // tr
	boundsC.min[0] = 5;
	boundsC.min[1] = 5;
	boundsC.max[0] = 10;
	boundsC.max[1] = 10;

    AABB_2D boundsD; // bl
	boundsD.min[0] = 0;
	boundsD.min[1] = 0;
	boundsD.max[0] = 5;
	boundsD.max[1] = 5;

    AABB_2D boundsE; // br
    boundsE.min[0] = 5;
    boundsE.min[1] = 0;
    boundsE.max[0] = 10;
    boundsE.max[1] = 5;

    assert(CheckBoundingBoxCollision2D(boundsA, boundsB));
    assert(CheckBoundingBoxCollision2D(boundsA, boundsC));
    assert(CheckBoundingBoxCollision2D(boundsA, boundsD));
    assert(CheckBoundingBoxCollision2D(boundsA, boundsE));

    AABB_2D bbA; // br
    bbA.min[0] = 6;
    bbA.min[1] = 0;
    bbA.max[0] = 7;
    bbA.max[1] = 1;
    assert(CheckBoundingBoxCollision2D(boundsA, bbA)); // full bb
    assert(!CheckBoundingBoxCollision2D(boundsB, bbA)); // tl
    assert(!CheckBoundingBoxCollision2D(boundsC, bbA)); // tr
    assert(!CheckBoundingBoxCollision2D(boundsD, bbA)); // bl
    assert(CheckBoundingBoxCollision2D(boundsE, bbA)); // br <- FAILS

    AABB_2D bbB; // tl
    bbB.min[0] = 0;
    bbB.min[1] = 6;
    bbB.max[0] = 1;
    bbB.max[1] = 7;
    assert(CheckBoundingBoxCollision2D(boundsA, bbB)); // full bb
    assert(CheckBoundingBoxCollision2D(boundsB, bbB)); // tl
    assert(!CheckBoundingBoxCollision2D(boundsC, bbB)); // tr
    assert(!CheckBoundingBoxCollision2D(boundsD, bbB)); // bl
    assert(!CheckBoundingBoxCollision2D(boundsE, bbB)); // br

    AABB_2D bbC;
    bbC.min[0] = 6;
    bbC.min[1] = 6;
    bbC.max[0] = 7;
    bbC.max[1] = 7;
    assert(CheckBoundingBoxCollision2D(boundsA, bbC)); // full bb
    assert(!CheckBoundingBoxCollision2D(boundsB, bbC)); // tl
    assert(CheckBoundingBoxCollision2D(boundsC, bbC)); // tr
    assert(!CheckBoundingBoxCollision2D(boundsD, bbC)); // bl
    assert(!CheckBoundingBoxCollision2D(boundsE, bbC)); // br

    AABB_2D bbD;
    bbC.min[0] = 4;
    bbC.min[1] = 4;
    bbC.max[0] = 6;
    bbC.max[1] = 6;

    // AABB_2D bbE;


    // AABB_2D bbF;


}

// Entry point
int main(void) {

    TestBoundingBoxCollisions();

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
  modelC.aabb.max[0] = 7;
  modelC.aabb.max[1] = 1;
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
  
  // First recursive split
  assert(Add(qt, &modelF));

  // Second recursive split test
  assert(Add(qt, &modelA));
  assert(Add(qt, &modelA));
  assert(Add(qt, &modelA));
  assert(Add(qt, &modelA)); 

  PrintQuadTree(*qt);

  FreeQuadTree(qt);

  return(0);
}