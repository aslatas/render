#pragma once

struct RenderSettings {
};

RenderSettings* LoadRenderSettings(char* filename);
void SaveRenderSettings(char* filename);
void FreeRenderSettings(RenderSettings* engine_settings);