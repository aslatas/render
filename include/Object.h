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

struct ModelData {
    void*    memory_block;
    size_t   memory_block_size;
    
    u32* indices;
    glm::vec3* position; // 12
    glm::vec3* normal;   // 24
    glm::vec4* tangent;  // 40
    glm::vec4* color;    // 56
    glm::vec2* uv0;      // 64
    glm::vec2* uv1;      // 72
    glm::vec2* uv2;      // 80
    
    size_t attribute_offsets[7];
    
    size_t model_index;
};

// Model Taken from engine. Actual Data is commented out as it requires other parts of the engine I do not want
/// to include right now
struct Model
{
    u32 vertex_count;
    u32 index_count;
    u32 uniform_index;
    // AxisAlignedBoundingBox bounds;
    bool hit_test_enabled;
    
    u32 material_type; // index into material layout in SceneManager
    u32 shader_id;     // index into material list in a the above material layout
    u32 model_data_index; // index to find the model data in the SceneManager
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scl;
    
    // Heap allocated:
    ModelData* model_data; // separate data format 
    AABB_3D bounds;
    // u32 material_index;
    // u32 hash_index;
};

struct ModelInstanced
{
    u32 vertex_count;
    u32 index_count;
    u32 instance_count;
    // TODO(Matt): We should probably allow hit testing of instances too - keep track of an AABB per instance?
    u32 material_type; // index into material layout in SceneManager
    u32 shader_id;     // index into material list in a the above material layout
    //u32 model_data_index; // index to find the model data in the SceneManager
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    
    // Heap allocated:
    void* memory;
    size_t memory_size;
    size_t attribute_offsets[4];
    
    u32* indices;
    
    // TODO(Matt): This only supports primitive rendering. We should be able to draw instances of other vertex
    // layouts too.
    glm::vec3* locations;
    glm::vec4* colors;
    glm::vec4* user_data;
    glm::mat4* transforms;
};

struct LoadedModel {
    u32 num_models = 0;
    Model* models = nullptr;
    ModelData* model_data = nullptr;
};

// Temporary model used for the Quad tree. Values are currently harcoded
// into the tree, but this will change in later iterations.
// struct Model
// {
//     // General representation of a model in a scene
//     AABB_3D aabb;
//     int val; // For now a model is simply an integer type
//     size_t material_index;
//     size_t hash_index;
// };

// struct Temp {
//     int material_index;
// };

// Partial Representaion of a Model. This is what is inserted into the OctTree
// model_index is the index into the HashTable for the full Model
// aabb: Axis-Aligned Bounding Box for the Model
// model_index: index into a HashTable that stores the model information
// material_index: index into a HashTable that stores the material information
struct SpatialModel
{
    AABB_3D aabb;
    u32 model_index;
    u32 material_type_idx; // index into material type list
    u32 material_idx;     // index into material list
    bool isVisible;
    bool lastFrameVisible = false;
    uint64_t query_idx = -1;
};

// struct Material
// {
//     // Material Data for Pipeline -> Not included in this "demo"

//     // List of mod index that use this material
//     Model* model_list = nullptr;

// };

#define OBJECT_H
#endif
