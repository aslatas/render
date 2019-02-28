// Handles loading model files. Currently only supports GLTF formats.

#include <string>

#include <utils.h>
#include <RenderTypes.h>
#include "VulkanLoader.h"
 
// Returned error cases for failed loading
typedef enum EModelLoadResult {
  MODEL_LOAD_RESULT_SUCCESS,
  MODEL_LOAD_RESULT_DATA_TOO_SHORT,
  MODEL_LOAD_RESULT_INVALID_JSON,
  MODEL_LOAD_RESULT_INVALID_GLTF,
  MODEL_LOAD_RESULT_INVALID_CGLTF_OPTIONS,
  MODEL_LOAD_RESULT_FILE_NOT_FOUND,
  MODEL_LOAD_RESULT_IO_ERROR,
  MODEL_LOAD_RESULT_OUT_OF_MEMORY,
  MODEL_LOAD_RESULT_UNKNOWN_FORMAT,
  MODEL_LOAD_RESULT_UNKNOWN_ERROR
} EModelLoadResult;

struct Model_GLTF {
  unsigned int shader_id;
  unsigned int material_id;

  UniformBufferObject ubo;
  VkBuffer vertex_buffer;
  VkBuffer index_buffer;

  VkDeviceMemory vertex_buffer_memory;
  VkDeviceMemory index_buffer_memory;

  VkBuffer *uniform_buffers;
  VkDeviceMemory *uniform_buffer_memory;
};

// Load a GLTF Model at the given filepath
EModelLoadResult LoadGTLFModel(std::string filepath, Model_GLTF* model);