// Handles loading model files. Currently only supports GLTF formats.
#pragma once

#include <string>

#include <utils.h>
#include <RenderTypes.h>
#include "VulkanLoader.h"

// Load a GLTF Model at the given filepath
EModelLoadResult LoadGTLFModel(std::string filepath, Model_GLTF& model, uint32_t material_type, 
                               uint32_t shader_id, uint32_t uniform_count);
// Destroy loaded model
// void DestroyGLTFModel(Model_GLTF *model, const VulkanInfo *vulkan_info);
