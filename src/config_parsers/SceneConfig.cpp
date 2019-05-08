#define _CRT_SECURE_NO_WARNINGS

#define MAX_FILE_LEN 256

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

  char* buffer = (char*)malloc(size + 1);
  size_t read_size = fread(buffer, 1, size, fp);
  buffer[read_size] = '\0';
  fclose(fp);

  // Load the JSON into the struct
  Document scene_document;
  scene_document.ParseInsitu(buffer);

  // 
  SceneSettings* scene_settings = (SceneSettings*)malloc(sizeof(SceneSettings));

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
    scene_settings->model_data[i].position[0] = model_array[i]["position"].GetArray()[0].GetFloat();
    scene_settings->model_data[i].position[1] = model_array[i]["position"].GetArray()[1].GetFloat();
    scene_settings->model_data[i].position[2] = model_array[i]["position"].GetArray()[2].GetFloat();

    // scale
    scene_settings->model_data[i].scale[0] = model_array[i]["scale"].GetArray()[0].GetFloat();
    scene_settings->model_data[i].scale[1] = model_array[i]["scale"].GetArray()[1].GetFloat();
    scene_settings->model_data[i].scale[2] = model_array[i]["scale"].GetArray()[2].GetFloat();

    // rotation
    scene_settings->model_data[i].rotation[0] = model_array[i]["rotation"].GetArray()[0].GetFloat();
    scene_settings->model_data[i].rotation[1] = model_array[i]["rotation"].GetArray()[1].GetFloat();
    scene_settings->model_data[i].rotation[2] = model_array[i]["rotation"].GetArray()[2].GetFloat();
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

  free(buffer);

  return scene_settings;
}

void SaveSceneSettings(SceneSettings* scene, char* filename)
{
    Document d;
    d.SetObject();

    Value models(kArrayType);
    Value cameras(kArrayType);
    Value lights(kArrayType);

    for (uint32_t i = 0; i < scene->num_models; ++i) 
    {
        SceneModelData* model = scene->model_data + i;

        Value element(kObjectType);

        Value filepath;
        Value id;
        Value position(kArrayType);
        Value scale(kArrayType);
        Value rotation(kArrayType);

        // filename: only allow up to 256 characters
        size_t len = strnlen(model->filepath, MAX_FILE_LEN);
        filepath.SetString(model->filepath, (int)len, d.GetAllocator());

        // model id
        id.SetInt(model->id);

        // position
        position.PushBack(model->position[0], d.GetAllocator());
        position.PushBack(model->position[1], d.GetAllocator());
        position.PushBack(model->position[2], d.GetAllocator());

        // scale
        scale.PushBack(model->scale[0], d.GetAllocator());
        scale.PushBack(model->scale[1], d.GetAllocator());
        scale.PushBack(model->scale[2], d.GetAllocator());

        // position
        rotation.PushBack(model->rotation[0], d.GetAllocator());
        rotation.PushBack(model->rotation[1], d.GetAllocator());
        rotation.PushBack(model->rotation[2], d.GetAllocator());


        // Add each parameter to the element object
        element.AddMember("filepath", filepath, d.GetAllocator());
        element.AddMember("id",       id,       d.GetAllocator());
        element.AddMember("position", position, d.GetAllocator());
        element.AddMember("scale",    scale,    d.GetAllocator());
        element.AddMember("rotation", rotation, d.GetAllocator());

        // add to model array
        models.PushBack(element, d.GetAllocator());
    }
    
    // Camera Data
    for (uint32_t i = 0; i < scene->num_cameras; ++i) 
    {
        SceneCameraData* camera = scene->camera_data + i;

        Value element(kObjectType);

        Value type;
        Value position(kArrayType);
        Value up_vector(kArrayType);
        Value look_at_vector(kArrayType);
        Value pitch;
        Value yaw;
        Value roll;
        Value zoom;

        // position
        position.PushBack(camera->position[0], d.GetAllocator());
        position.PushBack(camera->position[1], d.GetAllocator());
        position.PushBack(camera->position[2], d.GetAllocator());

        // up vec
        up_vector.PushBack(camera->up_vector[0], d.GetAllocator());
        up_vector.PushBack(camera->up_vector[1], d.GetAllocator());
        up_vector.PushBack(camera->up_vector[2], d.GetAllocator());

        // look at vec
        look_at_vector.PushBack(camera->look_at_vector[0], d.GetAllocator());
        look_at_vector.PushBack(camera->look_at_vector[1], d.GetAllocator());
        look_at_vector.PushBack(camera->look_at_vector[2], d.GetAllocator());

        // pitch, yaw, roll, zoom
        pitch.SetFloat(camera->pitch);
        yaw.SetFloat(camera->yaw);
        roll.SetFloat(camera->roll);
        zoom.SetFloat(camera->zoom);

        element.AddMember("position",       position,       d.GetAllocator());
        element.AddMember("up_vector",      up_vector,      d.GetAllocator());
        element.AddMember("look_at_vector", look_at_vector, d.GetAllocator());
        element.AddMember("pitch",          pitch,          d.GetAllocator());
        element.AddMember("yaw",            yaw,            d.GetAllocator());
        element.AddMember("roll",           roll,           d.GetAllocator());
        element.AddMember("zoom",           zoom,           d.GetAllocator());

        // add to camera array
        cameras.PushBack(element, d.GetAllocator());
    }

    for (uint32_t i = 0; i < scene->num_lights; ++i) 
    {

        SceneLightData* light = scene->light_data + i;

        Value element(kObjectType);

        Value type;
        Value position(kArrayType);
        Value color(kObjectType);
        Value direction(kArrayType);

        // Set the light type
        {
            switch(light->light_type)
            {
                case DIRECTIONAL:
                {
                    char* ltype = "DIRECTIONAL\0";
                    size_t len = strnlen(ltype, MAX_FILE_LEN);
                    type.SetString(ltype, (int)len, d.GetAllocator());
                } break;
                default: 
                {
                    char* ltype = "NO_TYPE\0";
                    size_t len = strnlen(ltype, MAX_FILE_LEN);
                    type.SetString(ltype, (int)len, d.GetAllocator());
                } break;
            }
        }

        // position
        position.PushBack(light->position[0], d.GetAllocator());
        position.PushBack(light->position[1], d.GetAllocator());
        position.PushBack(light->position[2], d.GetAllocator());

        // color
        {
            Value diffuse(kArrayType);
            Value specular(kArrayType);
            Value ambient(kArrayType);

            // diffuse
            diffuse.PushBack(light->color.diffuse[0], d.GetAllocator());
            diffuse.PushBack(light->color.diffuse[1], d.GetAllocator());
            diffuse.PushBack(light->color.diffuse[2], d.GetAllocator());

            // diffuse
            specular.PushBack(light->color.specular[0], d.GetAllocator());
            specular.PushBack(light->color.specular[1], d.GetAllocator());
            specular.PushBack(light->color.specular[2], d.GetAllocator());

            // diffuse
            ambient.PushBack(light->color.ambient[0], d.GetAllocator());
            ambient.PushBack(light->color.ambient[1], d.GetAllocator());
            ambient.PushBack(light->color.ambient[2], d.GetAllocator());

            color.AddMember("diffuse",  diffuse,  d.GetAllocator());
            color.AddMember("specular", specular, d.GetAllocator());
            color.AddMember("ambient",  ambient,  d.GetAllocator());
        }

        // direction
        direction.PushBack(light->direction[0], d.GetAllocator());
        direction.PushBack(light->direction[1], d.GetAllocator());
        direction.PushBack(light->direction[2], d.GetAllocator());
        direction.PushBack(light->direction[3], d.GetAllocator());

        // Now add them to the element
        element.AddMember("type",      type,      d.GetAllocator());
        element.AddMember("position",  position,  d.GetAllocator());
        element.AddMember("color",     color,     d.GetAllocator());
        element.AddMember("direction", direction, d.GetAllocator());

        // Finally, add to the lights Object
        lights.PushBack(element, d.GetAllocator());
    }

    d.AddMember("models",  models,  d.GetAllocator());
    d.AddMember("cameras", cameras, d.GetAllocator());
    d.AddMember("lights",  lights,  d.GetAllocator());

    // Write to the specified file
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    d.Accept(writer);

    FILE* fp = fopen(filename, "w+");
    if (!fp)
    {
      printf("Failed to write to file %s!\n", filename);
    }
    fputs(buffer.GetString(), fp);
    fclose(fp);


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