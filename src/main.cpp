
#include "WindowBase.h"
#include "RenderBase.h"

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

int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int command_show)
{
    
    UNREFERENCED_PARAMETER(prev_instance);
    UNREFERENCED_PARAMETER(command_line);
    
    Win32CreateWindow();
    Win32RegisterResizeCallback(ResizeCallback);
    InitializeVulkan();
    Win32ShowWindow();
    RunMainLoop();
    ShutdownVulkan();
    return EXIT_SUCCESS;
}
