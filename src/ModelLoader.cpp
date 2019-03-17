#define _CRT_SECURE_NO_WARNINGS

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

#include <ModelLoader.h>
#include <glm/gtc/type_ptr.hpp>
#include "RenderBase.h"

EModelLoadResult LoadGTLFModel(SceneModelData& scene_model, Model_Separate_Data &model, PerDrawUniformObject *ubo, uint32_t material_type,
                               uint32_t shader_id, uint32_t uniform_index)
{
    
    // TODO(Dustin): Remove this hardcoded nonsense
    model.material_type = material_type;
    model.shader_id = shader_id;
    model.uniform_index = uniform_index;
    
    glm::vec3 pos = glm::vec3(0.0f, -3.0f, 0.0f);
    glm::vec3 ext = glm::vec3(0.5f, 0.5f, 0.5f);


    // TODO(Dustin): get this from the model matrix 
    model.pos                   = pos;
    model.rot                   = glm::vec3(glm::radians(90.0f), 0.0f, 0.0f);
    model.scl                   = glm::vec3(0.1f);
    model.bounds.min            = glm::vec3(0.0f);
    model.bounds.max            = ext;
    // ubo->model = glm::scale(glm::mat4(1.0f), model.scl);
    // ubo->model = glm::rotate(ubo->model, model.rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
    // ubo->model = glm::rotate(ubo->model, model.rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
    // ubo->model = glm::rotate(ubo->model, model.rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
    // ubo->model = glm::translate(ubo->model, model.pos);

    ubo->model = glm::make_mat4x4(&scene_model.model_matrix[0]);
    
    // char *file = scene_model.filepath;
    //char *file = "resources/models/Lantern/glTF-Binary/Lantern.glb";
    
    tinygltf::Model gltf_model;
    tinygltf::TinyGLTF gltf_ctx;
    std::string err;
    std::string warn;
    
    bool ret = gltf_ctx.LoadBinaryFromFile(&gltf_model, &err, &warn, scene_model.filepath);
    // bool ret = gltf_ctx.LoadASCIIFromFile(&model, &err, &warn, file);
    if (!ret)
        return MODEL_LOAD_RESULT_FILE_NOT_FOUND;
    
    glm::vec3 max;
    glm::vec3 min;
    
    model.model_data = (ModelData *)malloc(sizeof(ModelData));
    
    // determine the size of the model
    model.vertex_count = 0;
    model.index_count  = 0;
    for (int i = 0; i < gltf_model.nodes.size(); ++i)
    {
        if (gltf_model.nodes[i].mesh < 0)
            continue;
        
        model.vertex_count += (uint32_t)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[0].attributes.find("POSITION")->second].count;
        model.index_count  += (uint32_t)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[0].indices].count;
    }
    
    // Allocate memory for the model
    {
        size_t indices_length  = model.index_count * sizeof(uint32_t);
        size_t position_length = model.vertex_count * sizeof(glm::vec3);
        size_t normal_length   = model.vertex_count * sizeof(glm::vec3);
        size_t tangent_length  = model.vertex_count * sizeof(glm::vec4);
        size_t color_length    = model.vertex_count * sizeof(glm::vec4);
        size_t uv0_length      = model.vertex_count * sizeof(glm::vec2);
        size_t uv1_length      = model.vertex_count * sizeof(glm::vec2);
        size_t uv2_length      = model.vertex_count * sizeof(glm::vec2);
        
        model.model_data->memory_block_size = indices_length + position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length + uv2_length;
        model.model_data->memory_block      = (void *)malloc(model.model_data->memory_block_size);
        
        model.model_data->indices  = (uint32_t *)model.model_data->memory_block;
        model.model_data->position = (glm::vec3 *)((char *)model.model_data->memory_block + indices_length);
        model.model_data->normal   = (glm::vec3 *)((char *)model.model_data->memory_block + indices_length + position_length);
        model.model_data->tangent  = (glm::vec4 *)((char *)model.model_data->memory_block + indices_length + position_length + normal_length);
        model.model_data->color    = (glm::vec4 *)((char *)model.model_data->memory_block + indices_length + position_length + normal_length + tangent_length);
        model.model_data->uv0      = (glm::vec2 *)((char *)model.model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length);
        model.model_data->uv1      = (glm::vec2 *)((char *)model.model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length + uv0_length);
        model.model_data->uv2      = (glm::vec2 *)((char *)model.model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length);
        
        model.model_data->attribute_offsets[0] = 0; // First offset into the VertexBuffer is 0
        model.model_data->attribute_offsets[1] = position_length;
        model.model_data->attribute_offsets[2] = position_length + normal_length;
        model.model_data->attribute_offsets[3] = position_length + normal_length + tangent_length;
        model.model_data->attribute_offsets[4] = position_length + normal_length + tangent_length + color_length;
        model.model_data->attribute_offsets[5] = position_length + normal_length + tangent_length + color_length + uv0_length;
        model.model_data->attribute_offsets[6] = position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length;
    }
    
    int vertex_offset = 0;
    int index_offset = 0;
    for (int i = 0; i < gltf_model.nodes.size(); ++i)
    {
        if (gltf_model.nodes[i].mesh < 0)
            continue;
        
        tinygltf::Node node = gltf_model.nodes[i];
        tinygltf::Mesh mesh = gltf_model.meshes[node.mesh];
        
        for (int j = 0; j < mesh.primitives.size(); ++j) // what is a case where there will be more than one primitive?
        {
            tinygltf::Primitive primitive = mesh.primitives[j];
            if (primitive.indices < 0)
                continue; // no indices were found
            
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
                    return MODEL_LOAD_RESULT_INVALID_GLTF;
                
                tinygltf::Accessor &pos_accessor = gltf_model.accessors[primitive.attributes.find("POSITION")->second];
                tinygltf::BufferView &pos_buffer_view = gltf_model.bufferViews[pos_accessor.bufferView];
                position_buffer = (float *)(&(gltf_model.buffers[pos_buffer_view.buffer].data[pos_accessor.byteOffset + pos_buffer_view.byteOffset]));
                min = glm::vec3(pos_accessor.minValues[0], pos_accessor.minValues[1], pos_accessor.minValues[2]);
                max = glm::vec3(pos_accessor.maxValues[0], pos_accessor.maxValues[1], pos_accessor.maxValues[2]);
                
                if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    tinygltf::Accessor &normal_accessor = gltf_model.accessors[primitive.attributes.find("NORMAL")->second];
                    tinygltf::BufferView &normal_buffer_view = gltf_model.bufferViews[normal_accessor.bufferView];
                    normal_buffer = (float *)(&(gltf_model.buffers[normal_buffer_view.buffer].data[normal_accessor.byteOffset + normal_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
                {
                    tinygltf::Accessor &tangent_accessor = gltf_model.accessors[primitive.attributes.find("TANGENT")->second];
                    tinygltf::BufferView &tangent_buffer_view = gltf_model.bufferViews[tangent_accessor.bufferView];
                    tangent_buffer = (float *)(&(gltf_model.buffers[tangent_buffer_view.buffer].data[tangent_accessor.byteOffset + tangent_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
                {
                    tinygltf::Accessor &color_accessor = gltf_model.accessors[primitive.attributes.find("COLOR_0")->second];
                    tinygltf::BufferView &color_buffer_view = gltf_model.bufferViews[color_accessor.bufferView];
                    color_buffer = (float *)(&(gltf_model.buffers[color_buffer_view.buffer].data[color_accessor.byteOffset + color_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    tinygltf::Accessor &uv0_accessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    tinygltf::BufferView &uv0_buffer_view = gltf_model.bufferViews[uv0_accessor.bufferView];
                    uv0_buffer = (float *)(&(gltf_model.buffers[uv0_buffer_view.buffer].data[uv0_accessor.byteOffset + uv0_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
                {
                    tinygltf::Accessor &uv1_accessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
                    tinygltf::BufferView &uv1_buffer_view = gltf_model.bufferViews[uv1_accessor.bufferView];
                    uv1_buffer = (float *)(&(gltf_model.buffers[uv1_buffer_view.buffer].data[uv1_accessor.byteOffset + uv1_buffer_view.byteOffset]));
                }
                
                if (primitive.attributes.find("TEXCOORD_2") != primitive.attributes.end()) // gltf files do not current have a 3rd tex coord
                {
                }
                
                // Fill in the memory block from the vertex_offset onwards
                for (size_t k = 0; k < pos_accessor.count; ++k)
                {
                    model.model_data->position[k + vertex_offset] = glm::make_vec3((position_buffer + (k * 3)));
                    model.model_data->normal[k + vertex_offset]   = (normal_buffer) ? glm::make_vec3((normal_buffer + (k * 3))) : glm::vec3(0);
                    model.model_data->tangent[k + vertex_offset]  = (tangent_buffer) ? glm::make_vec4((tangent_buffer + (k * 4))) : glm::vec4(0);
                    model.model_data->color[k + vertex_offset]    = (color_buffer) ? glm::make_vec4((color_buffer + (k * 4))) : glm::vec4(1);
                    model.model_data->uv0[k + vertex_offset]      = (uv0_buffer) ? glm::make_vec2((uv0_buffer + (k * 2))) : glm::vec2(0);
                    model.model_data->uv1[k + vertex_offset]      = (uv1_buffer) ? glm::make_vec2((uv1_buffer + (k * 2))) : glm::vec2(0);
                    model.model_data->uv2[k + vertex_offset]      = (uv2_buffer) ? glm::make_vec2((uv2_buffer + (k * 2))) : glm::vec2(0);
                }
            }
            
            // indices
            {
                tinygltf::Accessor indices_accessor = gltf_model.accessors[primitive.indices];
                tinygltf::BufferView indices_buffer_view = gltf_model.bufferViews[indices_accessor.bufferView];
                tinygltf::Buffer &buffer = gltf_model.buffers[indices_buffer_view.buffer];
                
                void *data_ptr = &(buffer.data[indices_accessor.byteOffset + indices_buffer_view.byteOffset]);
                
                int idx_count = (int)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[j].indices].count;
                
                // Fill in the memory block from the index_offset onwards
                switch (indices_accessor.componentType)
                {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        uint32_t *buf = static_cast<uint32_t *>(data_ptr);
                        for (size_t k = 0; k < idx_count; ++k)
                        {
                            model.model_data->indices[k + index_offset] = buf[k] + vertex_offset;
                        }
                    }
                    break;
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        uint16_t *buf = static_cast<uint16_t *>(data_ptr);
                        for (size_t k = 0; k < idx_count; ++k)
                        {
                            model.model_data->indices[k + index_offset] = buf[k] + vertex_offset;
                        }
                    }
                    break;
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        uint8_t *buf = static_cast<uint8_t *>(data_ptr);
                        for (size_t k = 0; k < idx_count; ++k)
                        {
                            model.model_data->indices[k + index_offset] = buf[k] + vertex_offset;
                        }
                    }
                    break;
                    default:
                    {
                        printf("ERROR: INVALID INDEX TYPE\n");
                    }
                    break;
                }
            }
            
            vertex_offset += (uint32_t)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[j].attributes.find("POSITION")->second].count;
            index_offset  += (uint32_t)gltf_model.accessors[gltf_model.meshes[gltf_model.nodes[i].mesh].primitives[j].indices].count;
        }
    }
    
    return MODEL_LOAD_RESULT_SUCCESS;
}


void DestroyModelSeparateDataTest(Model_Separate_Data *model, const VulkanInfo *vulkan_info)
{
    free(model->model_data->memory_block);
    free(model->model_data);
    vkDestroyBuffer(vulkan_info->logical_device, model->vertex_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, model->vertex_buffer_memory, nullptr);
    vkDestroyBuffer(vulkan_info->logical_device, model->index_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, model->index_buffer_memory, nullptr);
    model = nullptr;
}

Model_Separate_Data CreateBoxNonInterleaved(glm::vec3 pos, glm::vec3 ext, PerDrawUniformObject *ubo, uint32_t material_type, uint32_t shader_id, uint32_t uniform_index) 
{
    
    Model_Separate_Data model;
    model.material_type = material_type;
    model.shader_id = shader_id;
    model.uniform_index = uniform_index;
    model.hit_test_enabled = false;
    model.vertex_count = 24;
    model.index_count = 36;
    
    // Create the memory block for the data
    model.model_data = (ModelData*)malloc(sizeof(ModelData));
    {
        size_t indices_length  = model.index_count  * sizeof(uint32_t);
        size_t position_length = model.vertex_count * sizeof(glm::vec3);
        size_t normal_length   = model.vertex_count * sizeof(glm::vec3);
        size_t color_length    = model.vertex_count * sizeof(glm::vec4);
        size_t uv0_length      = model.vertex_count * sizeof(glm::vec2);
        size_t uv1_length      = model.vertex_count * sizeof(glm::vec2);
        size_t uv2_length      = model.vertex_count * sizeof(glm::vec2);
        
        model.model_data->memory_block_size = indices_length + position_length + normal_length + color_length + 
            uv0_length + uv1_length + uv2_length;
        
        model.model_data->memory_block = (void*)malloc(model.model_data->memory_block_size);
        
        model.model_data->indices  = (uint32_t* )((char*)model.model_data->memory_block + 0); // indices are first
        model.model_data->position = (glm::vec3*)((char*)model.model_data->memory_block + indices_length);
        model.model_data->normal   = (glm::vec3*)((char*)model.model_data->memory_block + indices_length + position_length);
        model.model_data->color    = (glm::vec4*)((char*)model.model_data->memory_block + indices_length + position_length + normal_length);
        model.model_data->uv0      = (glm::vec2*)((char*)model.model_data->memory_block + indices_length + position_length + normal_length +
                                                  color_length);
        model.model_data->uv1      = (glm::vec2*)((char*)model.model_data->memory_block + indices_length + position_length + normal_length +
                                                  color_length   + uv0_length);
        model.model_data->uv2      = (glm::vec2*)((char*)model.model_data->memory_block + indices_length + position_length + normal_length +
                                                  color_length   + uv0_length      + uv1_length);
    }
    
    // Load the index data
    {
        model.model_data->indices[0] = 0;
        model.model_data->indices[1] = 3;
        model.model_data->indices[2] = 2;
        model.model_data->indices[3] = 0;
        model.model_data->indices[4] = 2;
        model.model_data->indices[5] = 1;
        model.model_data->indices[6] = 4;
        model.model_data->indices[7] = 7;
        model.model_data->indices[8] = 6;
        model.model_data->indices[9] = 4;
        model.model_data->indices[10] = 6;
        model.model_data->indices[11] = 5;
        model.model_data->indices[12] = 8;
        model.model_data->indices[13] = 11;
        model.model_data->indices[14] = 10;
        model.model_data->indices[15] = 8;
        model.model_data->indices[16] = 10;
        model.model_data->indices[17] = 9;
        model.model_data->indices[18] = 12;
        model.model_data->indices[19] = 15;
        model.model_data->indices[20] = 14;
        model.model_data->indices[21] = 12;
        model.model_data->indices[22] = 14;
        model.model_data->indices[23] = 13;
        model.model_data->indices[24] = 16;
        model.model_data->indices[25] = 19;
        model.model_data->indices[26] = 18;
        model.model_data->indices[27] = 16;
        model.model_data->indices[28] = 18;
        model.model_data->indices[29] = 17;
        model.model_data->indices[30] = 20;
        model.model_data->indices[31] = 23;
        model.model_data->indices[32] = 22;
        model.model_data->indices[33] = 20;
        model.model_data->indices[34] = 22;
        model.model_data->indices[35] = 21;
        //}
        
        // Load the position data
        //{
        model.model_data->position[ 0] = glm::vec3(0.0f, 0.0f, 0.0f);
        model.model_data->position[ 1] = glm::vec3(0.0f, 0.0f, ext.z);
        model.model_data->position[ 2] = glm::vec3(ext.x, 0.0f, ext.z);
        model.model_data->position[ 3] = glm::vec3(ext.x, 0.0f, 0.0f);
        model.model_data->position[ 4] = glm::vec3(ext.x, 0.0f, 0.0f);
        model.model_data->position[ 5] = glm::vec3(ext.x, 0.0f, ext.z);
        model.model_data->position[ 6] = glm::vec3(ext.x, ext.y, ext.z);
        model.model_data->position[ 7] = glm::vec3(ext.x, ext.y, 0.0f);
        model.model_data->position[ 8] = glm::vec3(ext.x, ext.y, 0.0f);
        model.model_data->position[ 9] = glm::vec3(ext.x, ext.y, ext.z);
        model.model_data->position[10] = glm::vec3(0.0f, ext.y, ext.z);
        model.model_data->position[11] = glm::vec3(0.0f, ext.y, 0.0f);
        model.model_data->position[12] = glm::vec3(0.0f, ext.y, 0.0f);
        model.model_data->position[13] = glm::vec3(0.0f, ext.y, ext.z);
        model.model_data->position[14] = glm::vec3(0.0f, 0.0f, ext.z);
        model.model_data->position[15] = glm::vec3(0.0f, 0.0f, 0.0f);
        model.model_data->position[16] = glm::vec3(0.0f, 0.0f, ext.z);
        model.model_data->position[17] = glm::vec3(0.0f, ext.y, ext.z);
        model.model_data->position[18] = glm::vec3(ext.x, ext.y, ext.z);
        model.model_data->position[19] = glm::vec3(ext.x, 0.0f, ext.z);
        model.model_data->position[20] = glm::vec3(0.0f, 0.0f, 0.0f);
        model.model_data->position[21] = glm::vec3(ext.x, 0.0f, 0.0f);
        model.model_data->position[22] = glm::vec3(ext.x, ext.y, 0.0f);
        model.model_data->position[23] = glm::vec3(0.0f, ext.y, 0.0f);
    }
    
    // Load the normal data
    {
        model.model_data->normal[ 0] = glm::vec3(0.0f, -1.0f, 0.0f);
        model.model_data->normal[ 1] = glm::vec3(0.0f, -1.0f, 0.0f);
        model.model_data->normal[ 2] = glm::vec3(0.0f, -1.0f, 0.0f);
        model.model_data->normal[ 3] = glm::vec3(0.0f, -1.0f, 0.0f);
        model.model_data->normal[ 4] = glm::vec3(1.0f, 0.0f, 0.0f);
        model.model_data->normal[ 5] = glm::vec3(1.0f, 0.0f, 0.0f);
        model.model_data->normal[ 6] = glm::vec3(1.0f, 0.0f, 0.0f);
        model.model_data->normal[ 7] = glm::vec3(1.0f, 0.0f, 0.0f);
        model.model_data->normal[ 8] = glm::vec3(0.0f, 1.0f, 0.0f);
        model.model_data->normal[ 9] = glm::vec3(0.0f, 1.0f, 0.0f);
        model.model_data->normal[10] = glm::vec3(0.0f, 1.0f, 0.0f);
        model.model_data->normal[11] = glm::vec3(0.0f, 1.0f, 0.0f);
        model.model_data->normal[12] = glm::vec3(-1.0f, 0.0f, 0.0f);
        model.model_data->normal[13] = glm::vec3(-1.0f, 0.0f, 0.0f);
        model.model_data->normal[14] = glm::vec3(-1.0f, 0.0f, 0.0f);
        model.model_data->normal[15] = glm::vec3(-1.0f, 0.0f, 0.0f);
        model.model_data->normal[16] = glm::vec3(0.0f, 0.0f, 1.0f);
        model.model_data->normal[17] = glm::vec3(0.0f, 0.0f, 1.0f);
        model.model_data->normal[18] = glm::vec3(0.0f, 0.0f, 1.0f);
        model.model_data->normal[19] = glm::vec3(0.0f, 0.0f, 1.0f);
        model.model_data->normal[20] = glm::vec3(0.0f, 0.0f, -1.0f);
        model.model_data->normal[21] = glm::vec3(0.0f, 0.0f, -1.0f);
        model.model_data->normal[22] = glm::vec3(0.0f, 0.0f, -1.0f);
        model.model_data->normal[23] = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    
    // Load the color data
    {
        model.model_data->color[ 0] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 1] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 2] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 3] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 4] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 5] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 6] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 7] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 8] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[ 9] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[10] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[11] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[12] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[13] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[14] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[15] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[16] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[17] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[18] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[19] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[20] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[21] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[22] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        model.model_data->color[23] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    }
    
    // Load the uv0 data
    {
        model.model_data->uv0[ 0] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv0[ 1] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv0[ 2] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv0[ 3] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv0[ 4] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv0[ 5] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv0[ 6] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv0[ 7] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv0[ 8] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv0[ 9] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv0[10] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv0[11] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv0[12] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv0[13] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv0[14] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv0[15] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv0[16] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv0[17] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv0[18] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv0[19] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv0[20] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv0[21] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv0[22] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv0[23] = glm::vec2(0.0f, 0.0f);
    }
    
    // Load the uv1 data
    {
        model.model_data->uv1[ 0] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv1[ 1] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv1[ 2] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv1[ 3] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv1[ 4] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv1[ 5] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv1[ 6] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv1[ 7] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv1[ 8] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv1[ 9] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv1[10] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv1[11] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv1[12] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv1[13] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv1[14] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv1[15] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv1[16] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv1[17] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv1[18] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv1[19] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv1[20] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv1[21] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv1[22] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv1[23] = glm::vec2(0.0f, 0.0f);
    }
    
    // Load the uv2 data
    {
        model.model_data->uv2[ 0] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv2[ 1] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv2[ 2] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv2[ 3] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv2[ 4] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv2[ 5] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv2[ 6] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv2[ 7] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv2[ 8] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv2[ 9] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv2[10] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv2[11] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv2[12] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv2[13] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv2[14] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv2[15] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv2[16] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv2[17] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv2[18] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv2[19] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv2[20] = glm::vec2(1.0f, 0.0f);
        model.model_data->uv2[21] = glm::vec2(1.0f, 1.0f);
        model.model_data->uv2[22] = glm::vec2(0.0f, 1.0f);
        model.model_data->uv2[23] = glm::vec2(0.0f, 0.0f);
        model.model_data->uv2[23] = glm::vec2(0.0f, 0.0f);
    }
    
    model.pos = pos;
    model.rot = glm::vec3(0.0f);
    model.scl = glm::vec3(1.0f);
    model.bounds.min = glm::vec3(0.0f);
    model.bounds.max = ext;
    ubo->model = glm::translate(glm::mat4(1.0f), pos);
    
    return model;
}
