#ifndef OBJECT_H

// Don't know if this will clash with real model
// Full representation of a Model. In actual system, this contains references
// to device memory for rendering.
// model_data: pointer to a region of memory that contains a Model's full data
// material_index: index into a HashTable that stores the nexessary index information
// struct Model
// {
//     ModelData* model_data;

//     u32 material_index;

//     // Device memory references
// };
// Bounding region for 2D space
//   min: minimum x,y values located in the model
//   max: maximum x,y values located in the model
struct AABB_2D
{
    float min[2];
    float max[2];
};
// Bounding region for 3D space
//   min: minimum x,y,z values located in the model
//   max: maximum x,y,z values located in the model
struct AABB_3D
{
    float min[3];
    float max[3];
};

// Temporary model used for the Quad tree. Values are currently harcoded
// into the tree, but this will change in later iterations.
struct Model
{
    // General representation of a model in a scene
    AABB_2D aabb;
    int val; // For now a model is simply an integer type
    int material_index;
};

struct Temp {
    int material_index;
};

// Partial Representaion of a Model. This is what is inserted into the OctTree
// model_index is the index into the HashTable for the full Model
// aabb: Axis-Aligned Bounding Box for the Model
// model_index: index into a HashTable that stores the model information
// material_index: index into a HashTable that stores the material information
struct ModelBounds 
{
    // aabb

    u32 model_index;
    u32 material_index;
};

struct ModelData
{
    // Buch of stuff here. This is for Visualization rather than real stuff. 
    // Full implementation can be seen in ModelLoader.cpp
};

struct Material
{
    // Material Data for Pipeline -> Not included in this "demo"

    // List of mod index that use this material
    Model* model_list = nullptr;

};

#define OBJECT_H
#endif
