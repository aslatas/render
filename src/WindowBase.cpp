
#include "WindowBase.h"

static Win32WindowInfo window_info = {};

void Win32CreateWindow()
{
    // TODO(Matt): App icon loading.
    window_info.window_handle = nullptr;
    WNDCLASSEXW window_class  = {};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32WindowProc;
    window_class.hInstance = GetModuleHandle(nullptr);
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    window_class.hIcon = (HICON)LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED); 
    window_class.lpszClassName = WNDCLASS_NAME;
    RegisterClassExW(&window_class);
    
    HWND window = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, 
                                  WNDCLASS_NAME, WINDOW_TITLE,  WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    window_info.window_handle = window;
}

LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        }
        return 0;
        case WM_PAINT: {
        }
        return 0;
        case WM_SIZE: {
            if (!window_info.is_initialized || window_info.is_resizing) return 0;
            if (wParam == SIZE_MINIMIZED) {
                window_info.is_minimized = true;
                return 0;
            }
            
            if (Win32GetSurfaceSize(&window_info.surface_width, &window_info.surface_height)) {
                window_info.is_minimized = false;
                window_info.resize_callback(window_info.surface_width, window_info.surface_height);
            }
        }
        return 0;
        case WM_ENTERSIZEMOVE: {
            if (window_info.is_initialized) {
                window_info.is_resizing = true;
            }
        }
        return 0;
        case WM_EXITSIZEMOVE: {
            if (window_info.is_initialized) {
                window_info.is_resizing = false;
                Win32GetSurfaceSize(&window_info.surface_width, &window_info.surface_height);
                window_info.resize_callback(window_info.surface_width, window_info.surface_height);
            }
        }
        return 0;
        case WM_ERASEBKGND: {
            HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RECT rect;
            GetClientRect(window_info.window_handle, &rect);
            FillRect((HDC)wParam, &rect, brush);
        }
        return 0;
        default:
        return DefWindowProc(window, message, wParam, lParam);
    }
}

bool Win32PollEvents()
{
    MSG message = {};
    bool should_close = GetMessage(&message, nullptr, 0, 0);
    TranslateMessage(&message);
    DispatchMessage(&message);
    return should_close;
}

bool Win32GetSurfaceSize(uint32_t *width, uint32_t *height)
{
    RECT rect;
    GetClientRect(window_info.window_handle, &rect);
    *width = rect.right;
    *height = rect.top;
    return (width > 0 && height > 0);
}

void Win32RegisterResizeCallback(void (*resize_callback)(uint32_t width, uint32_t height))
{
    window_info.resize_callback = resize_callback;
}

void Win32ShowWindow()
{
    ShowWindow(window_info.window_handle, SW_SHOW);
    window_info.is_initialized = true;
}

HWND Win32GetWindowHandle()
{
    return window_info.window_handle;
}