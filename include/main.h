
#pragma once

#include "RenderBase.h"

enum EditorInputMode
{
    UI, VIEWPORT
};

int Main();
void Shutdown();
void ExitWithError(const char *message);
void ProcessInput();
EditorInputMode GetEditorInputMode();
float GetForwardAxis();
float GetRightAxis();
float GetUpAxis();