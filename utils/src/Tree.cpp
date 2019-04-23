#include "Tree.h"

// TODO(Matt): Should these go in a config file? (maybe even a hotloaded one
// to run speed tests)?
#define TREE_CHILDREN 8
#define TREE_BIN_SIZE 100
//#define QUAD_TREE_BIN_SIZE 4

// #define OCT_TREE_CHILDREN 8
// #define OCT_TREE_BIN_SIZE 20

// Helper Functions for the QuadTree
// static bool  helper_add(QuadTree *qt, int position, Model *model);
// static void  helper_print_quad_tree(Node **tree, int position);
// static void  helper_free_quad_tree(Node **tree, int position);
// static void  Split(QuadTree *qt, int position);
// static Node* CreateNode(float *s_min, float *s_max, size_t per_element_bin_size, Node *parent);

//-------------------------------------------------------------------//
// Helper Functions
//-------------------------------------------------------------------//
OctTree::Node* OctTree::create_node(AABB_3D* aabb, Node *parent)
{
    Node *n = (Node *)malloc(sizeof(Node));
    n->bounding_box = aabb;
    n->parent = parent;
    n->isLeaf = true;
    n->bin = (Bin *)malloc(sizeof(Bin));
    n->bin->count = 0;
    n->bin->model = nullptr;
    
    return n;
}

bool OctTree::helper_add(int position, Model *model)
{
    Node *node = tree[position];
    if (!node->isLeaf) // this node has children, need to go further down the tree
    {
        bool at_least_one_node_intersection_found = false;
        for (int i = 0; i < TREE_CHILDREN; ++i)
        {
            assert((position * TREE_CHILDREN) + (i + 1) < arrlen(tree)); // the child node should be within bounds of the tree
            if (CheckBoundingBoxCollision3D(model->aabb, *tree[(position * TREE_CHILDREN) + (i + 1)]->bounding_box))
            {
                at_least_one_node_intersection_found = true;
                helper_add((position * TREE_CHILDREN) + (i + 1), model);
            }
        }
        
        return at_least_one_node_intersection_found;
    }
    else // we are at a leaf
    {
        assert(node->bin); // the bin should be allocated
        if (node->bin->count + 1 > TREE_BIN_SIZE)
        {
            // split and then re-add to this node, which is not a leaf
            split(position);
            helper_add(position, model);
        }
        else
        {
            arrput(node->bin->model, model);
            ++node->bin->count;
        }
        
        return true;
    }
}

void OctTree::split(int position)
{
    Node *node = tree[position];
    
    AABB_3D *aabb = node->bounding_box;
    glm::vec3   min  = aabb->min;
    glm::vec3   max  = aabb->max;

    // half the distance between min and max
    float ext[3] = {(max[0] - min[0]) / 2, (max[1] - min[1]) / 2, (max[2] - min[2]) / 2};
    
    // Create the AABBs of the children of this node 
    // front top left
    float ftl_min[3] = {min[0],          min[1] + ext[1], min[2]         };
    float ftl_max[3] = {min[0] + ext[0], max[1],          min[2] + ext[2]};
    AABB_3D *ftl = Create3DAxisAlignedBoundingBox(ftl_min, ftl_max);
    
    // front bottom left
    float fbl_min[3] = {min[0],          min[1],          min[2]         };
    float fbl_max[3] = {min[0] + ext[0], min[1] + ext[1], min[2] + ext[2]};
    AABB_3D *fbl = Create3DAxisAlignedBoundingBox(fbl_min, fbl_max);

    // front top right
    float ftr_min[3] = {min[0] + ext[0], min[1] + ext[1], min[0]         };
    float ftr_max[3] = {max[0],          max[1],          min[2] + ext[2]};
    AABB_3D *ftr = Create3DAxisAlignedBoundingBox(ftr_min, ftr_max);

    // front bottom right
    float fbr_min[3] = {min[0] + ext[0], min[1],          min[2]};
    float fbr_max[3] = {max[0],          min[1] + ext[1], min[2] + ext[2]};
    AABB_3D *fbr = Create3DAxisAlignedBoundingBox(fbr_min, fbr_max);

    // back top left
    float btl_min[3] = {min[0],          min[1] + ext[1], min[2] + ext[2]};
    float btl_max[3] = {min[0] + ext[0], max[1],          max[2]};
    AABB_3D *btl = Create3DAxisAlignedBoundingBox(btl_min, btl_max);

    // back bottom left
    float bbl_min[3] = {min[0],          min[1],          min[2] + ext[2]};
    float bbl_max[3] = {min[0] + ext[0], min[1] + ext[1], max[2]};
    AABB_3D *bbl = Create3DAxisAlignedBoundingBox(bbl_min, bbl_max);

    // back top right
    float btr_min[3] = {min[0] + ext[0], min[1] + ext[1], min[2] + ext[2]};
    float btr_max[3] = {max[0],          max[1],          max[2]};
    AABB_3D *btr = Create3DAxisAlignedBoundingBox(btr_min, btr_max);

    // back bottom right
    float bbr_min[3] = {min[0] + ext[0], min[1],          min[2] + ext[2]};
    float bbr_max[3] = {max[0],          min[1] + ext[1], max[2]};
    AABB_3D *bbr = Create3DAxisAlignedBoundingBox(bbr_min, bbr_max);

    // grow if necessary
    if ((position * TREE_CHILDREN) + TREE_CHILDREN >= arrlen(tree))
    {
        arrsetlen(tree, ((u32)arrlen(tree) + 1) * TREE_CHILDREN);
    }
    
    // Create nodes for childre
    size_t len = arrlen(tree);
    u32 access = (u32)(position * TREE_CHILDREN) + 1;
    tree[(u32)(position * TREE_CHILDREN) + 1] = create_node(ftl, node);
    tree[(position * TREE_CHILDREN) + 2] = create_node(fbl, node);
    tree[(position * TREE_CHILDREN) + 3] = create_node(ftr, node);
    tree[(position * TREE_CHILDREN) + 4] = create_node(fbr, node);
    tree[(position * TREE_CHILDREN) + 5] = create_node(btl, node);
    tree[(position * TREE_CHILDREN) + 6] = create_node(bbl, node);
    tree[(position * TREE_CHILDREN) + 7] = create_node(btr, node);
    tree[(position * TREE_CHILDREN) + 8] = create_node(bbr, node);
    
    // Now an internal node
    node->isLeaf = false;
    
    // Re-add the contents of this Bin to the node so they are added to the approppriate child
    for (int i = 0; i < node->bin->count; ++i)
    {
        Model *model = node->bin->model[i];
        helper_add(position, model);
    }
    
    // No longer should have a bin attached to the node
    arrfree(node->bin->model);
    free(node->bin);
    node->bin = nullptr;
}

void OctTree::helper_print_quad_tree(int position)
{
    Node *node = tree[position];
    
    printf("Node %d at level %d.\n", position, position / 4);
    printf("This node has this bounding box.\n");
    printf("  Bounding Box:\n      Minimum: (%f, %f, %f)\n      Maximum: (%f, %f, %f)\n      Center: (%f, %f, %f)\n      Extent: (%f, %f, %f)\n", 
        node->bounding_box->min[0], node->bounding_box->min[1], node->bounding_box->min[2], 
        node->bounding_box->max[0], node->bounding_box->max[1], node->bounding_box->max[2], 
        node->bounding_box->center[0], node->bounding_box->center[1], node->bounding_box->center[2], 
        node->bounding_box->ext[0], node->bounding_box->ext[1], node->bounding_box->ext[2]);
    
    if (!node->parent)
    {
        printf("  This node is the root.\n");
    }
    if (!node->isLeaf)
    {
        printf("  This node has the following children at the indices:\n");
        for (int i = 1; i <= TREE_CHILDREN; ++i) {
            helper_print_quad_tree((position * TREE_CHILDREN) + i);    
        }
        
        // helper_print_quad_tree(tree, (position * TREE_CHILDREN) + 2);
        // helper_print_quad_tree(tree, (position * TREE_CHILDREN) + 3);
        // helper_print_quad_tree(tree, (position * TREE_CHILDREN) + 4);
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
                Model *m = node->bin->model[j];
                printf("    Bounding Box:\n      Minimum: (%f, %f, %f)\n      Maximum: (%f, %f, %f)\n      Center: (%f, %f, %f)\n      Extent: (%f, %f, %f)\n", 
                    m->aabb.min[0], m->aabb.min[1], m->aabb.min[2],
                    m->aabb.max[0], m->aabb.max[1], m->aabb.max[2],
                    m->aabb.center[0], m->aabb.center[1], m->aabb.center[2], 
                    m->aabb.ext[0], m->aabb.ext[1], m->aabb.ext[2]);
                printf("    Data: %d\n", m->val);
            }
            //printf("  ");
        }
    }
    printf("\n");
}

//-------------------------------------------------------------------//
// Main Functions
//-------------------------------------------------------------------//
OctTree::OctTree(float* min, float* max)
{
    // First set defaults
    // size_element_per_bin = 0; // keep? Helps to determine the required space for a bin element
    num_levels = 0; // depth of the tree, default is 0

    // array representation of the tree. Must be set to null to the stb library
    tree = nullptr; 
    arrsetlen(tree, 1);
    tree[0] = create_node(Create3DAxisAlignedBoundingBox(min, max), nullptr);
}

OctTree::~OctTree()
{

}

void OctTree::Shutdown()
{

}

void OctTree::Print()
{
    helper_print_quad_tree(0);
}

bool OctTree::Add(Model* model)
{
    if (tree == nullptr)
        return false;
    else if (!CheckBoundingBoxCollision3D(*tree[0]->bounding_box, model->aabb))
        return false;
    else 
        return helper_add(0, model);
}
