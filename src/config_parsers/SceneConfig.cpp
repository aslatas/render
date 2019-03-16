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

  size_t model_length = scene_document["models"].GetArray().Size();
  size_t cameras_length = scene_document["cameras"].GetArray().Size();
  size_t lights_length = scene_document["lights"].GetArray().Size();

  //scene_settings->camera_data = (SceneSettings->SceneCameraData*)malloc(1);



  return scene_settings;
}

void SaveSceneSettings(char* filename)
{
}

void FreePScneeSettings(SceneSettings* scene_settings)
{
}