#ifndef TREE_H

// Check bounding box-bounding box collisions. Return true if there was a collision, false otherwise 
// bool CheckBoundingBoxCollision2D(AABB_2D& boxA, AABB_2D& boxB);
// bool CheckBoundingBoxCollision3D(AABB_3D& boxA, AABB_3D& boxB);

class OctTree 
{
protected:
    // Bins contain a list of models.
    //   model: an array of models utilizing the header only library, stb_ds.h.
    //   count: number of models located in the bin.
    struct Bin
    {
        Model** model = nullptr; // list of models in this bin
        uint8_t count = 0;
    };
    // A node in the OctTree.
    //  bounding_box: a 2D bounding box that represents the spatial position of the Node. 
    //  parent: TODO(Dustin): implement. Not currently being used. The type will be changed to int and
    //    will represent the position of the parent in the list. Alternatively, the parent can be found
    //    by (given the position of this node): ((position - 1) / QUAD_TREE_CHILDREN).
    //  Bin: the bin containing a list of models located at this node. The bin will either be allocated or
    //    be set to NULL. When the bin is allocated, the Node will be a leaf node, otherwise the node is a
    //    branch node. At Node creation, the bin is always allocated and assumbed to be a leaf.
    //  isLeaf: a quick check whether or not the Node is a leaf. True if leaf node, false otherwise.
    struct Node
    {
       // Bounding Box for this node
       AABB_3D* bounding_box;
    
        Node *parent   = NULL;
        Bin  *bin      = NULL;
        bool isLeaf    = true;
    };

private:
    // some nifty state
    // size_t size_element_per_bin; // keep? Helps to determine the required space for a bin element
    uint8_t num_levels = 0; // depth of the tree, default is 0
    Node** tree = NULL; // array representation of the tree

    // Private helper functions
    Node* create_node(AABB_3D* aabb, Node *parent);
    bool helper_add(int position, Model *model);
    void helper_print_quad_tree(int position);
    void split(int position);


public:
    OctTree(float* min, float* max);
    ~OctTree();

    void Shutdown();
    void Print();

    bool Add(Model* model);
    

};

// Quad Tree structure
//  size_element_per_bin: size of the struct/elements that go into the Bins. This will eventually allow for the bins to be
//    abstracted from the data  structure, leaving it up to the user to decide how the data interacts with the client.   
//    TODO(Dustin): currently hard-coded. Fix that.
//  num_levels: not currently used, but could be useful for restriciting the maximum number of levels the quad tree can have.
//  tree: an array containing all of the nodes in the tree. If given a position, you can find the location first child Node
//    by: (postion * QUAD_TREE_CHILDREN) + 1. The array is handled by the header only library, stb_ds.h.
// Unimplemented:
//   Render Function: this will be a void* that user sets to control how the tree gets rendered.
//   Hueristic Functions: Hueristics can be applied to a Tree to improve space/time complexities. The currrent plan is to have
//     a set of heuristics the user can define should they desire. These hueristics will either be void* for the user to set from
//     either a custom implementation or one of ours. 
// struct QuadTree
// {
//     size_t size_element_per_bin; // keep? Helps to determine the required space for a bin element
//     uint8_t num_levels = 0; // depth of the tree, default is 0
//     Node** tree = NULL; // array representation of the tree
// };



// // Constructs a quad tree given a minimum and maximum 2D coordinates and the size each model will be.
// //   min: minimum x,y of the quad tree.
// //   max: maximum x,y of the quad tree.
// //   element_size_bin: size of each model that is placed in the bin. Currently not in use.
// QuadTree* CreateQuadTree(float* min, float* max, size_t element_size_bin);

// // Add a Model to the QuadTree.
// //   qt: quad tree to add the model to.
// //   model: model to add.
// //   Returns true if the Add operation was successful. False otherwise.
// bool Add(QuadTree* qt, Model* model);

// // Prints the quad tree passed as the parameter.
// void PrintQuadTree(QuadTree &qt);

// // Free the quad tree passed as the parameter.
// void FreeQuadTree(QuadTree* qt);


#define TREE_H
#endif