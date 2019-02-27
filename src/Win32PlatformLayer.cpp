
#include "Win32PlatformLayer.h"
// TODO(Matt): This is all to call Main(). Maybe set up a window callback 
// to avoid this include.
#include "Main.h"

static Win32WindowInfo window_info = {};
// TODO(Matt): This should maybe go in the window struct? Maybe not.
static HMODULE vulkan_library = nullptr;
static LARGE_INTEGER timer_frequency;
static LARGE_INTEGER timer_start;
static LARGE_INTEGER timer_last;


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

// TODO(Matt): I think swapchain is getting remade a bunch on startup,
// because of event mis-fires.
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
        case WM_KEYDOWN: {
            if (window_info.key_callback) {
                window_info.key_callback((uint32_t)wParam, (lParam & (1 << 30)) ? HELD : PRESSED);
            }
        }
        return 0;
        case WM_KEYUP: {
            if (window_info.key_callback) window_info.key_callback((uint32_t)wParam, RELEASED);
            
        }
        return 0;
        case WM_LBUTTONDOWN: {
            if (window_info.mouse_button_callback) window_info.mouse_button_callback(1, PRESSED);
        }
        return 0;
        case WM_LBUTTONUP: {
            if (window_info.mouse_button_callback) window_info.mouse_button_callback(1, RELEASED);
        }
        return 0;
        case WM_RBUTTONDOWN: {
            if (window_info.mouse_button_callback) window_info.mouse_button_callback(2, PRESSED);
        }
        return 0;
        case WM_RBUTTONUP: {
            if (window_info.mouse_button_callback) window_info.mouse_button_callback(2, RELEASED);
        }
        return 0;
        case WM_MBUTTONDOWN: {
            if (window_info.mouse_button_callback) window_info.mouse_button_callback(3, PRESSED);
        }
        return 0;
        case WM_MBUTTONUP: {
            if (window_info.mouse_button_callback) window_info.mouse_button_callback(3, RELEASED);
        }
        return 0;
        case WM_XBUTTONDOWN: {
            if (window_info.mouse_button_callback) {
                uint32_t button = 0;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) button = 4;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2) button = 5;
                window_info.mouse_button_callback(button, PRESSED);
            }
        }
        return 0;
        case WM_XBUTTONUP: {
            if (window_info.mouse_button_callback) {
                uint32_t button = 0;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) button = 4;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2) button = 5;
                window_info.mouse_button_callback(button, RELEASED);
            }
        }
        return 0;
        case WM_MOUSEWHEEL: {
            if (window_info.mouse_wheel_callback) window_info.mouse_wheel_callback(GET_WHEEL_DELTA_WPARAM(wParam));
        }
        return 0;
        case WM_MOUSEMOVE: {
            window_info.mouse_x = (int32_t)GET_X_LPARAM(lParam);
            window_info.mouse_y = (int32_t)GET_Y_LPARAM(lParam);
        }
        return 0;
        default:
        return DefWindowProc(window, message, wParam, lParam);
    }
}

bool Win32PollEvents()
{
    MSG message = {};
    bool should_close = PeekMessage(&message, nullptr, 0, 0, PM_REMOVE);
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

void Win32ShowWindow()
{
    ShowWindow(window_info.window_handle, SW_SHOW);
    window_info.is_initialized = true;
}

HWND Win32GetWindowHandle()
{
    return window_info.window_handle;
}

void Win32LoadVulkanLibrary()
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

void Win32FreeVulkanLibrary()
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

void Win32InitializeTimer()
{
    QueryPerformanceFrequency(&timer_frequency);
    QueryPerformanceCounter(&timer_start);
    timer_last = timer_start;
}

double Win32GetTimerDelta()
{
    LARGE_INTEGER timer_now;
    QueryPerformanceCounter(&timer_now);
    double delta = ((double)(timer_now.QuadPart - timer_last.QuadPart)) / timer_frequency.QuadPart;
    timer_last = timer_now;
    return delta;
}

double Win32GetTimerTotalSeconds()
{
    LARGE_INTEGER timer_now;
    QueryPerformanceCounter(&timer_now);
    double delta = ((double)(timer_now.QuadPart - timer_start.QuadPart)) / timer_frequency.QuadPart;
    return delta;
}

void Win32RegisterResizeCallback(Win32ResizeCallback callback)
{
    window_info.resize_callback = callback;
}

void Win32RegisterKeyCallback(Win32KeyCallback callback)
{
    window_info.key_callback = callback;
}


void Win32RegisterMouseButtonCallback(Win32MouseButtonCallback callback)
{
    window_info.mouse_button_callback = callback;
}

void Win32RegisterMouseWheelCallback(Win32MouseWheelCallback callback)
{
    window_info.mouse_wheel_callback = callback;
}

void Win32GetMousePosition(int32_t *x, int32_t *y)
{
    *x = window_info.mouse_x;
    *y = window_info.mouse_y;
}