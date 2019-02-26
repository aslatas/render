
#include "Main.h"

#define MAX_PHYSICS_DELTA 0.03
#define MAX_PHYSICS_STEPS 4

void ResizeCallback(uint32_t width, uint32_t height)
{
    RecreateSwapchain();
}

void RunMainLoop()
{
    double frame_delta_max = MAX_PHYSICS_DELTA * MAX_PHYSICS_STEPS;
    while (Win32PollEvents()) {
        double frame_delta = Win32GetTimerDelta();
        if (frame_delta > frame_delta_max) frame_delta = frame_delta_max;
        // Pre-physics update here: Simulate(frame_delta).
        uint32_t steps = ((uint32_t)(frame_delta / MAX_PHYSICS_DELTA)) + 1;
        if (steps > MAX_PHYSICS_STEPS) steps = MAX_PHYSICS_STEPS;
        double step_delta = frame_delta / steps;
        for (uint32_t i = 0; i < steps; ++i) {
            // Physics update here: Simulate(step_delta).
        }
        // Post-physics update here: Simulate(frame_delta).
        DrawFrame();
        // Post-render update here: Simulate(frame_delta).
    }
}

int Main()
{
    Win32CreateWindow();
    Win32RegisterResizeCallback(ResizeCallback);
    InitializeVulkan();
    Win32ShowWindow();
    Win32InitializeTimer();
    RunMainLoop();
    ShutdownVulkan();
    return EXIT_SUCCESS;
}
