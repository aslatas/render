
#pragma once

#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <stdint.h>

#define WNDCLASS_NAME L"WindowClass"
#define WINDOW_TITLE L"Rendering Prototype"
#define VULKAN_LIB_PATH L"vulkan-1.dll"

// Struct for window handle and state info.
struct Win32WindowInfo
{
    HWND window_handle;
    uint32_t surface_width;
    uint32_t surface_height;
    bool is_minimized;
    bool is_resizing;
    bool is_initialized;
    void (*resize_callback)(uint32_t width, uint32_t height);
};


// Creates and shows the window. Does not start the message loop.
void Win32CreateWindow();
LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// Starts and runs the message loop.
bool Win32PollEvents();

// Gets the client area of the window. Returns false if either measure is zero (usually implies that the window is minimized).
bool Win32GetSurfaceSize(uint32_t *width, uint32_t *height);

// Registers a function to be called when the window finishes resizing.
void Win32RegisterResizeCallback(void (*resize_callback)(uint32_t width, uint32_t height));

// Shows the window, and marks initialization complete.
void Win32ShowWindow();

// Gets the window handle.
HWND Win32GetWindowHandle();

// Helpers to load vulkan library.
void Win32LoadVulkanLibrary();
void Win32FreeVulkanLibrary();