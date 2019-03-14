#define _CRT_SECURE_NO_WARNINGS
//#define CGLTF_IMPLEMENTATION
//#include <model_loader/cgltf.h>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

#include <ModelLoader.h>
#include <glm/gtc/type_ptr.hpp>
#include "RenderBase.h"

//#include <stb/stb_ds.h>

char default_model_location[] = "resources/models/Cube/glTF/";

// Convert the CGLTF result return type to be a more engine-friendly type
// internal EModelLoadResult
// GLTFFailType(cgltf_result result)
// {
//     if (result == cgltf_result_data_too_short)
//         return MODEL_LOAD_RESULT_DATA_TOO_SHORT;
//     else if (result == cgltf_result_unknown_format)
//         return MODEL_LOAD_RESULT_UNKNOWN_FORMAT;
//     else if (result == cgltf_result_invalid_json)
//         return MODEL_LOAD_RESULT_INVALID_JSON;
//     else if (result == cgltf_result_invalid_gltf)
//         return MODEL_LOAD_RESULT_INVALID_GLTF;
//     else if (result == cgltf_result_invalid_options)
//         return MODEL_LOAD_RESULT_INVALID_CGLTF_OPTIONS;
//     else if (result == cgltf_result_file_not_found)
//         return MODEL_LOAD_RESULT_FILE_NOT_FOUND;
//     else if (result == cgltf_result_io_error)
//         return MODEL_LOAD_RESULT_IO_ERROR;
//     else if (result == cgltf_result_out_of_memory)
//         return MODEL_LOAD_RESULT_OUT_OF_MEMORY;
//     else
//         return MODEL_LOAD_RESULT_UNKNOWN_ERROR;
// }

// internal VkFormat
// ConvertGLTFAttributeFormatToVulkanFormat(cgltf_type type)
// {
//     switch (type) {
//         case cgltf_type_scalar:
//         {
//             return VK_FORMAT_R32_SINT;
//         } break;
//         case cgltf_type_vec2:
//         {
//             return VK_FORMAT_R32G32_SFLOAT;
//         } break;
//         case cgltf_type_vec3:
//         {
//             return VK_FORMAT_R32G32B32_SFLOAT;
//         } break;
//         case cgltf_type_vec4:
//         {
//             return VK_FORMAT_R32G32B32A32_SFLOAT;
//         } break;
//         case cgltf_type_mat2:
//         {
//             // TODO(Dustin)
//             return VK_FORMAT_UNDEFINED;
//         } break;
//         case cgltf_type_mat3:
//         {
//             // TODO(Dustin)
//             return VK_FORMAT_UNDEFINED;
//         } break;
//         case cgltf_type_mat4:
//         {
//             // TODO(Dustin)
//             return VK_FORMAT_UNDEFINED;
//         } break;
//         default: return VK_FORMAT_UNDEFINED;;
//     }
// }

EModelLoadResult LoadGTLFModel(std::string filepath, Model_Separate_Data &ten, uint32_t material_type,
                               uint32_t shader_id, uint32_t uniform_count)
{

    ten.material_type = material_type;
    ten.shader_id = shader_id;
    ten.uniform_count = uniform_count;

    glm::vec3 box_pos = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 box_ext = glm::vec3(0.5f, 0.5f, 0.5f);

    ten.pos = box_pos;
    ten.rot = glm::vec3(0.0f);
    ten.scl = glm::vec3(1.0f);
    ten.bounds.min = glm::vec3(0.0f);
    ten.bounds.max = box_ext;
    ten.ubo.model = glm::translate(glm::mat4(1.0f), box_pos);
    ten.ubo.view_position = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    ten.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ten.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
    ten.ubo.projection[1][1] *= -1;

    // char *file = "resources/models/BlenderCube.glb";
    char *file = "resources/models/Cube/glTF/Cube.gltf";
    char *file_bin = "resources/models/BoxTextured/glTF/BoxTextured.bin";

    tinygltf::Model model;
    tinygltf::TinyGLTF gltf_ctx;
    std::string err;
    std::string warn;

    bool ret = gltf_ctx.LoadBinaryFromFile(&model, &err, &warn, file);
    if (!ret)
      return MODEL_LOAD_RESULT_FILE_NOT_FOUND;

    // for now I am going to assume:
    //   1 model, so one node
    //   that there is a node

    glm::vec3 max;
    glm::vec3 min;

    ten.model_data = (ModelData *)malloc(sizeof(ModelData));

    tinygltf::Node node = model.nodes[0];
    tinygltf::Mesh mesh = model.meshes[node.mesh];
    tinygltf::Primitive primitive = mesh.primitives[0];
    tinygltf::Accessor &pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
    tinygltf::BufferView &pos_buffer_view = model.bufferViews[pos_accessor.bufferView];

    ten.vertex_count = pos_accessor.count;
    ten.index_count = model.accessors[model.meshes[model.nodes[0].mesh].primitives[0].indices].count;

    size_t indices_length = ten.index_count * sizeof(uint32_t);
    size_t position_length = ten.vertex_count * sizeof(glm::vec3);
    size_t normal_length = ten.vertex_count * sizeof(glm::vec3);
    size_t tangent_length = ten.vertex_count * sizeof(glm::vec4);
    size_t color_length = ten.vertex_count * sizeof(glm::vec4);
    size_t uv0_length = ten.vertex_count * sizeof(glm::vec2);
    size_t uv1_length = ten.vertex_count * sizeof(glm::vec2);
    size_t uv2_length = ten.vertex_count * sizeof(glm::vec2);

    ten.model_data->memory_block_size = indices_length + position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length + uv2_length;
    ten.model_data->memory_block = (void *)malloc(ten.model_data->memory_block_size);

    ten.model_data->indices = (uint32_t *)ten.model_data->memory_block;
    ten.model_data->position = (glm::vec3 *)((char *)ten.model_data->memory_block + indices_length);
    ten.model_data->normal = (glm::vec3 *)((char *)ten.model_data->memory_block + indices_length + position_length);
    ten.model_data->tangent = (glm::vec4 *)((char *)ten.model_data->memory_block + indices_length + position_length + normal_length);
    ten.model_data->color = (glm::vec4 *)((char *)ten.model_data->memory_block + indices_length + position_length + normal_length + tangent_length);
    ten.model_data->uv0 = (glm::vec2 *)((char *)ten.model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length);
    ten.model_data->uv1 = (glm::vec2 *)((char *)ten.model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length + uv0_length);
    ten.model_data->uv2 = (glm::vec2 *)((char *)ten.model_data->memory_block + indices_length + position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length);

    for (int i = 0; i < model.nodes.size(); ++i)
    {

        tinygltf::Node node = model.nodes[i];
        tinygltf::Mesh mesh = model.meshes[node.mesh];

        for (int j = 0; j < mesh.primitives.size(); ++j) // what is a case where there will be more than one primitive?
        {
            tinygltf::Primitive primitive = mesh.primitives[j];
            if (primitive.indices < 0)
                continue; // no indices were found

    //         // Allocate memory for the memory block
    //         //{

    //         //}

            // vertices
            {
                float *position_buffer = nullptr;
                float *normal_buffer = nullptr;
                float *tangent_buffer = nullptr;
                float *color_buffer = nullptr;
                float *uv0_buffer = nullptr;
                float *uv1_buffer = nullptr;
                float *uv2_buffer = nullptr;

                if (primitive.attributes.find("POSITION") == primitive.attributes.end())
                    return MODEL_LOAD_RESULT_INVALID_GLTF;

                tinygltf::Accessor &pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
                tinygltf::BufferView &pos_buffer_view = model.bufferViews[pos_accessor.bufferView];
                position_buffer = (float *)(&(model.buffers[pos_buffer_view.buffer].data[pos_accessor.byteOffset + pos_buffer_view.byteOffset]));
                min = glm::vec3(pos_accessor.minValues[0], pos_accessor.minValues[1], pos_accessor.minValues[2]);
                max = glm::vec3(pos_accessor.maxValues[0], pos_accessor.maxValues[1], pos_accessor.maxValues[2]);

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    tinygltf::Accessor &normal_accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                    tinygltf::BufferView &normal_buffer_view = model.bufferViews[normal_accessor.bufferView];
                    normal_buffer = (float *)(&(model.buffers[normal_buffer_view.buffer].data[normal_accessor.byteOffset + normal_buffer_view.byteOffset]));
                }

                if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
                {
                    tinygltf::Accessor &tangent_accessor = model.accessors[primitive.attributes.find("TANGENT")->second];
                    tinygltf::BufferView &tangent_buffer_view = model.bufferViews[tangent_accessor.bufferView];
                    tangent_buffer = (float *)(&(model.buffers[tangent_buffer_view.buffer].data[tangent_accessor.byteOffset + tangent_buffer_view.byteOffset]));
                }

                if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
                {
                    tinygltf::Accessor &color_accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
                    tinygltf::BufferView &color_buffer_view = model.bufferViews[color_accessor.bufferView];
                    color_buffer = (float *)(&(model.buffers[color_buffer_view.buffer].data[color_accessor.byteOffset + color_buffer_view.byteOffset]));
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    tinygltf::Accessor &uv0_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    tinygltf::BufferView &uv0_buffer_view = model.bufferViews[uv0_accessor.bufferView];
                    uv0_buffer = (float *)(&(model.buffers[uv0_buffer_view.buffer].data[uv0_accessor.byteOffset + uv0_buffer_view.byteOffset]));
                }

                if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
                {
                    tinygltf::Accessor &uv1_accessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
                    tinygltf::BufferView &uv1_buffer_view = model.bufferViews[uv1_accessor.bufferView];
                    uv1_buffer = (float *)(&(model.buffers[uv1_buffer_view.buffer].data[uv1_accessor.byteOffset + uv1_buffer_view.byteOffset]));
                }

                if (primitive.attributes.find("TEXCOORD_2") != primitive.attributes.end()) // gltf files do not current have a 3rd tex coord
                {
                }

                for (size_t k = 0; k < ten.vertex_count; ++k)
                {
                    ten.model_data->position[k] = glm::make_vec3((position_buffer + (k * 3)));
                    ten.model_data->normal[k] = (normal_buffer) ? glm::make_vec3((normal_buffer + (k * 3))) : glm::vec3(0);
                    ten.model_data->tangent[k] = (tangent_buffer) ? glm::make_vec4((tangent_buffer + (k * 4))) : glm::vec4(0);
                    ten.model_data->color[k] = (color_buffer) ? glm::make_vec4((color_buffer + (k * 4))) : glm::vec4(0);
                    ten.model_data->uv0[k] = (uv0_buffer) ? glm::make_vec2((uv0_buffer + (k * 2))) : glm::vec2(0);
                    ten.model_data->uv1[k] = (uv1_buffer) ? glm::make_vec2((uv1_buffer + (k * 2))) : glm::vec2(0);
                    ten.model_data->uv2[k] = (uv2_buffer) ? glm::make_vec2((uv2_buffer + (k * 2))) : glm::vec2(0);
                }
            }

            // indices
            {
                tinygltf::Accessor indices_accessor = model.accessors[primitive.indices];
                tinygltf::BufferView indices_buffer_view = model.bufferViews[indices_accessor.bufferView];
                tinygltf::Buffer &buffer = model.buffers[indices_buffer_view.buffer];

                void *data_ptr = &(buffer.data[indices_accessor.byteOffset + indices_buffer_view.byteOffset]);

                switch (indices_accessor.componentType)
                {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                {
                    uint32_t *buf = static_cast<uint32_t *>(data_ptr);
                    for (size_t k = 0; k < ten.index_count; ++k)
                    {
                        ten.model_data->indices[k] = buf[k];
                    }
                }
                break;
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                {
                    uint16_t *buf = static_cast<uint16_t *>(data_ptr);
                    for (size_t k = 0; k < ten.index_count; ++k)
                    {
                        ten.model_data->indices[k] = buf[k];
                    }
                }
                break;
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                {
                    uint8_t *buf = static_cast<uint8_t *>(data_ptr);
                    for (size_t k = 0; k < ten.index_count; ++k)
                    {
                        ten.model_data->indices[k] = buf[k];
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
        }
    }

    // for (auto node : model.nodes)
    // {
    //     delete node;
    // }
    //model.nodes.resize(0);

    return MODEL_LOAD_RESULT_SUCCESS;
}

// EModelLoadResult LoadGTLFModel(std::string filepath, Model_Separate_Data& model, uint32_t material_type,
//                                uint32_t shader_id, uint32_t uniform_count)
// {
//     LoadTinyGTLFModel();

//     model.material_type = material_type;
//     model.shader_id = shader_id;
//     model.uniform_count = uniform_count;

//     //model = (Model_GLTF*)malloc(sizeof(Model_GLTF));

//     cgltf_options options = {}; // it should auto-detect the file type
//     cgltf_data* data = NULL;
//     cgltf_result result = cgltf_parse_file(&options, "resources/models/Cube/glTF/Cube.gltf", &data);

//     if (result != cgltf_result_success)
//       return GLTFFailType(result);

//     // const size_t length = sizeof(default_model_location) + sizeof(data->buffers->uri) + 1;
//     // char filebin[length] = {0};
//     // snprintf(filebin, sizeof(filebin), "%s%s", default_model_location, data->buffers->uri);

//     // Now read the bin file
//     result = cgltf_load_buffers(&options, data, "resources/models/Cube/glTF/Cube.bin");
//     if (result != cgltf_result_success)
//       return GLTFFailType(result);

//     BufferDataInfo data_info;

//     size_t num_attr = data->scenes[0].nodes[0]->mesh->primitives->attributes_count;
//     size_t attr_size = 0;

//     void *buffer_indices = nullptr;
//     size_t indices_size = 0;
//     float *buffer_pos = nullptr;
//     // size_t pos_size = 0;
//     float *buffer_normal = nullptr;
//     // size_t normal_size = 0;
//     float *buffer_tangent = nullptr;
//     // size_t tangent_size = 0;
//     float *buffer_color = nullptr;
//     // size_t color_size = 0;
//     float *buffer_uv0 = nullptr;
//     // size_t uv0_size = 0;
//     float *buffer_uv1 = nullptr;
//     // size_t uv1_size = 0;
//     float *buffer_uv2 = nullptr;
//     // size_t uv2_size = 0;

//     //---------------------------------------------------------------------------------------------------------------
//     // Load Indices
//     //---------------------------------------------------------------------------------------------------------------
//     void *ind_temp_ptr = nullptr;
//     model.index_count = data->scenes[0].nodes[0]->mesh->primitives->indices->count;
//     indices_size = sizeof(uint32_t) * model.index_count;

//     // The offset into the data buffer is: (buffer view offset) + (accessor offset)
//     buffer_indices = (void*)((char*)data->buffers[0].data + data->scenes[0].nodes[0]->mesh->primitives->indices->offset +
//         data->scenes[0].nodes[0]->mesh->primitives->indices->buffer_view->offset);

//     //---------------------------------------------------------------------------------------------------------------
//     // Load Attributes
//     //---------------------------------------------------------------------------------------------------------------
//     int uv_count = 0;
//     for (int i = 0; i < num_attr; ++i)
//     {
//         // default: cgltf_attribute_type_invalid,
//         switch(data->scenes[0].nodes[0]->mesh->primitives->attributes[i].type)
//         {
//             case cgltf_attribute_type_position:
//             {
//                 int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
//                 buffer_pos = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +
//                     data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
//                 model.vertex_count = data->accessors[acc_idx].count;
//             } break;
//             case cgltf_attribute_type_normal:
//             {
//                 int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
//                 buffer_normal = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +
//                     data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
//             } break;
//             case cgltf_attribute_type_tangent:
//             {
//                 int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
//                 buffer_tangent = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +
//                     data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
//             } break;
//             case cgltf_attribute_type_texcoord:
//             {
//                 if (uv_count == 0)
//                 {
//                     ++uv_count;
//                     int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
//                     buffer_uv0 = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +
//                     data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
//                 }
//                 else if (uv_count == 1)
//                 {
//                     ++uv_count;
//                     int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
//                     buffer_uv1 = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +
//                     data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
//                 }
//                 else if (uv_count == 2)
//                 {
//                     ++uv_count;
//                     int acc_idx = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].index;
//                     buffer_uv2 = (float *)((char*)data->buffers[0].data + data->accessors[acc_idx].offset +
//                     data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset);
//                 }
//                 else
//                 {
//                     printf("MODEL LOADING: INVALID NUMBER OF UV ATTRIBUTES\n");
//                 }
//             } break;
//             case cgltf_attribute_type_color:
//             {
//                 // found_attributes |= 0x0001000;
//                 // data_info.color_0_offset = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
//                 // //data_info.color_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
//                 // data_info.color_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->type);
//             } break;
//             // case cgltf_attribute_type_joints:
//             // {
//             //     data_info.joints_0_offset = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
//             //     attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
//             //     //data_info.joints_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
//             //     data_info.joints_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->type);
//             // } break;
//             // case cgltf_attribute_type_weights:
//             // {
//             //     data_info.weights_0_offset = data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
//             //     attr_size += data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
//             //     //data_info.weights_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
//             //     data_info.weights_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scenes[0].nodes[0]->mesh->primitives->attributes[i].data->type);
//             // } break;
//             default: break;
//         }
//     }

//     glm::vec3 box_pos = glm::vec3(-0.3f, -0.3f, -0.3f);
//     glm::vec3 box_ext = glm::vec3(0.5f, 0.5f, 0.5f);

//     model.pos = box_pos;
//     model.rot = glm::vec3(0.0f);
//     model.scl = glm::vec3(1.0f);
//     model.bounds.min = glm::vec3(0.0f);
//     model.bounds.max = box_ext;
//     model.ubo.model = glm::translate(glm::mat4(1.0f), box_pos);
//     model.ubo.view_position = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
//     model.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//     model.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
//     model.ubo.projection[1][1] *= -1;

//     model.model_data = (ModelData*)malloc(sizeof(ModelData));

//     size_t position_length = model.vertex_count * sizeof(glm::vec3);
//     size_t normal_length   = model.vertex_count * sizeof(glm::vec3);
//     size_t tangent_length  = model.vertex_count * sizeof(glm::vec4);
//     size_t color_length    = model.vertex_count * sizeof(glm::vec4);
//     size_t uv0_length      = model.vertex_count * sizeof(glm::vec2);
//     size_t uv1_length      = model.vertex_count * sizeof(glm::vec2);
//     size_t uv2_length      = model.vertex_count * sizeof(glm::vec2);

//     model.model_data->memory_block_size = indices_size + position_length + normal_length + tangent_length + color_length + uv0_length + uv1_length + uv2_length;
//     model.model_data->memory_block = (void*)malloc(model.model_data->memory_block_size);

//     model.model_data->indices = (uint32_t*)model.model_data->memory_block;
//     switch (data->scenes[0].nodes[0]->mesh->primitives->indices->component_type)
//     {
//         case cgltf_component_type_r_8u:// unsigned byte
//         {
//             uint8_t *buff = static_cast<uint8_t*>(buffer_indices);
//             for (int i = 0; i < model.index_count; ++i)
//             {
//                 model.model_data->indices[i] = buff[i];
//             }
//         } break;
//         case cgltf_component_type_r_16u: // unsigned short
//         {
//             uint16_t *buff = static_cast<uint16_t*>(buffer_indices);
//             for (int i = 0; i < model.index_count; ++i)
//             {
//                 model.model_data->indices[i] = buff[i];
//             }
//         } break;
//         case cgltf_component_type_r_32u: // unsigned int
//         {
//             uint32_t *buff = static_cast<uint32_t*>(buffer_indices);
//             for (int i = 0; i < model.index_count; ++i)
//             {
//                 model.model_data->indices[i] = buff[i];
//             }
//         } break;
//         default:
//         {
//             printf("INVALID INDEX COMPONENT TYPE\n");
//         }break;
//     }

//     // Positions
//     assert(buffer_pos); // assert that this is not NULL
//     model.model_data->position  = (glm::vec3*)((char*)model.model_data->memory_block + indices_size);
//     for (int j = 0; j < model.vertex_count; ++j)
//     {
//         model.model_data->position[j] = glm::make_vec3(buffer_pos + (j * 3));
//     }

//     // Normals
//     model.model_data->normal   = (glm::vec3*)((char*)model.model_data->memory_block + indices_size + position_length);
//     for (int j = 0; j < model.vertex_count; ++j)
//     {
//         model.model_data->normal[j] = (buffer_normal) ? glm::make_vec3(buffer_normal + (j * 3)) : glm::vec3(0);
//     }

//     // Tangents
//     model.model_data->tangent  = (glm::vec4*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length);
//     for (int j = 0; j < model.vertex_count; ++j)
//     {
//         model.model_data->tangent[j] = (buffer_tangent) ? glm::make_vec4(buffer_tangent + (j * 4)) : glm::vec4(0);
//     }

//     // Color
//     model.model_data->color    = (glm::vec4*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length +
//                                     tangent_length);
//     for (int j = 0; j < model.vertex_count; ++j)
//     {
//         model.model_data->color[j] = (buffer_color) ? glm::make_vec4(buffer_color + (j * 4)) : glm::vec4(0);
//     }

//     // UV0
//     model.model_data->uv0      = (glm::vec2*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length +
//                                     tangent_length + color_length);
//     for (int j = 0; j < model.vertex_count; ++j)
//     {
//         model.model_data->uv0[j] = (buffer_uv0) ? glm::make_vec2(buffer_uv0 + (j * 2)) : glm::vec2(0);
//     }

//     // UV1
//     model.model_data->uv1      = (glm::vec2*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length +
//                                     tangent_length + color_length + uv0_length);
//     for (int j = 0; j < model.vertex_count; ++j)
//     {
//          model.model_data->uv1[j] = (buffer_uv1) ? glm::make_vec2(buffer_uv1 + (j * 2)) : glm::vec2(0);
//     }

//     // UV2
//     model.model_data->uv2      = (glm::vec2*)((char*)model.model_data->memory_block + indices_size + position_length + normal_length +
//                                     tangent_length + color_length + uv0_length + uv1_length);
//     for (int j = 0; j < model.vertex_count; ++j)
//     {
//          model.model_data->uv2[j] = (buffer_uv2) ? glm::make_vec2(buffer_uv2 + (j * 2)) : glm::vec2(0);
//     }

//      // These for-loops allow me to visualize the buffer contents when debugging is VS
//     // for (int i = 0; i < model.index_count; ++i) {

//     // }

//     // for (int i = 0; i < model.vertex_count; ++i) {

//     // }

//     return MODEL_LOAD_RESULT_SUCCESS;
// }