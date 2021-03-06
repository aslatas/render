// Handles loading model files. Currently only supports GLTF formats.
#ifndef MODEL_H
//#include <model_loader/cgltf.h>

// #include <string>

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
// First param was: SceneModelData* model_data, 
// Third Param was: PerDrawUniformObject *ubo, 
EModelLoadResult LoadGTLFModel(SceneModelData* model_data, Model &model, PerDrawUniformObject *ubo,
                                u32 material_type, u32 shader_id, u32 uniform_index);
// Destroy loaded model
void DestroyModelSeparateDataTest(Model *model);

// Creates a generic box. Used for initial testing 
// Model CreateBoxNonInterleaved(glm::vec3 pos, glm::vec3 ext, PerDrawUniformObject *ubo, u32 material_type, u32 shader_id, u32 uniform_index);

#define MODEL_H
#endif