#ifndef MESH_H

/*
- Mesh Struct
  - Contains list of material ids
  - Contains list of texture ids
  -  Contains list of attributes
    - Attributes are grouped by material
    - Contain mat and texture id
    - Contain offset into overall buffer
  - Single vertex, index buffer
    - The attributes know their own offset
*/

struct Attribute
{
    // attrib information
    u32 vertex_count;
    u32 index_count;
    u32 uniform_index;

    // Descriptor Set information
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scl;

    typedef struct AABB aabb;

    // attribute offsets?
};

struct Mesh
{
    u32       material_type;
    u32       *material_ids = nullptr;
    u32       *texture_ids  = nullptr;
    Attribute *attribs      = nullptr;


    // Does this go here or in attribute?
    // Going to have a single buffer, but each attribute gets
    // its own offset into the buffer
    //VkBuffer vertex_buffer;
    //VkBuffer index_buffer;
    //VkDeviceMemory vertex_buffer_memory;
    //VkDeviceMemory index_buffer_memory;
    //size_t attribute_offsets[7];
};

#define MESH_H
#endif