#define _CRT_SECURE_NO_WARNINGS
#include <config_parsers/SceneConfig.h>
#include <config_parsers/ConfigUtils.h>
#include "rapidjson/error/en.h"

SceneSettings* LoadSceneSettings(const char *filename)
{
  // Read from the file
  FILE *fp = fopen(filename, "r");
  if (!fp)
    return nullptr;

  // Get the length of the file
  fseek(fp, 0, SEEK_END);
  const size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* buffer = (char*)malloc(size);
  FileReadStream frs(fp, buffer, size);

  fclose(fp);

  // Load the JSON into the struct
  Document scene_document;
  scene_document.ParseStream(frs);
  if (scene_document.HasParseError()) {
    fprintf(stderr, "\nError(offset %u): %s\n", 
        (unsigned)scene_document.GetErrorOffset(),
        GetParseError_En(scene_document.GetParseError()));
    // ...
}

  // 
  SceneSettings* scene_settings = (SceneSettings*)malloc(sizeof(scene_settings));

  scene_settings->num_models  = scene_document["models"].GetArray().Size();
  scene_settings->num_cameras = scene_document["cameras"].GetArray().Size();
  scene_settings->num_lights  = scene_document["lights"].GetArray().Size();

  scene_settings->model_data  = (SceneModelData*)malloc(scene_settings->num_models * sizeof(SceneModelData));
  scene_settings->camera_data = (SceneCameraData*)malloc(scene_settings->num_cameras * sizeof(SceneCameraData));
  scene_settings->light_data  = (SceneLightData*)malloc(scene_settings->num_lights * sizeof(SceneLightData));

  Value model_array = scene_document["models"].GetArray();
  for (unsigned int i = 0; i < scene_settings->num_models; ++i)
  {
    // filename
    size_t name_len = model_array[i]["filepath"].GetStringLength();;
    scene_settings->model_data[i].filepath = (char*)malloc(name_len + 1);
    strncpy(scene_settings->model_data[i].filepath, model_array[i]["filepath"].GetString(), name_len);
    scene_settings->model_data[i].filepath[name_len] = '\0';

    // id
    scene_settings->model_data[i].id = model_array[i]["id"].GetInt();

    // position
    scene_settings->model_data[i].position[0] = model_array[i]["mposition"].GetArray()[0].GetFloat();
    scene_settings->model_data[i].position[1] = model_array[i]["mposition"].GetArray()[1].GetFloat();
    scene_settings->model_data[i].position[2] = model_array[i]["mposition"].GetArray()[2].GetFloat();

    // scale
    scene_settings->model_data[i].scale[0] = model_array[i]["mscale"].GetArray()[0].GetFloat();
    scene_settings->model_data[i].scale[1] = model_array[i]["mscale"].GetArray()[1].GetFloat();
    scene_settings->model_data[i].scale[2] = model_array[i]["mscale"].GetArray()[2].GetFloat();

    // rotation
    scene_settings->model_data[i].rotation[0] = model_array[i]["mrotation"].GetArray()[0].GetFloat();
    scene_settings->model_data[i].rotation[1] = model_array[i]["mrotation"].GetArray()[1].GetFloat();
    scene_settings->model_data[i].rotation[2] = model_array[i]["mrotation"].GetArray()[2].GetFloat();
  }

  Value camera_array = scene_document["cameras"].GetArray();
  for (unsigned int i = 0; i < scene_settings->num_cameras; ++i)
  {
    // position
    scene_settings->camera_data[i].position[0] = camera_array[i]["position"].GetArray()[0].GetFloat();
    scene_settings->camera_data[i].position[1] = camera_array[i]["position"].GetArray()[1].GetFloat();
    scene_settings->camera_data[i].position[2] = camera_array[i]["position"].GetArray()[2].GetFloat();

    // up vector
    scene_settings->camera_data[i].up_vector[0] = camera_array[i]["up_vector"].GetArray()[0].GetFloat();
    scene_settings->camera_data[i].up_vector[1] = camera_array[i]["up_vector"].GetArray()[1].GetFloat();
    scene_settings->camera_data[i].up_vector[2] = camera_array[i]["up_vector"].GetArray()[2].GetFloat();

    // look at vector
    scene_settings->camera_data[i].look_at_vector[0] = camera_array[i]["look_at_vector"].GetArray()[0].GetFloat();
    scene_settings->camera_data[i].look_at_vector[1] = camera_array[i]["look_at_vector"].GetArray()[1].GetFloat();
    scene_settings->camera_data[i].look_at_vector[2] = camera_array[i]["look_at_vector"].GetArray()[2].GetFloat();

    // pitch
    scene_settings->camera_data[i].pitch = camera_array[i]["pitch"].GetFloat();

    // yaw
    scene_settings->camera_data[i].yaw = camera_array[i]["yaw"].GetFloat();

    // roll
    scene_settings->camera_data[i].roll = camera_array[i]["roll"].GetFloat();

    // zoom
    scene_settings->camera_data[i].zoom = camera_array[i]["zoom"].GetFloat();
  }

  Value light_array = scene_document["lights"].GetArray();
  for (unsigned int i = 0; i < scene_settings->num_lights; ++i)
  {
    if (strncmp("DIRECTIONAL", light_array[i]["type"].GetString(), light_array[i]["type"].GetStringLength()) == 0)
    {
      scene_settings->light_data[i].light_type = DIRECTIONAL;

      // position
      scene_settings->light_data[i].position[0] = light_array[i]["position"].GetArray()[0].GetFloat();
      scene_settings->light_data[i].position[1] = light_array[i]["position"].GetArray()[1].GetFloat();
      scene_settings->light_data[i].position[2] = light_array[i]["position"].GetArray()[2].GetFloat();

      // color: diffuse
      scene_settings->light_data[i].color.diffuse[0] = light_array[i]["color"]["diffuse"].GetArray()[0].GetFloat();
      scene_settings->light_data[i].color.diffuse[1] = light_array[i]["color"]["diffuse"].GetArray()[1].GetFloat();
      scene_settings->light_data[i].color.diffuse[2] = light_array[i]["color"]["diffuse"].GetArray()[2].GetFloat();

      // color: specular
      scene_settings->light_data[i].color.specular[0] = light_array[i]["color"]["specular"].GetArray()[0].GetFloat();
      scene_settings->light_data[i].color.specular[1] = light_array[i]["color"]["specular"].GetArray()[1].GetFloat();
      scene_settings->light_data[i].color.specular[2] = light_array[i]["color"]["specular"].GetArray()[2].GetFloat();

      // color: ambient
      scene_settings->light_data[i].color.ambient[0] = light_array[i]["color"]["ambient"].GetArray()[0].GetFloat();
      scene_settings->light_data[i].color.ambient[1] = light_array[i]["color"]["ambient"].GetArray()[1].GetFloat();
      scene_settings->light_data[i].color.ambient[2] = light_array[i]["color"]["ambient"].GetArray()[2].GetFloat();

      // direction
      scene_settings->light_data[i].direction[0] = light_array[i]["direction"].GetArray()[0].GetFloat();
      scene_settings->light_data[i].direction[1] = light_array[i]["direction"].GetArray()[1].GetFloat();
      scene_settings->light_data[i].direction[2] = light_array[i]["direction"].GetArray()[2].GetFloat();
      scene_settings->light_data[i].direction[3] = light_array[i]["direction"].GetArray()[3].GetFloat();
    }
  }

  return scene_settings;
}

void SaveSceneSettings(char* filename)
{
}

void FreeSceneSettings(SceneSettings* scene_settings)
{
  for (unsigned int i = 0; i < scene_settings->num_models; ++i)
  {
    free(scene_settings->model_data[i].filepath);
  }

  free(scene_settings->model_data);
  free(scene_settings->camera_data);
  free(scene_settings->light_data);

  free(scene_settings);
  scene_settings = nullptr;
}