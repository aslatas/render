
#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cstdlib>

// NOTE(Matt): The "static" keyword has several uses in C/C++. We define a
// keyword for each primary use, to improve clarity/searchability.
#define global static
#define internal static
#define persistent static

#include "Platform.h"
#include "Main.h"
#include "Utils.h"

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#include "VulkanFunctions.h"

#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <windowsx.h>

#define WNDCLASS_NAME L"Gerald"
#define WINDOW_TITLE L"Rendering Prototype"
#define VULKAN_LIB_PATH L"vulkan-1.dll"

struct PlatformWindow
{
    HWND handle;
    RAWINPUTDEVICE input_devices[1];
    s32 mouse_x;
    s32 mouse_y;
    s32 mouse_x_delta;
    s32 mouse_y_delta;
    bool is_minimized;
    bool is_resizing;
    bool is_visible;
    bool is_running;
    bool has_been_shown;
    PlatformResizeCallback resize_callback;
    PlatformMouseButtonCallback mouse_button_callback;
    PlatformMouseWheelCallback mouse_wheel_callback;
    PlatformKeyCallback key_callback;
};

struct PlatformGlobalTimer
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER last;
};

global Win32GlobalTimer global_timer = {};
global HMODULE vulkan_library = nullptr;
global POINT cursor_location_at_capture = {};

void PlatformCreateWindow(PlatformWindow *window)
{
    WNDCLASSEXW window_class  = {};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32WindowProc;
    window_class.cbWndExtra = sizeof (window);
    window_class.hInstance = GetModuleHandle(nullptr);
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    // TODO(Matt): App icon loading.
    window_class.hIcon = (HICON)LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED); 
    window_class.lpszClassName = WNDCLASS_NAME;
    RegisterClassExW(&window_class);
    
    window->handle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, 
                                     WNDCLASS_NAME, WINDOW_TITLE,  WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    SetWindowLongPtrW(window->handle, GWLP_USERDATA, window);
    window->is_running = true;
}

void PlatformSetupInputDevices(const PlatformWindow *window)
{
    window->input_devices[0].usUsagePage = 1; // Generic usage page. 
    window->input_devices[0].usUsage = 2; // Generic mouse usage.
    // TODO(Matt): Is this valid when the window is in the background?
    window->input_devices[0].dwFlags = RIDEV_INPUTSINK;
    window->input_devices[0].hwndTarget = window->handle;
    RegisterRawInputDevices(window->input_devices, 1, sizeof(window->input_devices[0]));
}

EKeyCode Win32TranslateKeycode(WPARAM virtual_code)
{
    switch (virtual_code) {
        case 0x41: return KEY_A;
        case 0x44: return KEY_D;
        case 0x45: return KEY_E;
        case 0x51: return KEY_Q;
        case 0x53: return KEY_S;
        case 0x57: return KEY_W;
        case VK_CONTROL: return KEY_CTRL;
    }
    return KEY_UNKOWN;
}
// TODO(Matt): I think swapchain is getting remade a bunch on startup,
// because of event mis-fires.
LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PlatformWindow *window = (PlatformWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch(message) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        }
        return 0;
        case WM_PAINT: {
            // TODO(Matt): Handle me.
        }
        return 0;
        case WM_SIZE: {
            if (!window->is_visible || window->is_resizing) {
                return 0;
            } else if (wParam == SIZE_MINIMIZED) {
                window->is_minimized = true;
                return 0;
            } else {
                if (window->resize_callback) {
                    u32 width, height;
                    if (PlatformGetWindowSize(window, &width, &height)) {
                        window->is_minimized = false;
                        window->resize_callback(width, height);
                    }
                }
            }
        }
        return 0;
        case WM_ENTERSIZEMOVE: {
            if (window->is_visible) {
                window->is_resizing = true;
            }
        }
        return 0;
        case WM_EXITSIZEMOVE: {
            if (window->is_visible) {
                window->is_resizing = false;
                if (window->resize_callback) {
                    u32 width, height;
                    if (PlatformGetWindowSize(window, &width, &height)) {
                        window->resize_callback(width, height);
                    } else {
                        window->is_minimized = true;
                    }
                }
            }
        }
        return 0;
        case WM_ERASEBKGND: {
            HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect((HDC)wParam, &rect, brush);
        }
        return 0;
        case WM_KEYDOWN: {
            if (window->key_callback) {
                window->key_callback(Win32TranslateKeyCode(wParam), (lParam & (1 << 30)) ? HELD : PRESSED);
            }
        }
        return 0;
        case WM_KEYUP: {
            if (window->key_callback) window->key_callback(Win32TranslateKeyCode(wParam), RELEASED);
            
        }
        return 0;
        case WM_LBUTTONDOWN: {
            if (window->mouse_button_callback) window->mouse_button_callback(1, PRESSED);
        }
        return 0;
        case WM_LBUTTONUP: {
            if (window->mouse_button_callback) window->mouse_button_callback(1, RELEASED);
        }
        return 0;
        case WM_RBUTTONDOWN: {
            if (window->mouse_button_callback) window->mouse_button_callback(2, PRESSED);
        }
        return 0;
        case WM_RBUTTONUP: {
            if (window->mouse_button_callback) window->mouse_button_callback(2, RELEASED);
        }
        return 0;
        case WM_MBUTTONDOWN: {
            if (window->mouse_button_callback) window->mouse_button_callback(3, PRESSED);
        }
        return 0;
        case WM_MBUTTONUP: {
            if (window->mouse_button_callback) window->mouse_button_callback(3, RELEASED);
        }
        return 0;
        case WM_XBUTTONDOWN: {
            if (window->mouse_button_callback) {
                uint32_t button = 0;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) button = 4;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2) button = 5;
                window->mouse_button_callback(button, PRESSED);
            }
        }
        return 0;
        case WM_XBUTTONUP: {
            if (window->mouse_button_callback) {
                uint32_t button = 0;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) button = 4;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2) button = 5;
                window->mouse_button_callback(button, RELEASED);
            }
        }
        return 0;
        case WM_MOUSEWHEEL: {
            if (window->mouse_wheel_callback) window->mouse_wheel_callback(GET_WHEEL_DELTA_WPARAM(wParam));
        }
        return 0;
        case WM_MOUSEMOVE: {
            window->mouse_x = (int32_t)GET_X_LPARAM(lParam);
            window->mouse_y = (int32_t)GET_Y_LPARAM(lParam);
        }
        return 0;
        case WM_INPUT: 
        {
            UINT dwSize = sizeof(RAWINPUT);
            static BYTE lpb[sizeof(RAWINPUT)];
            
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 
                            lpb, &dwSize, sizeof(RAWINPUTHEADER));
            
            RAWINPUT* raw = (RAWINPUT*)lpb;
            
            if (raw->header.dwType == RIM_TYPEMOUSE) 
            {
                window->mouse_x_delta += raw->data.mouse.lLastX;
                window->mouse_y_delta += raw->data.mouse.lLastY;
            } 
        }
        return 0;
        default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

// TODO(Matt): Correct this to process all outstanding events.
// Will require special handling of WM_PAINT.
s32 PlatformPeekEvents()
{
    MSG message = {};
    PeekMessage(&message, nullptr, 0, 0, PM_REMOVE);
    if (message.message == WM_QUIT) {
        return -1;
    }
    TranslateMessage(&message);
    DispatchMessage(&message);
    
    return 1;
}

s32 PlatformPollEvents()
{
    MSG message = {};
    // Call GetMessage, which blocks until an event is received.
    // NOTE(Matt): Unlike PeekMessage(), GetMessage() returns 0 on WM_QUIT.
    if (!GetMessage(&message, nullptr, 0, 0)) return -1;
    
    // Process the retreived message.
    TranslateMessage(&message);
    DispatchMessage(&message);
    return 1;
}

bool PlatformGetWindowSize(const PlatformWindow *window, uint32_t *width, uint32_t *height)
{
    RECT rect;
    GetClientRect(window->handle, &rect);
    if (rect.right < 0) rect.right = 0;
    if (rect.bottom < 0) rect.bottom = 0;
    *width = (uint32_t)rect.right;
    *height = (uint32_t)rect.bottom;
    return (*width > 0 && *height > 0);
}

void PlatformShowWindow(PlatformWindow *window)
{
    if (window->has_been_shown) {
        ShowWindow(window->handle, SW_SHOW);
    } else {
        ShowWindow(window->handle, SW_SHOWNORMAL);
        window->has_been_shown = true;
    }
    window->is_visible = true;
}

void PlatformHideWindow(PlatformWindow *window)
{
    ShowWindow(window->handle, SW_HIDE);
    window->is_visible = false;
}

// TODO(Matt): Remove iostream calls, replace with MessageBox error proc.
void PlatformLoadVulkanLibrary()
{
    vulkan_library = LoadLibrary(VULKAN_LIB_PATH);
    if (!vulkan_library) {
        std::cerr << "Unable to load vulkan library!" << std::endl;
        exit(EXIT_FAILURE);
    }
#define VK_EXPORTED_FUNCTION(name)                                         \
    if (!(name = (PFN_##name)GetProcAddress(vulkan_library, #name))) {         \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void PlatformFreeVulkanLibrary()
{
    FreeLibrary(vulkan_library);
}

// Actual program launch point. Calls renderer main.
// TODO(Matt): Rename Main.h/cpp and Main() to be less confusing.
int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int command_show)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(prev_instance);
    UNREFERENCED_PARAMETER(command_line);
    UNREFERENCED_PARAMETER(command_show);
    // Abstract past the platform specific launch point.
    return Main();
}

void PlatformInitializeGlobalTimer(PlatformGlobalTimer *timer)
{
    QueryPerformanceFrequency(&timer->frequency);
    QueryPerformanceCounter(&timer->start);
    timer->last = timer->start;
}

double PlatformGetTimerDelta(PlatformGlobalTimer *timer)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double delta = ((double)(now.QuadPart - timer->last.QuadPart)) / timer->frequency.QuadPart;
    timer->last = now;
    return delta;
}

double PlatformGetTimerTotalSeconds(PlatformGlobalTimer *timer)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double delta = ((double)(now.QuadPart - timer->start.QuadPart)) / timer->frequency.QuadPart;
    return delta;
}

void PlatformRegisterResizeCallback(PlatformWindow *window, PlatformResizeCallback callback)
{
    window->resize_callback = callback;
}

void PlatformRegisterKeyCallback(PlatformWindow *window, PlatformKeyCallback callback)
{
    window->key_callback = callback;
}


void PlatformRegisterMouseButtonCallback(PlatformWindow *window, PlatformMouseButtonCallback callback)
{
    window->mouse_button_callback = callback;
}

void PlatformRegisterMouseWheelCallback(PlatformWindow *window, PlatformMouseWheelCallback callback)
{
    window->mouse_wheel_callback = callback;
}

void PlatformGetCursorLocation(PlatformWindow *window, s32 *x, s32 *y)
{
    *x = window->mouse_x;
    *y = window->mouse_y;
}

void PlatformGetCursorLocationNormalized(PlatformWindow *window, float *x, float *y)
{
    u32 window_width, window_height;
    s32 window_x, window_y;
    PlatformGetWindowSize(window, &window_width, &window_height);
    PlatformGetCursorLocation(window, &window_x, &window_y);
    *x = ((float)window_x) / ((float)window_width);
    *y = ((float)window_y) / ((float)window_height);
}

void PlatformGetCursorDelta(s32 *x, s32 *y)
{
    *x = window->mouse_x_delta;
    *y = window->mouse_y_delta;
}

s32 PlatformPeekInputEvents()
{
    window_info->mouse_x_delta = 0;
    window_info->mouse_y_delta = 0;
    MSG message = {};
    s32 message_count = 0;
    while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE | PM_QS_INPUT)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
        message_count++;
        if (message.message == WM_QUIT) {
            return -message_count;
        }
    }
    return message_count;
}

void PlatformCaptureCursor(const PlatformWindow *window)
{
    RECT client_rect;
    GetClientRect(window->handle, &client_rect);
    ClipCursor(&client_rect);
    SetCapture(window->handle);
}

void PlatformReleaseCursor()
{
    ClipCursor(nullptr);
    ReleaseCapture();
}

void PlatformShowCursor()
{
    SetCursorPos(cursor_capture_location.x, cursor_capture_location.y);
    cursor_capture_location = {};
    ShowCursor(true);
}

void PlatformHideCursor()
{
    GetCursorPos(&cursor_capture_location);
    while (ShowCursor(false) > 0);
}

// TODO(Matt): Implement.
// TODO(Matt): Remove iostream calls.
void PlatformSetCursor(ECursor cursor)
{
    std::cerr << "PlatformSetCursor() doesn't do anything yet. Sorry." << std::endl;
}

#undef global
#undef internal
#undef persistent