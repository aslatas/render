#define _CRT_SECURE_NO_WARNINGS
#define CGLTF_IMPLEMENTATION
//#include <model_loader/cgltf.h>

#include <ModelLoader.h>
#include <glm/gtc/type_ptr.hpp>
#include "RenderBase.h"

#include <stb/stb_ds.h>

char default_model_location[] = "resources/models/Cube/glTF/";

/*
// TESTING
void CreateModelBuffer(VkDeviceSize buffer_size, void* buffer_data, VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
    //VkDeviceSize buffer_size = sizeof(Vertex) * model->vertex_count;
    //VkDeviceSize buffer_size = buffer_size;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    //memcpy(data, model->vertices, (size_t)buffer_size);
    memcpy(data, buffer_data, (size_t)buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    
    CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *buffer, *buffer_memory);
    
    CopyBuffer(&vulkan_info, staging_buffer, *buffer, buffer_size);
    
    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void CreateModelUniformBuffers(VkDeviceSize buffer_size, 
                               VkBuffer* uniform_buffers, 
                               VkDeviceMemory* uniform_buffers_memory, 
                               uint32_t uniform_count)
{
    // VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    // uniform_buffers = (VkBuffer *)malloc(sizeof(VkBuffer) * uniform_count);
    // uniform_buffers_memory = (VkDeviceMemory *)malloc(sizeof(VkDeviceMemory) * uniform_count);
    for (uint32_t i = 0; i < uniform_count; ++i)
    {
        CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers[i], uniform_buffers_memory[i]);
    }
}

void CreateModelDescriptorSets(uint32_t uniform_count, 
                               uint32_t material_type, 
                               uint32_t shader_id, 
                               VkBuffer* uniform_buffers, 
                               VkDescriptorSet *descriptor_sets)
{
    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = vulkan_info.descriptor_pool;
    allocate_info.descriptorSetCount = swapchain_info.image_count;
    allocate_info.pSetLayouts = material_types[material_type].descriptor_layouts;
    VK_CHECK_RESULT(vkAllocateDescriptorSets(vulkan_info.logical_device, &allocate_info, descriptor_sets));
    
    for (uint32_t i = 0; i < uniform_count; ++i)
    {
        VkDescriptorBufferInfo descriptor_info = {};
        descriptor_info.buffer = uniform_buffers[i];
        descriptor_info.offset = 0;
        descriptor_info.range = sizeof(UniformBufferObject);
        
        VkDescriptorImageInfo image_info = {};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // TODO(Matt): Temporary hack to get a second texture sampler for text.
        if (shader_id == 4) {
            
            image_info.imageView = font.texture.image_view;
            image_info.sampler = font.texture.sampler;
            
        } else {
            image_info.imageView = texture.image_view;
            image_info.sampler = texture.sampler;
        }
        VkWriteDescriptorSet uniform_write = {};
        uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniform_write.dstSet = descriptor_sets[i];
        uniform_write.dstBinding = 0;
        uniform_write.dstArrayElement = 0;
        uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniform_write.descriptorCount = 1;
        uniform_write.pBufferInfo = &descriptor_info;
        
        VkWriteDescriptorSet sampler_write = {};
        sampler_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        sampler_write.dstSet = descriptor_sets[i];
        sampler_write.dstBinding = 1;
        sampler_write.dstArrayElement = 0;
        sampler_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_write.descriptorCount = 1;
        sampler_write.pImageInfo = &image_info;
        VkWriteDescriptorSet descriptor_writes[] = {uniform_write, sampler_write};
        vkUpdateDescriptorSets(vulkan_info.logical_device, 2, descriptor_writes, 0, nullptr);
    }
}


*/

// Convert the CGLTF result return type to be a more engine-friendly type
internal EModelLoadResult
GLTFFailType(cgltf_result result) 
{
    printf("Result: %d", result);
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

// void DestroyGLTFModel(Model_GLTF *model, const VulkanInfo *vulkan_info)
// {
//     cgltf_free(model->data);
//     // free(model->data);
//     // free(model->indices);
//     for (uint32_t i = 0; i < model->uniform_count; ++i) {
//         vkDestroyBuffer(vulkan_info->logical_device, model->uniform_buffers[i], nullptr);
//         vkFreeMemory(vulkan_info->logical_device, model->uniform_buffers_memory[i], nullptr);
//     }
//     vkDestroyBuffer(vulkan_info->logical_device, model->vertex_buffer, nullptr);
//     vkFreeMemory(vulkan_info->logical_device, model->vertex_buffer_memory, nullptr);
//     vkDestroyBuffer(vulkan_info->logical_device, model->index_buffer, nullptr);
//     vkFreeMemory(vulkan_info->logical_device, model->index_buffer_memory, nullptr);
//     free(model->uniform_buffers);
//     free(model->uniform_buffers_memory);
//     free(model->descriptor_sets);
//     model = nullptr;
//     //free(model);
// }

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

// struct Vertex_GLTF
// {
//     glm::vec3 position; // 12
//     glm::vec3 normal;   // 24
//     glm::vec3 tangent;
//     // glm::vec4 color; // 40
//     glm::vec2 uv0;   // 48
//     // glm::vec2 uv1;   // 56
//     // glm::vec2 uv2;   // 64
// };

internal void 
ParseData(cgltf_data* data, BufferDataInfo data_info)
{
    // printf("Position Size: %d\nPosition Start: %d\n", data_info.pos_size, data_info.pos_offset);
    // printf("Normal Size: %d\nNormal Start: %d\n", data_info.normal_size, data_info.normal_offset);
    // printf("Texture Coord Size: %d\nTexture Coord Start: %d\n", data_info.tex_0_size, data_info.tex_0_offset);
    // printf("UV Size: %d\nUV Start: %d\n", data_info.tex_0_size, data_info.tex_0_offset);
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
    
    const size_t length = sizeof(default_model_location) + sizeof(data->buffers->uri) + 1;
    char filebin[length] = {0};
    snprintf(filebin, sizeof(filebin), "%s%s", default_model_location, data->buffers->uri);

    // Now read the bin file 
    result = cgltf_load_buffers(&options, data, "resources/models/Cube/glTF/Cube.bin");
    if (result != cgltf_result_success)
      return GLTFFailType(result);

    
    BufferDataInfo data_info;

    // Order of flags:
    // uv2 uv1 uv0 ~color~ tangent normal position
    // unsigned int found_attributes = 0x000000000;

    printf("String of vec3: %d", sizeof(glm::vec3));

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

    

    // size_t len = arrlen(buffer_indices);
    // for (int i = 0; i < arrlen(buffer_indices); ++i)
    // {
    //     printf("Index %d: %d", i, buffer_indices[i]);
    // }

    //int ind_idx = data->scenes[0].nodes[0]->mesh->primitives->indices->;

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

    // Now that the buffer arrays are set...create the memory block to hold them




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
    // memcpy(model.model_data->memory_block, data->buffers[0].data, data->buffers[0].size);

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


    for (int i = 0; i < model.index_count; ++i) {

    }

    for (int i = 0; i < model.vertex_count; ++i) {
        
    }

    return MODEL_LOAD_RESULT_SUCCESS;
}