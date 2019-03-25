
#pragma once

#include "RenderBase.h"
enum EButtonState
{
    NONE, RELEASED, PRESSED, HELD
};

enum EKeyCode
{
    KEY_UNKNOWN,
    KEY_CTRL,// = VK_CONTROL,
    KEY_W,// = 0x57,
    KEY_A,// = 0x41,
    KEY_S,// = 0x53,
    KEY_D,// = 0x44,
    KEY_Q,// = 0x51,
    KEY_E,// = 0x45
};

enum EInputMode
{
    UI, VIEWPORT
};

enum ECursor
{
    ARROW, CROSS, SPINNER, CARET
};

int Main();
void Shutdown();
void ExitWithError(const char *message);
void ProcessInput();
EInputMode GetInputMode();
float GetForwardAxis();
float GetRightAxis();
float GetUpAxis();