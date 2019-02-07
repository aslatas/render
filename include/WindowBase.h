
#pragma once
#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <cstdint>
#define WNDCLASS_NAME L"WindowClass"
#define WINDOW_TITLE L"Rendering Prototype"

// Holds the window handle and other app properties.
struct WindowInfo
{
    HWND window;
    bool is_initialized;
    bool is_minimized;
    bool is_resizing;
};

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

void Win32CreateWindow(WindowInfo *window_info);

// Gets the width/height of the client area. Returns false if either dimension is zero, or true otherwise.
bool Win32GetClientRect(WindowInfo *window_info, uint32_t *width, uint32_t *height);