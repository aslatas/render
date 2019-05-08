#pragma once

#include <stdint.h>

struct EngineSettings {
  struct Window {
    uint32_t width;
    uint32_t height;
  } window;
  
  struct Scene {
    char* filename;
  } scene;
};

EngineSettings* LoadEngineSettings(char* filename);
void SaveEngineSettings(char* filename);
void FreeEngineSettings(EngineSettings* engine_settings);