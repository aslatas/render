// Handles loading model files. Currently only supports GLTF formats.
#pragma once

#include <model_loader/cgltf.h>

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
  cgltf_data* data;

  unsigned int shader_id;
  unsigned int material_id;

  UniformBufferObject ubo;
  VkBuffer vertex_buffer;
  VkBuffer index_buffer;

  VkDeviceMemory vertex_buffer_memory;
  VkDeviceMemory index_buffer_memory;

  VkBuffer        *uniform_buffers;
  VkDeviceMemory  *uniform_buffers_memory;
  VkDescriptorSet *descriptor_sets;
  uint32_t         uniform_count;

  glm::vec3 pos;
  glm::vec3 rot;
  glm::vec3 scl;

  AxisAlignedBoundingBox bounds;
  bool hit_test_enabled;

};

// Load a GLTF Model at the given filepath
EModelLoadResult LoadGTLFModel(std::string filepath, Model_GLTF& model, EMaterialType material_type, 
                               uint32_t shader_id, uint32_t uniform_count);
// Destroy loaded model
void DestroyGLTFModel(Model_GLTF *model, const VulkanInfo *vulkan_info);
