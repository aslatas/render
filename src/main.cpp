
#include "Main.h"

void ResizeCallback(uint32_t width, uint32_t height)
{
    RecreateSwapchain();
}

void RunMainLoop()
{
    while (Win32PollEvents()) {
        DrawFrame();
    }
}

int Main()
{
    Win32CreateWindow();
    Win32RegisterResizeCallback(ResizeCallback);
    InitializeVulkan();
    Win32ShowWindow();
    RunMainLoop();
    ShutdownVulkan();
    return EXIT_SUCCESS;
}
