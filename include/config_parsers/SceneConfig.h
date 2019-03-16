#pragma once

#include <stdint.h>

typedef enum {
  DIRECTIONAL
} ELightType;

struct SceneModelData {
    char* filepath;
    int id;
    float model_matrix[16]; // mat4
};

struct SceneCameraData {
    float position[3];       // vec3
    float up_vector[3];      // vec3
    float look_at_vector[3]; // vec3
    float pitch;
    float yaw;
    float roll;
    float zoom;
};

struct Color {
      float diffuse[3];  // vec3
      float specular[3]; // vec3
      float ambient[3];  // vec3
}; 

struct SceneLightData {
    ELightType light_type;
    float position[3];   // vec3
    Color color;
    float direction[4];  // vec4
};

struct SceneSettings {
   SceneModelData *model_data;
  uint32_t num_models;

  SceneCameraData *camera_data;
  uint32_t num_cameras;

  SceneLightData *light_data;
  uint32_t num_lights;
};

SceneSettings* LoadSceneSettings(char* filename);
void SaveSceneSettings(char* filename);
void FreeSceneSettings(SceneSettings* scene_settings);