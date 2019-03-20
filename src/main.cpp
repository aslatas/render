
#include "Main.h"
#include "VulkanInit.h"

#define MAX_PHYSICS_DELTA 0.03
#define MAX_PHYSICS_STEPS 4

// TODO(Matt): Better input mapping (array of mappings, where a mapping
// has a map of keys->axis values)
static bool is_ctrl_held = false;
static bool is_rmb_held = false;
static bool is_w_held = false;
static bool is_s_held = false;
static bool is_a_held = false;
static bool is_d_held = false;
static bool is_q_held = false;
static bool is_e_held = false;
static EditorInputMode editor_input_mode = UI;

void ResizeCallback(uint32_t width, uint32_t height)
{
    OnWindowResized();
}

void KeyCallback(KeyCode key, EButtonState state)
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

void MouseButtonCallback(uint32_t button, EButtonState state)
{
    if (button == 1 && state == PRESSED && editor_input_mode == UI) {
        int32_t x, y;
        Win32GetMousePosition(&x, &y);
        SelectObject(x, y, is_ctrl_held);
    }
    if (button == 2 && state == PRESSED) {
        is_rmb_held = true;
        editor_input_mode = VIEWPORT;
        Win32CaptureMouse();
    }
    if (button == 2 && state == RELEASED) {
        is_rmb_held = false;
        editor_input_mode = UI;
        Win32ReleaseMouse();
    }
}

void MouseWheelCallback(int32_t amount)
{
    
}

void RunMainLoop()
{
    double frame_delta_max = MAX_PHYSICS_DELTA * MAX_PHYSICS_STEPS;
    while (Win32PeekEvents()) {
        Win32ProcessInput();
        double frame_delta = Win32GetTimerDelta();
        if (frame_delta > frame_delta_max) frame_delta = frame_delta_max;
        // Pre-physics update here.
        UpdatePrePhysics(frame_delta);
        uint32_t steps = ((uint32_t)(frame_delta / MAX_PHYSICS_DELTA)) + 1;
        if (steps > MAX_PHYSICS_STEPS) steps = MAX_PHYSICS_STEPS;
        double step_delta = frame_delta / steps;
        for (uint32_t i = 0; i < steps; ++i) {
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

int Main()
{
    Win32CreateWindow();
    Win32RegisterResizeCallback(ResizeCallback);
    Win32RegisterKeyCallback(KeyCallback);
    Win32RegisterMouseButtonCallback(MouseButtonCallback);
    Win32RegisterMouseWheelCallback(MouseWheelCallback);
    InitializeRenderer();
    Win32ShowWindow();
    Win32InitializeTimer();
    RunMainLoop();
    Shutdown();
    return EXIT_SUCCESS;
}

void ExitWithError(const char *message)
{
    std::cerr << "Error in " << __FILE__<< ", line " << __LINE__ << "! message: \"" << message << "\"." << std::endl;
    exit(EXIT_FAILURE);
}

void Shutdown()
{
    ShutdownRenderer();
}

EditorInputMode GetEditorInputMode()
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
        return 1.0f;
    } else if (is_e_held && !is_q_held) {
        return -1.0f;
    } else {
        return 0.0f;
    }
}