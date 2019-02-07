
#include "WindowBase.h"



LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
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
            //if (render_info.is_initialized && !render_info.is_resizing) {
            //RecreateSwapchain();
            //}
        }
        return 0;
        case WM_ENTERSIZEMOVE: {
            //if (render_info.is_initialized) {
            //render_info.is_resizing = true;
            //}
        }
        return 0;
        case WM_EXITSIZEMOVE: {
            //if (render_info.is_initialized) {
            //render_info.is_resizing = false;
            //RecreateSwapchain();
            //}
        }
        return 0;
        case WM_ERASEBKGND: {
            HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RECT rect;
            GetClientRect(window, &rect);
            FillRect((HDC)wParam, &rect, brush);
        }
        return 0;
        default:
        return DefWindowProc(window, message, wParam, lParam);
    }
}


void Win32CreateWindow(WindowInfo *window_info)
{
    // TODO(Matt): App icon loading.
    WNDCLASSEXW window_class  = {};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = GetModuleHandle(nullptr);
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    window_class.hIcon = (HICON)LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED); 
    window_class.lpszClassName = WNDCLASS_NAME;
    RegisterClassExW(&window_class);
    
    (*window_info).window = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, 
                                            WNDCLASS_NAME, WINDOW_TITLE,  WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    (*window_info).is_initialized = false;
    (*window_info).is_minimized = false;
    (*window_info).is_resizing = false;
}

// Gets the width/height of the client area. Returns false if either dimension is zero, or true otherwise.
bool Win32GetClientRect(WindowInfo *window_info, uint32_t *width, uint32_t *height)
{
    RECT rect;
    GetClientRect(window_info->window, &rect);
    *width = (uint32_t)rect.right;
    *height = (uint32_t)rect.bottom;
    if (width <= 0 || height <= 0) return false;
    return true;
}