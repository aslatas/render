
#include "Main.h"

#define MAX_PHYSICS_DELTA 0.03
#define MAX_PHYSICS_STEPS 4

// TODO(Matt): Better input mapping (array of mappings, where a mapping
// has a map of keys->axis values)
global bool is_ctrl_held = false;
global bool is_rmb_held = false;
global bool is_w_held = false;
global bool is_s_held = false;
global bool is_a_held = false;
global bool is_d_held = false;
global bool is_q_held = false;
global bool is_e_held = false;
global float speed_multiplier = 1.0f;
global EInputMode editor_input_mode = UI;

global PlatformWindow global_window;
global PlatformGlobalTimer global_timer;

internal void ResizeCallback(const PlatformWindow *window, u32 width, u32 height)
{
    OnWindowResized();
}

internal void KeyCallback(const PlatformWindow *window, EKeyCode key, EButtonState state)
{
    if (key == KEY_CTRL && state == PRESSED) is_ctrl_held = true; 
    if (key == KEY_CTRL && state == RELEASED) is_ctrl_held = false; 
    if (key == KEY_W && state == PRESSED) is_w_held = true;
    if (key == KEY_W && state == RELEASED) is_w_held = false;
    if (key == KEY_S && state == PRESSED) is_s_held = true;
    if (key == KEY_S && state == RELEASED) is_s_held = false;
    if (key == KEY_A && state == PRESSED) is_a_held = true;
    if (key == KEY_A && state == RELEASED) is_a_held = false;
    if (key == KEY_D && state == PRESSED) is_d_held = true;
    if (key == KEY_D && state == RELEASED) is_d_held = false;
    if (key == KEY_Q && state == PRESSED) is_q_held = true;
    if (key == KEY_Q && state == RELEASED) is_q_held = false;
    if (key == KEY_E && state == PRESSED) is_e_held = true;
    if (key == KEY_E && state == RELEASED) is_e_held = false;
}

internal void MouseButtonCallback(const PlatformWindow *window, u32 button, EButtonState state)
{
    if (button == 1 && state == PRESSED && editor_input_mode == UI) {
        s32 x, y;
        PlatformGetCursorLocation(window, &x, &y);
        SelectObject(x, y, is_ctrl_held);
    }
    if (button == 2 && state == PRESSED) {
        is_rmb_held = true;
        editor_input_mode = VIEWPORT;
        PlatformCaptureCursor(window);
        PlatformHideCursor();
    }
    if (button == 2 && state == RELEASED) {
        is_rmb_held = false;
        editor_input_mode = UI;
        PlatformReleaseCursor();
        PlatformShowCursor();
    }
}

internal void MouseWheelCallback(const PlatformWindow *window, s32 amount)
{
    speed_multiplier *= 1 + ((float)amount / 720.0f);
    if (speed_multiplier < 0.125f) speed_multiplier = 0.125f;
}

void RunMainLoop()
{
    double frame_delta_max = MAX_PHYSICS_DELTA * MAX_PHYSICS_STEPS;
    while (PlatformPeekEvents() >= 0) {
        double frame_delta = PlatformGetTimerDelta(&global_timer);
        if (frame_delta > frame_delta_max) frame_delta = frame_delta_max;
        // Pre-physics update here.
        UpdatePrePhysics(frame_delta);
        u32 steps = ((u32)(frame_delta / MAX_PHYSICS_DELTA)) + 1;
        if (steps > MAX_PHYSICS_STEPS) steps = MAX_PHYSICS_STEPS;
        double step_delta = frame_delta / steps;
        for (u32 i = 0; i < steps; ++i) {
            // Physics update here (expensive, multiple steps per frame).
            UpdatePhysics(step_delta);
        }
        // Post-physics update here.
        UpdatePostPhysics(frame_delta);
        DrawFrame();
        // Post-render update.
        UpdatePostRender(frame_delta);
    }
}

s32 Main()
{
    PlatformCreateWindow(&global_window);
    PlatformSetupInputDevices(&global_window);
    PlatformRegisterResizeCallback(&global_window, ResizeCallback);
    PlatformRegisterKeyCallback(&global_window, KeyCallback);
    PlatformRegisterMouseButtonCallback(&global_window, MouseButtonCallback);
    PlatformRegisterMouseWheelCallback(&global_window, MouseWheelCallback);
    InitializeRenderer();
    PlatformShowWindow(&global_window);
    PlatformInitializeGlobalTimer(&global_timer);
    RunMainLoop();
    Shutdown();
    return EXIT_SUCCESS;
}

void ExitWithError(const char *message)
{
    char buffer[512];
    snprintf(buffer, 512, "Error in %s, line %d!\nMessage: \"%s\".", __FILE__, __LINE__, message);
    PlatformShowErrorDialog(buffer);
    exit(EXIT_FAILURE);
}

void Shutdown()
{
    ShutdownRenderer();
}

EInputMode GetInputMode()
{
    return editor_input_mode;
}

float GetForwardAxis()
{
    if (is_w_held && !is_s_held) {
        return 1.0f;
    } else if (is_s_held && !is_w_held) {
        return -1.0f;
    } else {
        return 0.0f;
    }
}

float GetRightAxis()
{
    if (is_d_held && !is_a_held) {
        return 1.0f;
    } else if (is_a_held && !is_d_held) {
        return -1.0f;
    } else {
        return 0.0f;
    }
}

float GetUpAxis()
{
    if (is_q_held && !is_e_held) {
        return -1.0f;
    } else if (is_e_held && !is_q_held) {
        return 1.0f;
    } else {
        return 0.0f;
    }
}

float GetSpeedMultiplier()
{
    return speed_multiplier;
}