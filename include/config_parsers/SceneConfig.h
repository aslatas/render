#pragma once

#include <stdint.h>

typedef enum {
  DIRECTIONAL
} ELightType;

struct SceneSettings {
  struct SceneModelData {
    char* filepath;
    long id;
    float model_matrix[16]; // mat4
  } *model_data;
  uint32_t num_models;

  struct SceneCameraData {
    float position[3];       // vec3
    float up_vector[3];      // vec3
    float look_at_vector[3]; // vec3
    float pitch;
    float yaw;
    float roll;
    float zoom;
  } *camera_data;
  uint32_t num_cameras;

  struct SceneLightData {
    ELightType light_type;
    float position[3];   // vec3
    struct Color {
      float diffuse[3];  // vec3
      float specular[3]; // vec3
      float ambient[3];  // vec3
    } color;
    float direction[4];  // vec4
  } *light_data;
  uint32_t num_lights;
};

SceneSettings* LoadSceneSettings(char* filename);
void SaveSceneSettings(char* filename);
void FreePScneeSettings(SceneSettings* scene_settings);