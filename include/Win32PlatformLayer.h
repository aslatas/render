
#pragma once

#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cstdlib>
// TODO(Matt): Is chrono the best (read: lightweight) timing system?
// For Windows, what about QueryPerformanceCounter()?
#include <chrono>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#include "VulkanFunctions.h"

#define WNDCLASS_NAME L"WindowClass"
#define WINDOW_TITLE L"Rendering Prototype"
#define VULKAN_LIB_PATH L"vulkan-1.dll"

// TODO(Matt): These enums aren't really platform specific. Maybe put them
// somewhere with some general defs for input callback types and whatnot.
enum EButtonState
{
    NONE, RELEASED, PRESSED, HELD
};

// TODO(Matt): These also probably don't need to be platform specific.
typedef void (*Win32ResizeCallback)(uint32_t width, uint32_t height);
// Button uses 1 as left, 2 as right, 3 as middle, 4 and 5 as thumbs.
typedef void (*Win32MouseButtonCallback)(uint32_t button, EButtonState state);
// Scroll amount is positive for away from the user, negative for towards.
typedef void (*Win32MouseWheelCallback)(int32_t amount);
// TODO(Matt): Key uses virtual key codes which are platform specific.
typedef void (*Win32KeyCallback)(uint32_t key, EButtonState state);
// TODO(Matt): Callback for double click, and mouse capture handling.

// Struct for window handle and state info.
struct Win32WindowInfo
{
    HWND window_handle;
    uint32_t surface_width;
    uint32_t surface_height;
    bool is_minimized;
    bool is_resizing;
    bool is_initialized;
    bool is_running;
    Win32ResizeCallback resize_callback;
    Win32MouseButtonCallback mouse_button_callback;
    Win32MouseWheelCallback mouse_wheel_callback;
    Win32KeyCallback key_callback;
    int32_t mouse_x;
    int32_t mouse_y;
};

// Creates and shows the window. Does not start the message loop.
void Win32CreateWindow();
LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// Runs the message loop.
bool Win32PeekEvents();
// Runs the message loop, blocking until further messages are received.
bool Win32PollEvents();
// Gets the client area of the window. Returns false if either measure is zero (usually implies that the window is minimized).
bool Win32GetSurfaceSize(uint32_t *width, uint32_t *height);

// Shows the window, and marks initialization complete.
void Win32ShowWindow();

// Gets the window handle.
HWND Win32GetWindowHandle();

// Helpers to load vulkan library.
void Win32LoadVulkanLibrary();
void Win32FreeVulkanLibrary();

// Timekeeping.
void Win32InitializeTimer();
double Win32GetTimerDelta();
double Win32GetTimerTotalSeconds();

// Input handling.
void Win32GetMousePosition(int32_t *x, int32_t *y);
void Win32RegisterResizeCallback(Win32ResizeCallback callback);
void Win32RegisterMouseButtonCallback(Win32MouseButtonCallback callback);
void Win32RegisterMouseWheelCallback(Win32MouseWheelCallback callback);
void Win32RegisterKeyCallback(Win32KeyCallback callback);