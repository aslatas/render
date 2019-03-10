#define CGLTF_IMPLEMENTATION
#include <model_loader/cgltf.h>
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

EModelLoadResult LoadGTLFModel(std::string filepath, Model_GLTF* model) 
{

    model = (Model_GLTF*)malloc(sizeof(Model_GLTF));

    cgltf_options options = {}; // it should auto-detect the file type
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, "resources/models/Cube/glTF/Cube.gltf", &data);

    if (result != cgltf_result_success)
      return GLTFFailType(result);
    
    const size_t length = sizeof(default_model_location) + sizeof(data->buffers->uri) + 1;
    char filebin[length] = {0};
    snprintf(filebin, sizeof(filebin), "%s%s", default_model_location, data->buffers->uri);

    // Now read the bin file 
    result = cgltf_load_buffers(&options, data, filebin);
    if (result != cgltf_result_success)
      return GLTFFailType(result);

    
    BufferDataInfo data_info;
    size_t num_attr = data->scene->nodes[0]->mesh->primitives->attributes_count; 
    size_t attr_size = 0;
    for (int i = 0; i < num_attr; ++i)
    {
        // default: cgltf_attribute_type_invalid,
        switch(data->scene->nodes[0]->mesh->primitives->attributes[i].type)
        {
            case cgltf_attribute_type_position:
            {
                data_info.pos_offset = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                // data_info.pos_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.pos_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_normal:
            {
                data_info.normal_offset = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                //  data_info.normal_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.normal_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_tangent:
            {
                data_info.tangent_offset = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                //  data_info.tangent_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.tangent_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_texcoord:
            {
                if (data_info.tex_0_format == VK_FORMAT_UNDEFINED)
                {
                    data_info.tex_0_offset = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                    attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                    //data_info.tex_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                    data_info.tex_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
                }
                else if (data_info.tex_1_format == VK_FORMAT_UNDEFINED)
                {
                    data_info.tex_1_offset = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                    attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                    //data_info.tex_1_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                    data_info.tex_1_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
                }
                else
                {
                    printf("MODEL READ ERROR: CURRENTLY DO NOT SUPPORT MORE THAN TWO TEXTURE COORDINATES\n");
                }
            } break;
            case cgltf_attribute_type_color:
            {
                data_info.color_0_offset = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                //data_info.color_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.color_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_joints:
            {
                data_info.joints_0_offset = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                //data_info.joints_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                data_info.joints_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            case cgltf_attribute_type_weights:
            {
                data_info.weights_0_offset = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->offset;
                attr_size += data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size;
                //data_info.weights_0_size = data->scene->nodes[0]->mesh->primitives->attributes[i].data->buffer_view->size; 
                data_info.weights_0_format = ConvertGLTFAttributeFormatToVulkanFormat(data->scene->nodes[0]->mesh->primitives->attributes[i].data->type);
            } break;
            default: break;
        }
    }

    void* data_buffer = data->buffers[0].data;
    //void* v_data_buffer = *data_buffer;// + data->scene->nodes[0]->mesh->primitives->indices->buffer_view->size;
    Model_GLTF temp;
    CreateDataBuffer(attr_size, data_buffer, temp.vertex_buffer, temp.vertex_buffer_memory);
    CreateDataBuffer(data->scene->nodes[0]->mesh->primitives->indices->buffer_view->size, data_buffer, model->index_buffer, model->index_buffer_memory);

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



    cgltf_free(data);

    return MODEL_LOAD_RESULT_SUCCESS;
}