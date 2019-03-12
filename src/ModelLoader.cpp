#define _CRT_SECURE_NO_WARNINGS
#define CGLTF_IMPLEMENTATION
//#include <model_loader/cgltf.h>

#include <ModelLoader.h>
#include <glm/gtc/type_ptr.hpp>
#include "RenderBase.h"

#include <stb/stb_ds.h>

char default_model_location[] = "resources/models/Cube/glTF/";

// Convert the CGLTF result return type to be a more engine-friendly type
internal EModelLoadResult
GLTFFailType(cgltf_result result) 
{
    if (result == cgltf_result_data_too_short)
        return MODEL_LOAD_RESULT_DATA_TOO_SHORT;
    else if (result == cgltf_result_unknown_format)
        return MODEL_LOAD_RESULT_UNKNOWN_FORMAT;
    else if (result == cgltf_result_invalid_json)
        return MODEL_LOAD_RESULT_INVALID_JSON;
    else if (result == cgltf_result_invalid_gltf)
        return MODEL_LOAD_RESULT_INVALID_GLTF;
    else if (result == cgltf_result_invalid_options)
        return MODEL_LOAD_RESULT_INVALID_CGLTF_OPTIONS;
    else if (result == cgltf_result_file_not_found)
        return MODEL_LOAD_RESULT_FILE_NOT_FOUND;
    else if (result == cgltf_result_io_error)
        return MODEL_LOAD_RESULT_IO_ERROR;
    else if (result == cgltf_result_out_of_memory)
        return MODEL_LOAD_RESULT_OUT_OF_MEMORY;
    else 
        return MODEL_LOAD_RESULT_UNKNOWN_ERROR;
}

internal VkFormat 
ConvertGLTFAttributeFormatToVulkanFormat(cgltf_type type) 
{
    switch (type) {
        case cgltf_type_scalar:
        {
            return VK_FORMAT_R32_SINT; 
        } break;
        case cgltf_type_vec2:
        {
            return VK_FORMAT_R32G32_SFLOAT;
        } break;
        case cgltf_type_vec3:
        {
            return VK_FORMAT_R32G32B32_SFLOAT;
        } break;
        case cgltf_type_vec4:
        {
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        } break;
        case cgltf_type_mat2:
        {
            // TODO(Dustin)
            return VK_FORMAT_UNDEFINED;
        } break;
        case cgltf_type_mat3:
        {
            // TODO(Dustin)
            return VK_FORMAT_UNDEFINED;
        } break;
        case cgltf_type_mat4:
        {
            // TODO(Dustin)
            return VK_FORMAT_UNDEFINED;
        } break;
        default: return VK_FORMAT_UNDEFINED;;
    }
}

EModelLoadResult LoadGTLFModel(std::string filepath, Model_Separate_Data& model, uint32_t material_type, 
                               uint32_t shader_id, uint32_t uniform_count)
{
    model.material_type = material_type;
    model.shader_id = shader_id;
    model.uniform_count = uniform_count; 

    //model = (Model_GLTF*)malloc(sizeof(Model_GLTF));

    cgltf_options options = {}; // it should auto-detect the file type
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, "resources/models/Cube/glTF/Cube.gltf", &data);

    if (result != cgltf_result_success)
      return GLTFFailType(result);
    
    // const size_t length = sizeof(default_model_location) + sizeof(data->buffers->uri) + 1;
    // char filebin[length] = {0};
    // snprintf(filebin, sizeof(filebin), "%s%s", default_model_location, data->buffers->uri);

    // Now read the bin file 
    result = cgltf_load_buffers(&options, data, "resources/models/Cube/glTF/Cube.bin");
    if (result != cgltf_result_success)
      return GLTFFailType(result);

    
    BufferDataInfo data_info;

    size_t num_attr = data->scenes[0].nodes[0]->mesh->primitives->attributes_count; 
    size_t attr_size = 0;

    void *buffer_indices = nullptr;
    size_t indices_size = 0; 
    float *buffer_pos = nullptr;
    // size_t pos_size = 0;
    float *buffer_normal = nullptr;
    // size_t normal_size = 0;
    float *buffer_tangent = nullptr;
    // size_t tangent_size = 0;
    float *buffer_color = nullptr;
    // size_t color_size = 0;
    float *buffer_uv0 = nullptr;
    // size_t uv0_size = 0;
    float *buffer_uv1 = nullptr;
    // size_t uv1_size = 0;
    float *buffer_uv2 = nullptr;
    // size_t uv2_size = 0;

    //---------------------------------------------------------------------------------------------------------------
    // Load Indices
    //---------------------------------------------------------------------------------------------------------------
    void *ind_temp_ptr = nullptr;
    model.index_count = data->scenes[0].nodes[0]->mesh->primitives->indices->count;
    indices_size = sizeof(uint32_t) * model.index_count;

    // The offset into the data buffer is: (buffer view offset) + (accessor offset) 
    buffer_indices = (void*)((char*)data->buffers[0].data + data->scenes[0].nodes[0]->mesh->primitives->indices->offset + 
        data->scenes[0].nodes[0]->mesh->primitives->indices->buffer_view->offset);

    //---------------------------------------------------------------------------------------------------------------
    // Load Attributes
    //---------------------------------------------------------------------------------------------------------------
    int uv_count = 0;
    for (int i = 0; i < num_attr; ++i)
    {
        // default: cgltf_attribute_type_invalid,
        switch(data->scenes[0].nodes[0]->mesh->primitives->attributes[i].type)
        {
            case cgltf_attribute_type_position:
            {
                int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
                buffer_pos = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +  
                    data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
                model.vertex_count = data->accessors[acc_idx].count;
            } break;
            case cgltf_attribute_type_normal:
            {
                int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
                buffer_normal = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +  
                    data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
            } break;
            case cgltf_attribute_type_tangent:
            {
                int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
                buffer_tangent = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +  
                    data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
            } break;
            case cgltf_attribute_type_texcoord:
            {
                if (uv_count == 0)
                {
                    ++uv_count;
                    int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
                    buffer_uv0 = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +  
                    data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
                }
                else if (uv_count == 1)
                {
                    ++uv_count;
                    int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
                    buffer_uv1 = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +  
                    data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
                }
                else if (uv_count == 2)
                {
                    ++uv_count;
                    int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
                    buffer_uv2 = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +  
                    data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
                }
                else 
                {
                    printf("MODEL LOADING: INVALID NUMBER OF UV ATTRIBUTES\n");
                }
            } break;
            case cgltf_attribute_type_color:
            {
                // found_attributes |= 0x0001000;
                // data_info.color_0_offset = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                // //data_info.color_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                // data_info.color_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            // case cgltf_attribute_type_joints:
            // {
            //     data_info.joints_0_offset = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
            //     attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
            //     //data_info.joints_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
            //     data_info.joints_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->type);
            // } break;
            // case cgltf_attribute_type_weights:
            // {
            //     data_info.weights_0_offset = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
            //     attr_size += data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
            //     //data_info.weights_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size; 
            //     data_info.weights_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->type);
            // } break;
            default: break;
        }
    }

    glm::vec3 box_pos = glm::vec3(-0.3f, -0.3f, -0.3f);
    glm::vec3 box_ext = glm::vec3(0.5f, 0.5f, 0.5f);

    model.pos = box_pos;
    model.rot = glm::vec3(0.0f);
    model.scl = glm::vec3(1.0f);
    model.bounds.min = glm::vec3(0.0f);
    model.bounds.max = box_ext;
    model.ubo.model = glm::translate(glm::mat4(1.0f), box_pos);
    model.ubo.view_position = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    model.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
    model.ubo.projection[1][1] *= -1;

    model.model_data = (ModelData*)malloc(sizeof(ModelData));

    size_t position_length = model.vertex_count * sizeof(glm::vec3);
    size_t normal_length   = model.vertex_count * sizeof(glm::vec3);
    size_t tangent_length  = model.vertex_count * sizeof(glm::vec4);
    size_t color_length    = model.vertex_count * sizeof(glm::vec4);
    size_t uv0_length      = model.vertex_count * sizeof(glm::vec2);
    size_t uv1_length      = model.vertex_count * sizeof(glm::vec2);
    size_t uv2_length      = model.vertex_count * sizeof(glm::vec2);

    model.model_data->memory_block_size = indices_size + position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length + uv2_length;
    model.model_data->memory_block = (void*)malloc(model.model_data->memory_block_size);

    model.model_data->indices = (uint32_t*)model.model_data->memory_block;
    switch (data->scenes[0].nodes[0]->mesh->primitives->indices->component_type)
    {
        case cgltf_component_type_r_8u:// unsigned byte
        {
            uint8_t *buff = static_cast<uint8_t*>(buffer_indices);
            for (int i = 0; i < model.index_count; ++i)
            {
                model.model_data->indices[i] = buff[i];
            }
        } break;
        case cgltf_component_type_r_16u: // unsigned short
        {
            uint16_t *buff = static_cast<uint16_t*>(buffer_indices);
            for (int i = 0; i < model.index_count; ++i)
            {
                model.model_data->indices[i] = buff[i];
            }
        } break;
        case cgltf_component_type_r_32u: // unsigned int
        {
            uint32_t *buff = static_cast<uint32_t*>(buffer_indices);
            for (int i = 0; i < model.index_count; ++i)
            {
                model.model_data->indices[i] = buff[i];
            }
        } break;
        default: 
        {
            printf("INVALID INDEX COMPONENT TYPE\n");
        }break;
    }

    // Positions
    assert(buffer_pos); // assert that this is not NULL
    model.model_data->position  = (glm::vec3*)((char*)model.model_data->memory_block + indices_size);
    for (int j = 0; j < model.vertex_count; ++j)
    {
        model.model_data->position[j] = glm::make_vec3(buffer_pos + (j * 3));
    }

    // Normals
    model.model_data->normal   = (glm::vec3*)((char*)model.model_data->memory_block + indices_size + position_length);
    for (int j = 0; j < model.vertex_count; ++j)
    {
        model.model_data->normal[j] = (buffer_normal) ? glm::make_vec3(buffer_normal + (j * 3)) : glm::vec3(0);
    }

    // Tangents
    model.model_data->tangent  = (glm::vec4*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length);
    for (int j = 0; j < model.vertex_count; ++j)
    {
        model.model_data->tangent[j] = (buffer_tangent) ? glm::make_vec4(buffer_tangent + (j * 4)) : glm::vec4(0);
    }

    // Color
    model.model_data->color    = (glm::vec4*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length +
                                    tangent_length);
    for (int j = 0; j < model.vertex_count; ++j)
    {
        model.model_data->color[j] = (buffer_color) ? glm::make_vec4(buffer_color + (j * 4)) : glm::vec4(0);
    }

    // UV0
    model.model_data->uv0      = (glm::vec2*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length +
                                    tangent_length + color_length);
    for (int j = 0; j < model.vertex_count; ++j)
    {
        model.model_data->uv0[j] = (buffer_uv0) ? glm::make_vec2(buffer_uv0 + (j * 2)) : glm::vec2(0);
    }

    // UV1
    model.model_data->uv1      = (glm::vec2*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length +
                                    tangent_length + color_length + uv0_length);
    for (int j = 0; j < model.vertex_count; ++j)
    {
         model.model_data->uv1[j] = (buffer_uv1) ? glm::make_vec2(buffer_uv1 + (j * 2)) : glm::vec2(0);
    }

    // UV2
    model.model_data->uv2      = (glm::vec2*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length +
                                    tangent_length + color_length + uv0_length + uv1_length);
    for (int j = 0; j < model.vertex_count; ++j)
    {
         model.model_data->uv2[j] = (buffer_uv2) ? glm::make_vec2(buffer_uv2 + (j * 2)) : glm::vec2(0);
    }

     // These for-loops allow me to visualize the buffer contents when debugging is VS
    // for (int i = 0; i < model.index_count; ++i) {

    // }

    // for (int i = 0; i < model.vertex_count; ++i) {
        
    // }

    return MODEL_LOAD_RESULT_SUCCESS;
}