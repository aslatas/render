#include "GLTF.h"

using namespace glTF;

enum EGLTFExtension
{
    ASCII,
    GLB,
    INVALID
};

// Actual data for information read from a GLTF file 
struct MeshData {
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

static MeshData *CreateMeshData(fx::gltf::Document &gltf_mesh, int node_idx)
{
    size_t index_count = 0;
    size_t vertex_count = 0;
    fx::gltf::Mesh c_mesh = gltf_mesh.meshes[gltf_mesh.nodes[node_idx].mesh];
    for (int i = 0; i < c_mesh.primitives.size(); ++i)
    {
        fx::gltf::Primitive primitive = c_mesh.primitives[i];
        // model.vertex_count += (u32)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[j].attributes.find("POSITION")->second].count;
        vertex_count += (u32)gltf_mesh.accessors[primitive.attributes.find("POSITION")->second].count;

        // model.index_count  += (u32)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[j].indices].count;
        index_count  += (u32)gltf_mesh.accessors[primitive.indices].count;
    }

    MeshData *model_data = (MeshData *)malloc(sizeof(MeshData));;
    // Allocate memory for the model
    {
        size_t indices_length  = index_count * sizeof(u32);
        size_t position_length = vertex_count * sizeof(glm::vec3);
        size_t normal_length   = vertex_count * sizeof(glm::vec3);
        size_t tangent_length  = vertex_count * sizeof(glm::vec4);
        size_t color_length    = vertex_count * sizeof(glm::vec4);
        size_t uv0_length      = vertex_count * sizeof(glm::vec2);
        size_t uv1_length      = vertex_count * sizeof(glm::vec2);
        size_t uv2_length      = vertex_count * sizeof(glm::vec2);
        
        model_data->memory_block_size = indices_length + position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length + uv2_length;
        model_data->memory_block      = (void *)malloc(model_data->memory_block_size);
        
        model_data->indices  = (u32 *)model_data->memory_block;
        model_data->position = (glm::vec3 *)((char *)model_data->memory_block + indices_length);
        model_data->normal   = (glm::vec3 *)((char *)model_data->memory_block + indices_length + position_length);
        model_data->tangent  = (glm::vec4 *)((char *)model_data->memory_block + indices_length + position_length + normal_length);
        model_data->color    = (glm::vec4 *)((char *)model_data->memory_block + indices_length + position_length + normal_length + tangent_length);
        model_data->uv0      = (glm::vec2 *)((char *)model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length);
        model_data->uv1      = (glm::vec2 *)((char *)model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length + uv0_length);
        model_data->uv2      = (glm::vec2 *)((char *)model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length);
        
        model_data->attribute_offsets[0] = 0; // First offset into the VertexBuffer is 0
        model_data->attribute_offsets[1] = position_length;
        model_data->attribute_offsets[2] = position_length + normal_length;
        model_data->attribute_offsets[3] = position_length + normal_length + tangent_length;
        model_data->attribute_offsets[4] = position_length + normal_length + tangent_length + color_length;
        model_data->attribute_offsets[5] = position_length + normal_length + tangent_length + color_length + uv0_length;
        model_data->attribute_offsets[6] = position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length;
    }

    printf("Found %td indices\n", index_count);
    printf("Found %td vertices\n", vertex_count);

    return model_data;
}

Mesh *glTF::LoadMesh(char *filename)
{
    fx::gltf::Document gltf_model;
    try {
        gltf_model = fx::gltf::LoadFromText(filename);
    } 
    catch (std::exception &e)
    {
        printf("ERROR loading gltf file %s\n", filename);
        return nullptr;
    }
    printf("Successfully read file.\n");

    // Need to first figure out the total size of the buffer...
    //  for each node
    //      for each primitive
    //          for each POSITION attribute
    //              increment vert count and index count
    MeshData **data_list = nullptr;
    size_t total_buffer_size = 0;
    int idx = 0;
    for (int i = 0; i < gltf_model.nodes.size(); ++i)
    {
        if (gltf_model.nodes[i].mesh < 0) continue;
        arrput(data_list, CreateMeshData(gltf_model, i));
        total_buffer_size += data_list[idx]->memory_block_size;
        ++idx;
    }

    printf("Required buffer size is %td\n", total_buffer_size);

    // There is a possibility of there being more than one mesh in the list
    // TODO(Dustin): find the count
    // the number of nodes = # of meshes
    Mesh *meshes = nullptr;

    idx = 0;
    for (int i = 0; i < gltf_model.nodes.size(); ++i)
    {
        
        Mesh mesh = {};

        fx::gltf::Node node = gltf_model.nodes[i];
        if (gltf_model.nodes[i].mesh < 0) continue;
        fx::gltf::Mesh m = gltf_model.meshes[node.mesh];

        // A mesh can have multiple attributes and we want to but those
        // attributes into the same buffer. These are the offsets into the
        // overall buffer
        int vertex_offset = 0;
        int index_offset = 0;
        // Using the Mesh data created above, let's get the corresponding data block
        MeshData *mesh_data = data_list[idx];
        printf("At model %d\n", i);
        printf("  This model has %zd primtives\n", m.primitives.size());

        for (int j = 0; j < m.primitives.size(); ++j)
        {
            fx::gltf::Primitive primitive = m.primitives[j];
            if (primitive.indices < 0)
                continue; // no indices were found
            Primitive m_primitive = {};

             // Vertex Attributes
            {
                float *position_buffer = nullptr;
                float *normal_buffer   = nullptr;
                float *tangent_buffer  = nullptr;
                float *color_buffer    = nullptr;
                float *uv0_buffer      = nullptr;
                float *uv1_buffer      = nullptr;
                float *uv2_buffer      = nullptr;

                if (primitive.attributes.find("POSITION") == primitive.attributes.end())
                    return nullptr;
                
                fx::gltf::Accessor &pos_accessor = gltf_model.accessors[primitive.attributes.find("POSITION")->second];
                fx::gltf::BufferView &pos_buffer_view = gltf_model.bufferViews[pos_accessor.bufferView];
                position_buffer = (float *)(&(gltf_model.buffers[pos_buffer_view.buffer].data[pos_accessor.byteOffset + pos_buffer_view.byteOffset]));
                // min = glm::vec3(pos_accessor.minValues[0], pos_accessor.minValues[1], pos_accessor.minValues[2]);
                // max = glm::vec3(pos_accessor.maxValues[0], pos_accessor.maxValues[1], pos_accessor.maxValues[2]);
            
                m_primitive.vertex_count = (u32)gltf_model.accessors[primitive.attributes.find("POSITION")->second].count;
                m_primitive.index_count = (u32)gltf_model.accessors[primitive.indices].count;

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    fx::gltf::Accessor &normal_accessor = gltf_model.accessors[primitive.attributes.find("NORMAL")->second];
                    fx::gltf::BufferView &normal_buffer_view = gltf_model.bufferViews[normal_accessor.bufferView];
                    normal_buffer = (float *)(&(gltf_model.buffers[normal_buffer_view.buffer].data[normal_accessor.byteOffset + normal_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
                {
                    fx::gltf::Accessor &tangent_accessor = gltf_model.accessors[primitive.attributes.find("TANGENT")->second];
                    fx::gltf::BufferView &tangent_buffer_view = gltf_model.bufferViews[tangent_accessor.bufferView];
                    tangent_buffer = (float *)(&(gltf_model.buffers[tangent_buffer_view.buffer].data[tangent_accessor.byteOffset + tangent_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
                {
                    fx::gltf::Accessor &color_accessor = gltf_model.accessors[primitive.attributes.find("COLOR_0")->second];
                    fx::gltf::BufferView &color_buffer_view = gltf_model.bufferViews[color_accessor.bufferView];
                    color_buffer = (float *)(&(gltf_model.buffers[color_buffer_view.buffer].data[color_accessor.byteOffset + color_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    fx::gltf::Accessor &uv0_accessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    fx::gltf::BufferView &uv0_buffer_view = gltf_model.bufferViews[uv0_accessor.bufferView];
                    uv0_buffer = (float *)(&(gltf_model.buffers[uv0_buffer_view.buffer].data[uv0_accessor.byteOffset + uv0_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
                {
                    fx::gltf::Accessor &uv1_accessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
                    fx::gltf::BufferView &uv1_buffer_view = gltf_model.bufferViews[uv1_accessor.bufferView];
                    uv1_buffer = (float *)(&(gltf_model.buffers[uv1_buffer_view.buffer].data[uv1_accessor.byteOffset + uv1_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("TEXCOORD_2") != primitive.attributes.end()) // gltf files do not current have a 3rd tex coord
                {
                }

                // Fill in the memory block from the vertex_offset onwards
                for (size_t k = 0; k < pos_accessor.count; ++k)
                {
                    mesh_data->position[k + vertex_offset] = glm::make_vec3((position_buffer + (k * 3)));
                    mesh_data->normal[k + vertex_offset]   = (normal_buffer) ? glm::make_vec3((normal_buffer + (k * 3))) : glm::vec3(0);
                    mesh_data->tangent[k + vertex_offset]  = (tangent_buffer) ? glm::make_vec4((tangent_buffer + (k * 4))) : glm::vec4(0);
                    mesh_data->color[k + vertex_offset]    = (color_buffer) ? glm::make_vec4((color_buffer + (k * 4))) : glm::vec4(1);
                    mesh_data->uv0[k + vertex_offset]      = (uv0_buffer) ? glm::make_vec2((uv0_buffer + (k * 2))) : glm::vec2(0);
                    mesh_data->uv1[k + vertex_offset]      = (uv1_buffer) ? glm::make_vec2((uv1_buffer + (k * 2))) : glm::vec2(0);
                    mesh_data->uv2[k + vertex_offset]      = (uv2_buffer) ? glm::make_vec2((uv2_buffer + (k * 2))) : glm::vec2(0);
                }  
            } // end vertex attribs

            // indices
            {
                fx::gltf::Accessor indices_accessor = gltf_model.accessors[primitive.indices];
                fx::gltf::BufferView indices_buffer_view = gltf_model.bufferViews[indices_accessor.bufferView];
                fx::gltf::Buffer &buffer = gltf_model.buffers[indices_buffer_view.buffer];
                
                void *data_ptr = &(buffer.data[indices_accessor.byteOffset + indices_buffer_view.byteOffset]);
                
                int idx_count = (int)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[j].indices].count;

                u32 *buf = static_cast<u32 *>(data_ptr);
                for (size_t k = 0; k < idx_count; ++k)
                {
                    mesh_data->indices[k + index_offset] = buf[k] + vertex_offset;
                }
            } // end index attribs

            // update offsets
            vertex_offset += (u32)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[j].attributes.find("POSITION")->second].count;
            index_offset  += (u32)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[j].indices].count;

            // Add Primitive to list in the mesh
            arrput(mesh.primitives, m_primitive);   
        }

        // Put the mesh in the mesh list
        arrput(meshes, mesh);

        ++idx;
    }


	// cgltf_free(data);

    return meshes;
}