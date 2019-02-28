
#include <model_loader/cgltf.h>
#include <ModelLoader.h>

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
    else if (result == gltf_result_out_of_memory)
        return MODEL_LOAD_RESULT_OUT_OF_MEMORY;
    else 
        return MODEL_LOAD_RESULT_UNKNOWN_ERROR;
}

EModelLoadResult LoadGTLFModel(std::string filepath, Model_GLTF* model) 
{

    cgltf_options options = {0}; // it should auto-detect the file type
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, "resources/models/Box/glTF-Binary/Box.glb", &data);
    if (result != cgltf_result_success)
      return GLTFFailType(result);
    
    //Vertex* v = (Vertex *)malloc(model.vertex_count * sizeof(Vertex));

    cgltf_size buffer_count = data->buffer_size; // should return the number of buffers present
    cgltf_data* data[buffer_count];

    // I assume what is stored in this is the name of the file that contains the file data
    for (int i = 0; i < buffers_count; ++i)
    {
        printf("Buffer: %s", data->buffers);
    }

    // Maybe parse buffer view? It contains 
    printf("Buffer view count: %d", data->buffer_views_count);

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



    cgltf_free(data)

    return MODEL_LOAD_RESULT_SUCCESS;
}