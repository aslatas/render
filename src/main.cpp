
#include "RenderBase.h"


void RunMainLoop(WindowInfo *window_info, VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info, BufferInfo *buffer_info)
{
    MSG message = {};
    while (GetMessage(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
        if (window_info->is_initialized && !window_info->is_resizing && !window_info->is_minimized) {
            DrawFrame(window_info, vulkan_info, swapchain_info, buffer_info);
        }
    }
}

int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int command_show)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(prev_instance);
    UNREFERENCED_PARAMETER(command_line);
    
    WindowInfo window_info;
    VulkanInfo vulkan_info;
    SwapchainInfo swapchain_info;
    BufferInfo buffer_info;
    Win32CreateWindow(&window_info);
    ShowWindow(window_info.window, command_show);
    InitializeVulkan(&window_info, &vulkan_info, &swapchain_info, &buffer_info);
    RunMainLoop(&window_info, &vulkan_info, &swapchain_info, &buffer_info);
    // TODO(Matt): Should probably do some cleanup here.
    return EXIT_SUCCESS;
}
