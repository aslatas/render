#define _CRT_SECURE_NO_WARNINGS
#include <config_parsers/SceneConfig.h>
#include <config_parsers/ConfigUtils.h>

SceneSettings* LoadSceneSettings(char* filename)
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
  scene_document.Parse(buffer);

  SceneSettings* scene_settings = (SceneSettings*)malloc(sizeof(scene_settings));

  size_t model_length   = scene_document["models"].GetArray().Size();
  size_t cameras_length = scene_document["cameras"].GetArray().Size();
  size_t lights_length  = scene_document["lights"].GetArray().Size();

  scene_settings->model_data  = (SceneModelData*)malloc(model_length * sizeof(SceneModelData));
  scene_settings->camera_data = (SceneCameraData*)malloc(cameras_length * sizeof(SceneCameraData));
  scene_settings->light_data  = (SceneLightData*)malloc(lights_length * sizeof(SceneLightData));

  Value model_array = scene_document["models"].GetArray();
  for (int i = 0; i < model_length; ++i)
  {
    // filename
    size_t name_len = model_array[i]["filepath"].GetStringLength();;
    scene_settings->model_data[i].filepath = (char*)malloc(name_len + 1);
    strncpy(scene_settings->model_data[i].filepath, model_array[i]["filepath"].GetString(), name_len);
    scene_settings->model_data[i].filepath[name_len] = '\0';

    // id
    scene_settings->model_data[i].id = model_array[i]["id"].GetInt();

    // model matrix
    Value model_matrix_array = model_array["model_matrix"].GetArray();
    for (int j = 0; j < model_matrix_array.Size(); ++i)
    {
      scene_settings->model_data[i].model_matrix[j] = model_matrix_array[j].GetFloat();
    }
  }

  Value camera_array = scene_document["cameras"].GetArray();
  for (int i = 0; i < cameras_length; ++i)
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
  for (int i = 0; i < lights_length; ++i)
  {
    if (strncpy("DIRECTIONAL", light_array[i]["type"].GetString(), light_array[i]["type"].GetStringLength()) == 0)
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
  for (int i = 0; i < scene_settings->num_models; ++i)
  {
    free(scene_settings->model_data[i].filepath);
  }

  free(scene_settings->model_data);
  free(scene_settings->camera_data);
  free(scene_settings->light_data);

  free(scene_settings);
  scene_settings = nullptr;
}