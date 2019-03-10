#define CGLTF_IMPLEMENTATION
#include <ModelLoader.h>

#include "RenderBase.h"

char default_model_location[] = "resources/models/Cube/glTF/";

struct {
  Vertex *vertices;
  uint32_t vertex_count;
  uint32_t *indices;
  uint32_t index_count;
} TemporaryModelData;

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

// Contain offsets/strides for attributes. Essential for creating the Attributes
struct BufferDataInfo {
    size_t index_offset = 0;
    size_t index_size = 0;
    VkFormat index_format = VK_FORMAT_UNDEFINED;

    size_t pos_offset = 0;
    size_t pos_size = 0;
    VkFormat pos_format = VK_FORMAT_UNDEFINED;
    
    size_t normal_offset = 0;
    size_t normal_size = 0;
    VkFormat normal_format = VK_FORMAT_UNDEFINED;
    
    size_t tangent_offset = 0;
    size_t tangent_size = 0;
    VkFormat tangent_format = VK_FORMAT_UNDEFINED;

    size_t tex_0_offset = 0;
    size_t tex_0_size = 0;
    VkFormat tex_0_format = VK_FORMAT_UNDEFINED;

    size_t tex_1_offset = 0;
    size_t tex_1_size = 0;
    VkFormat tex_1_format = VK_FORMAT_UNDEFINED;

    size_t color_0_offset = 0;
    size_t color_0_size = 0;
    VkFormat color_0_format = VK_FORMAT_UNDEFINED;

    size_t joints_0_offset = 0;
    size_t joints_0_size = 0;
    VkFormat joints_0_format = VK_FORMAT_UNDEFINED;

    size_t weights_0_offset = 0;
    size_t weights_0_size = 0;
    VkFormat weights_0_format = VK_FORMAT_UNDEFINED;
};

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

void DestroyGLTFModel(Model_GLTF *model, const VulkanInfo *vulkan_info)
{
    cgltf_free(model->data);
    // free(model->data);
    // free(model->indices);
    for (uint32_t i = 0; i < model->uniform_count; ++i) {
        vkDestroyBuffer(vulkan_info->logical_device, model->uniform_buffers[i], nullptr);
        vkFreeMemory(vulkan_info->logical_device, model->uniform_buffers_memory[i], nullptr);
    }
    vkDestroyBuffer(vulkan_info->logical_device, model->vertex_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, model->vertex_buffer_memory, nullptr);
    vkDestroyBuffer(vulkan_info->logical_device, model->index_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, model->index_buffer_memory, nullptr);
    free(model->uniform_buffers);
    free(model->uniform_buffers_memory);
    free(model->descriptor_sets);
    model = nullptr;
    //free(model);
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

EModelLoadResult LoadGTLFModel(std::string filepath, Model_GLTF& model, uint32_t uniform_count) 
{

    //model = (Model_GLTF*)malloc(sizeof(Model_GLTF));

    cgltf_options options = {}; // it should auto-detect the file type
    //cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, "resources/models/Cube/glTF/Cube.gltf", &model.data);

    if (result != cgltf_result_success)
      return GLTFFailType(result);
    
    const size_t length = sizeof(default_model_location) + sizeof(model.data->buffers->uri) + 1;
    char filebin[length] = {0};
    snprintf(filebin, sizeof(filebin), "%s%s", default_model_location, model.data->buffers->uri);

    // Now read the bin file 
    result = cgltf_load_buffers(&options, model.data, filebin);
    if (result != cgltf_result_success)
      return GLTFFailType(result);

    
    BufferDataInfo data_info;
    size_t num_attr = model.data->scene->nodes[0]->mesh->primitives->attributes_count; 
    size_t attr_size = 0;
    for (int i = 0; i < num_attr; ++i)
    {
        // default: cgltf_attribute_type_invalid,
        switch(model.data->scene->nodes[0]->mesh->primitives->attributes[i].type)
        {
            case cgltf_attribute_type_position:
            {
                data_info.pos_offset = model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                // data_info.pos_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                attr_size += model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.pos_format = ConvertGLTFAttributeFormatToVulkanFormat(model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_normal:
            {
                data_info.normal_offset = model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                //  data_info.normal_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                attr_size += model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.normal_format = ConvertGLTFAttributeFormatToVulkanFormat(model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_tangent:
            {
                data_info.tangent_offset = model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                //  data_info.tangent_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                attr_size += model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.tangent_format = ConvertGLTFAttributeFormatToVulkanFormat(model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_texcoord:
            {
                if (data_info.tex_0_format == VK_FORMAT_UNDEFINED)
                {
                    data_info.tex_0_offset = model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                    attr_size += model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                    //data_info.tex_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                    data_info.tex_0_format = ConvertGLTFAttributeFormatToVulkanFormat(model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
                }
                else if (data_info.tex_1_format == VK_FORMAT_UNDEFINED)
                {
                    data_info.tex_1_offset = model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                    attr_size += model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                    //data_info.tex_1_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                    data_info.tex_1_format = ConvertGLTFAttributeFormatToVulkanFormat(model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
                }
                else
                {
                    printf("MODEL READ ERROR: CURRENTLY DO NOT SUPPORT MORE THAN TWO TEXTURE COORDINATES\n");
                }
            } break;
            case cgltf_attribute_type_color:
            {
                data_info.color_0_offset = model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                //data_info.color_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.color_0_format = ConvertGLTFAttributeFormatToVulkanFormat(model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_joints:
            {
                data_info.joints_0_offset = model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                attr_size += model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                //data_info.joints_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.joints_0_format = ConvertGLTFAttributeFormatToVulkanFormat(model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_weights:
            {
                data_info.weights_0_offset = model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                attr_size += model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                //data_info.weights_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size; 
                data_info.weights_0_format = ConvertGLTFAttributeFormatToVulkanFormat(model.data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
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

    // TODO(Dustin): avoid the hardcode
    model.uniform_count = uniform_count;

    void* data_buffer = model.data->buffers[0].data;
    size_t index_size = model.data->scene->nodes[0]->mesh->primitives->indices->buffer_view->size; 
    CreateModelBuffer(attr_size, (void*)((char*)data_buffer + index_size), &model.vertex_buffer, &model.vertex_buffer_memory);
    CreateModelBuffer(index_size, data_buffer, &model.index_buffer, &model.index_buffer_memory);

    model.uniform_buffers = (VkBuffer *)malloc(sizeof(VkBuffer) * model.uniform_count);
    model.uniform_buffers_memory = (VkDeviceMemory *)malloc(sizeof(VkDeviceMemory) * model.uniform_count);
    model.descriptor_sets = (VkDescriptorSet *)malloc(sizeof(VkDescriptorSet) * model.uniform_count);

    CreateModelUniformBuffers(sizeof(UniformBufferObject), model.uniform_buffers, model.uniform_buffers_memory, model.uniform_count);
    CreateModelDescriptorSets(model.uniform_count, 0, 0, model.uniform_buffers, model.descriptor_sets);


    // Scene -> Node -> Mesh -> Primitives -> Attributes -> Data -> BufferView -> [offset, size, stride]
    // Attributes are layed out in order: indices vertices normals textcoord_0 ...and so forth
    //                                                   -> Attribute Count
    // number of attributes
    //                                                   -> Type
    // Determines the type of primitive (i.e. triangles, triangle_strip, etc.)
    //                                                   -> Indices
    // Index data for the model
    //                                                   -> Indices -> BufferView -> [offiset, size, stride (look at this again)]
    // Data for the vertices, tends to be first in the data 
    //                                                   -> Material
    // Material of this model
    //                                                   -> Material -> A bunch of information I will not worry about right now
    //                                                   -> Primitive Count
    // Don't know yet
    // 

    // After acquiring vertex data...
    // Set vertex positions
    // Set vertex normals
    // Set vertex color
    // Set uv channels 0-2

    // Create Model_GLTF struct based on passed param
    // Create VertexBuffers
    // Create IndexBuffers
    // Create UniformBuffers
    // Create DescriptorSets



    // Once I can load a single model properly...
    // Add texture support
    // Add basic material organization
    // Primitive Type read, supposedly this is defined in the model
    // Add scene reads for files with multiple objects
    // Light reading

    return MODEL_LOAD_RESULT_SUCCESS;
}