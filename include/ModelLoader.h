// Handles loading model files. Currently only supports GLTF formats.
#pragma once

//#include <model_loader/cgltf.h>

#include <string>

#include <utils.h>
#include <RenderTypes.h>
#include "VulkanLoader.h"

// Returned error cases for failed/successful loading
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

// Load a GLTF Model at the given filepath
EModelLoadResult LoadGTLFModel(std::string filepath, Model_Separate_Data& model, uint32_t material_type, 
                               uint32_t shader_id, uint32_t uniform_count);
// Destroy loaded model
void DestroyModelSeparateDataTest(Model_Separate_Data *model, const VulkanInfo *vulkan_info);

// Creates a generic box. Used for initial testing 
Model_Separate_Data CreateBoxNonInterleaved(glm::vec3 pos, glm::vec3 ext, uint32_t material_type, uint32_t shader_id);
