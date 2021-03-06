// TODO(Matt): Fullscreen support.
// TODO(Matt): Support minimze, maximize, restore via function call.
// TODO(Matt): Double click callback, or let the application handle that?
// TODO(Matt): Add functions for screen-space and NDC cursor location (GetCursorPos()).

#define UNICODE
#define _CRT_SECURE_NO_WARNINGS

// Standard library dependencies.
#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <fstream>
#include <iomanip>
#include <string.h>

// External libraries.
#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#pragma warning(push, 0)
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/norm.hpp"
#pragma warning(pop)

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include "stb/stb_ds.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "tinygltf/tiny_gltf.h"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

using namespace rapidjson;

// NOTE(Matt): The "static" keyword has several uses in C/C++. We define a
// keyword for each primary use, for clarity/searchability.
#define global static
#define internal static
#define persistent static

// Config Parser Component headers
#include "config_parsers/ConfigUtils.h"
#include "config_parsers/EngineConfig.h"
#include "config_parsers/PhysicsConfig.h"
#include "config_parsers/RenderConfig.h"
#include "config_parsers/SceneConfig.h"

// Component headers.
#include "Utils.h"
#include "VulkanFunctions.h"
#include "VulkanLoader.h"
#include "Common.h"
#include "Platform.h"
#include "Camera.h"
#include "Bounds.h"
#include "Object.h"
#include "RenderTypes.h"
#include "RenderBase.h"
#include "Texture.h"
#include "Font.h"
#include "Main.h"
#include "VulkanInit.h"
#include "OctTree.h"
#include "ModelLoader.h"
#include "SceneManager.h"

// Platform specific headers.
#include "Win32PlatformLayer.h"

// Config component source files (unity build).
#include "config_parsers/EngineConfig.cpp"
#include "config_parsers/PhysicsConfig.cpp"
#include "config_parsers/RenderConfig.cpp"
#include "config_parsers/SceneConfig.cpp"

// Component source files (unity build).
#include "VulkanFunctions.cpp"
#include "VulkanLoader.cpp"
#include "Camera.cpp"
#include "Object.cpp"
#include "RenderTypes.cpp"
#include "RenderBase.cpp"
#include "Texture.cpp"
#include "Font.cpp"
#include "Main.cpp"
#include "VulkanInit.cpp"
#include "Bounds.cpp"
#include "OctTree.cpp"
#include "ModelLoader.cpp"
#include "SceneManager.cpp"

// Platform specific libraries (windows headers).
#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#define WNDCLASS_NAME L"Gerald"
#define WINDOW_TITLE L"Rendering Prototype"
#define VULKAN_LIB_PATH L"vulkan-1.dll"
#define DIRECTORY_CHANGE_BUFFER_SIZE 4096
// TODO(Matt): No idea if this thread system will work. Just experimenting.
global PlatformThread io_thread;

// Dynamically loaded vulkan library.
global HMODULE vulkan_library = nullptr;

// Raw cursor movement.
// TODO(Matt): We should support a cursor delta pair per device (since we use
// RAWINPUT, we can support multiple cursors).
global s32 cursor_delta_x;
global s32 cursor_delta_y;

// Screen (not window) location of the cursor at the last capture. 
global POINT cursor_capture_location = {};

void PlatformCreateWindow(PlatformWindow *window)
{
    // Create the window class, 
    WNDCLASSEXW window_class  = {};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    // TODO(Matt): For multi-window, should each window get it's own DC?
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32WindowProc;
    // Leave space for a userdata pointer with the HWND.
    window_class.cbWndExtra = sizeof(window);
    window_class.hInstance = GetModuleHandle(nullptr);
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    // TODO(Matt): App icon loading.
    window_class.hIcon = (HICON)LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED); 
    window_class.lpszClassName = WNDCLASS_NAME;
    RegisterClassExW(&window_class);
    
    window->handle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, 
                                     WNDCLASS_NAME, WINDOW_TITLE,  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    
    // Pack the window struct pointer with the new window handle.
    SetWindowLongPtrW(window->handle, GWLP_USERDATA, (LONG_PTR)window);
}

void PlatformSetupInputDevices(PlatformWindow *window)
{
    // Configure RAWINPUT struct for the mouse.
    // TODO(Matt): Is RIDEV_INPUTSINK still going to be feeding input when
    // the window is in the background?
    window->input_devices[0].usUsagePage = 1; // Generic usage page. 
    window->input_devices[0].usUsage = 2; // Generic mouse usage.
    window->input_devices[0].dwFlags = RIDEV_INPUTSINK;
    window->input_devices[0].hwndTarget = window->handle;
    
    // Register the mouse for raw input.
    RegisterRawInputDevices(window->input_devices, 1, sizeof(window->input_devices[0]));
}

// Translates a VK keycode into a platform-independent code.
internal EKeyCode Win32TranslateKeycode(WPARAM virtual_code)
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
    // TODO(Matt): Maybe log when this case happens (once we feel like we're
    // handling all keys).
    return KEY_UNKNOWN;
}

// TODO(Matt): I think swapchain is getting remade a bunch on startup,
// because of event mis-fires.
// Window message handling procedure. 
LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Extract the platform window struct from the HWND userdata.
    PlatformWindow *window = (PlatformWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch(message) {
        // Requests a quit.
        case WM_DESTROY: {
            PostQuitMessage(0);
        }
        return 0;
        // OS request for repaint.
        case WM_PAINT: {
            // Return the whole update region as validated, so the message can be removed.
            // The whole client rect is redrawn anyway.
            RECT rect;
            GetClientRect(hwnd, &rect);
            ValidateRect(hwnd, &rect);
        }
        return 0;
        // Called when window is being resized, including (obnoxiously)
        // on fullscreen/minimize, and also during init.
        case WM_SIZE: {
            // Do nothing if hidden or mid-resize (handled on enter/exit).
            if (!window->is_visible || window->is_resizing) {
                return 0;
                // Detect if the window is now minimized.
            } else if (wParam == SIZE_MINIMIZED) {
                window->is_minimized = true;
                return 0;
                // Otherwise, the window has been restored (or fullscreen).
            } else {
                // If the resize callback is valid, execute it.
                if (window->resize_callback) {
                    u32 width, height;
                    if (PlatformGetWindowSize(window, &width, &height)) {
                        window->is_minimized = false;
                        window->resize_callback(window, width, height);
                    }
                }
            }
        }
        return 0;
        // Called when a user resize starts.
        case WM_ENTERSIZEMOVE: {
            // So long as the window is visible, mark it as resizing.
            if (window->is_visible) {
                window->is_resizing = true;
            }
        }
        return 0;
        // Called when a user resize ends.
        case WM_EXITSIZEMOVE: {
            // If the window is visible, end the resize.
            if (window->is_visible) {
                window->is_resizing = false;
                // If the resize callback is valid, execute it.
                if (window->resize_callback) {
                    u32 width, height;
                    if (PlatformGetWindowSize(window, &width, &height)) {
                        window->resize_callback(window, width, height);
                        // If width or height is zero, mark as minimized.
                    } else {
                        window->is_minimized = true;
                    }
                }
            }
        }
        return 0;
        // Called to clear the client surface. Erases to black.
        case WM_ERASEBKGND: {
            HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect((HDC)wParam, &rect, brush);
        }
        return 0;
        // TODO(Matt): Most of these inputs could likely fall through, to
        // be processed all at once. Maybe refactor to reduce duplication.
        case WM_KEYDOWN: {
            if (window->key_callback) {
                window->key_callback(window, Win32TranslateKeycode(wParam), (lParam & (1 << 30)) ? HELD : PRESSED);
            }
        }
        return 0;
        case WM_KEYUP: {
            if (window->key_callback) {
                window->key_callback(window, Win32TranslateKeycode(wParam), RELEASED);
            }
            
        }
        return 0;
        case WM_LBUTTONDOWN: {
            if (window->mouse_button_callback) window->mouse_button_callback(window, 1, PRESSED);
        }
        return 0;
        case WM_LBUTTONUP: {
            if (window->mouse_button_callback) window->mouse_button_callback(window, 1, RELEASED);
        }
        return 0;
        case WM_RBUTTONDOWN: {
            if (window->mouse_button_callback) window->mouse_button_callback(window, 2, PRESSED);
        }
        return 0;
        case WM_RBUTTONUP: {
            if (window->mouse_button_callback) window->mouse_button_callback(window, 2, RELEASED);
        }
        return 0;
        case WM_MBUTTONDOWN: {
            if (window->mouse_button_callback) window->mouse_button_callback(window, 3, PRESSED);
        }
        return 0;
        case WM_MBUTTONUP: {
            if (window->mouse_button_callback) window->mouse_button_callback(window, 3, RELEASED);
        }
        return 0;
        case WM_XBUTTONDOWN: {
            if (window->mouse_button_callback) {
                u32 button = 0;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) button = 4;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2) button = 5;
                window->mouse_button_callback(window, button, PRESSED);
            }
        }
        return 0;
        case WM_XBUTTONUP: {
            if (window->mouse_button_callback) {
                u32 button = 0;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) button = 4;
                if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2) button = 5;
                window->mouse_button_callback(window, button, RELEASED);
            }
        }
        return 0;
        case WM_MOUSEWHEEL: {
            if (window->mouse_wheel_callback) window->mouse_wheel_callback(window, GET_WHEEL_DELTA_WPARAM(wParam));
        }
        return 0;
        // Handles mouse movement in window space.
        case WM_MOUSEMOVE: {
            window->mouse_x = (s32)GET_X_LPARAM(lParam);
            window->mouse_y = (s32)GET_Y_LPARAM(lParam);
        }
        return 0;
        // Handles raw input (mouse delta).
        // NOTE(Matt): Raw input is unreliable for anything other than
        // relative movement.
        case WM_INPUT: 
        {
            // NOTE(Matt): RAWINPUT is 40 bytes on 32 bit windows, but not
            // on 64 bit windows. MSDN does not mention this, which is cool.
            UINT dwSize = sizeof(RAWINPUT);
            static BYTE lpb[sizeof(RAWINPUT)];
            
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 
                            lpb, &dwSize, sizeof(RAWINPUTHEADER));
            
            RAWINPUT* raw = (RAWINPUT*)lpb;
            
            if (raw->header.dwType == RIM_TYPEMOUSE) 
            {
                cursor_delta_x += raw->data.mouse.lLastX;
                cursor_delta_y += raw->data.mouse.lLastY;
            } 
        }
        return 0;
        // For unhandled cases, return the default window procedure result.
        default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

s32 PlatformPeekEvents()
{
    // Reset cursor delta, to be accumulated this frame by WM_INPUT.
    cursor_delta_x = 0;
    cursor_delta_y = 0;
    
    MSG message = {};
    s32 message_count = 0;
    // Process any outstanding events.
    while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
        message_count++;
        if (message.message == WM_QUIT) {
            return -message_count;
        }
    }
    return message_count;
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

bool PlatformGetWindowSize(const PlatformWindow *window, u32 *width, u32 *height)
{
    RECT rect;
    GetClientRect(window->handle, &rect);
    if (rect.right < 0) rect.right = 0;
    if (rect.bottom < 0) rect.bottom = 0;
    *width = (u32)rect.right;
    *height = (u32)rect.bottom;
    return (*width > 0 && *height > 0);
}

void PlatformShowWindow(PlatformWindow *window)
{
    // NOTE(Matt): If this is the first time showing the window, we use
    // SW_SHOWNORMAL. Otherwise, we use SW_SHOW. SW_SHOWNORMAL doesn't
    // respect existing size/location, but must be used on the first call.
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
    // NOTE(Matt): This doesn't minimize the window, it just puts it at
    // the back of the Z-order.
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
    
    // Vulkan loader macro, loads vulkan functions that are directly
    // exported by the DLL.
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

// TODO(Matt): Temporary.
bool wait_for_changes = true;
void Win32OnDirectoryChanged(DWORD error_code, DWORD change_size, LPOVERLAPPED overlapped)
{
    Win32DirectoryWatcher *watcher = (Win32DirectoryWatcher *)overlapped->hEvent;
    if (error_code == ERROR_OPERATION_ABORTED) {
        printf("Shutting down directory watcher...\n");
        CloseHandle(watcher->directory_handle);
        free(watcher->buffers[0]);
        free(watcher->buffers[1]);
        free(overlapped);
        wait_for_changes = false;
    } else if (change_size == 0) {
        printf("Got a directory change, but with size zero. We probably missed some changes.\n");
        printf("This should only happen if many changes occur at once.\n");
    } else {
        u32 current_index = watcher->buffer_index;
        watcher->buffer_index = (current_index + 1) % 2;
        BOOL result = ReadDirectoryChangesW(watcher->directory_handle, watcher->buffers[watcher->buffer_index], DIRECTORY_CHANGE_BUFFER_SIZE, watcher->watch_subdirectories, watcher->notify_flags, nullptr, overlapped, Win32OnDirectoryChanged);
        BYTE *current_offset = watcher->buffers[current_index];
        for(;;) {
            FILE_NOTIFY_INFORMATION *changes = (FILE_NOTIFY_INFORMATION *)current_offset;
            switch (changes->Action) {
                case FILE_ACTION_ADDED: {
                    printf("File added: ");
                }
                break;
                case FILE_ACTION_REMOVED: {
                    printf("File removed: ");
                }
                break;
                case FILE_ACTION_MODIFIED: {
                    printf("File modified: ");
                }
                break;
                case FILE_ACTION_RENAMED_OLD_NAME: {
                    printf("File renamed from: ");
                }
                break;
                case FILE_ACTION_RENAMED_NEW_NAME: {
                    printf("to: ");
                }
                break;
            }
            
            // TODO(Matt): This doesn't support using wide characters in the directory or file names.
            u32 size = (changes->FileNameLength / sizeof(wchar_t)) + 1;
            char *file_name = (char *)malloc(size);
            s32 char_count = (s32)wcstombs(file_name, changes->FileName, size - 1);
            if (char_count < 0) {
                printf("contained non-ANSI characters, skipping.\n");
            } else {
                file_name[char_count] = '\0';
                char *relative_path = (char *)malloc(strlen(file_name) + strlen(watcher->directory_path) + 2);
                sprintf(relative_path, "%s/%s", watcher->directory_path, file_name);
                printf("%s\n", relative_path);
                FILE *file = fopen(relative_path, "rb");
                if (file) {
                    printf("File opened for binary read! radical!\n");
                    fclose(file);
                }
                free(relative_path);
            }
            
            free(file_name);
            if (changes->NextEntryOffset) {
                current_offset += changes->NextEntryOffset;
            } else {
                break;
            }
        }
        
        // TODO(Matt): Probably should collect all of the changes for one pass, then process them here.
    }
}

// TODO(Matt): Temporary.
DWORD WINAPI ThreadProc(LPVOID param)
{
    Win32DirectoryWatcher watcher = {};
    watcher.directory_path = "shaders";
    watcher.directory_handle = CreateFileA("shaders", FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
    if (watcher.directory_handle == INVALID_HANDLE_VALUE) return -1;
    
    // Allocate out two 4k buffers. We use two of them so we can kick off another request before processing the
    // last one. It's possible that we get more than 4k worth of changes, in which case we'll just pretend
    // we got NO changes.
    watcher.buffers[0] = (BYTE *)malloc(DIRECTORY_CHANGE_BUFFER_SIZE);
    watcher.buffers[1] = (BYTE *)malloc(DIRECTORY_CHANGE_BUFFER_SIZE);
    watcher.notify_flags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
    watcher.watch_subdirectories = false;
    
    OVERLAPPED *overlapped = (OVERLAPPED *)malloc(sizeof(OVERLAPPED));
    *overlapped = {};
    overlapped->hEvent = &watcher; // Pass the directory watcher with the overlapped struct,
    // because the hEvent member isn't used by the sytem (so we can use it instead).
    BOOL result = ReadDirectoryChangesW(watcher.directory_handle, watcher.buffers[0], DIRECTORY_CHANGE_BUFFER_SIZE, watcher.watch_subdirectories, watcher.notify_flags, nullptr, overlapped, Win32OnDirectoryChanged);
    // Could use WaitForMultipleObjectsEx if we want more notifications.
    //CancelIo(dir_handle);
    while (wait_for_changes) {
        SleepEx(INFINITE, TRUE);
    }
    printf("Done waiting for changes.\n");
    return 0;
}

// Program entry  point. Calls renderer main.
// TODO(Matt): Rename Main.h/cpp and Main() to be less confusing.
int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int command_show)
{
    // We don't actually care about these parameters.
    // Maybe someday we will use some command-line arguments.
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(prev_instance);
    UNREFERENCED_PARAMETER(command_line);
    UNREFERENCED_PARAMETER(command_show);
    
    // TODO(Matt): Temporary.
    io_thread.type = IO;
    io_thread.handle = CreateThread(0, 0, ThreadProc, nullptr, 0, &io_thread.id);
    // Abstract past the platform specific launch point.
    return Main();
}

void PlatformInitializeGlobalTimer(PlatformGlobalTimer *timer)
{
    // Get the frequency, and take the first timer measurement.
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

void PlatformGetCursorLocation(const PlatformWindow *window, s32 *x, s32 *y)
{
    *x = window->mouse_x;
    *y = window->mouse_y;
}

void PlatformGetCursorLocationNormalized(PlatformWindow *window, float *x, float *y)
{
    // Normalize cursor x/y in window space - not NDC.
    // NOTE(Matt): "Normalized" cursor location can be less than zero or
    // greater than one if the cursor is outside the window.
    u32 window_width, window_height;
    s32 window_x, window_y;
    PlatformGetWindowSize(window, &window_width, &window_height);
    PlatformGetCursorLocation(window, &window_x, &window_y);
    *x = ((float)window_x) / ((float)window_width);
    *y = ((float)window_y) / ((float)window_height);
}

void PlatformGetCursorDelta(s32 *x, s32 *y)
{
    *x = cursor_delta_x;
    *y = cursor_delta_y;
}

void PlatformCaptureCursor(const PlatformWindow *window)
{
    RECT client_rect;
    GetClientRect(window->handle, &client_rect);
    ClipCursor(&client_rect);
    // NOTE(Matt): This can fail if cursor isn't in the capturing window.
    SetCapture(window->handle);
}

void PlatformReleaseCursor()
{
    ClipCursor(nullptr);
    ReleaseCapture();
}

void PlatformShowCursor()
{
    // TODO(Matt): Duplicate calls to PlatformShowCursor() will set cursor
    // location to (0, 0). We should only perform the move/reset if the
    // cursor was actually hidden. Maybe check the result of ShowCursor()?
    SetCursorPos(cursor_capture_location.x, cursor_capture_location.y);
    cursor_capture_location = {};
    ShowCursor(true);
}

void PlatformHideCursor()
{
    GetCursorPos(&cursor_capture_location);
    // ShowCursor(false) only decrements the "hidden count" by one. The
    // count can be > 1, so we must decrement it all the way to zero.
    while (ShowCursor(false) > 0);
}

// TODO(Matt): Implement.
// TODO(Matt): Remove iostream calls.
void PlatformSetCursor(ECursor cursor)
{
    fprintf(stderr, "PlatformSetCursor() doesn't do anything yet. Sorry.\n");
}

VkSurfaceKHR PlatformCreateSurface(const PlatformWindow *window, VkInstance instance)
{
    VkSurfaceKHR surface;
    VkWin32SurfaceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = window->handle;
    create_info.hinstance = GetModuleHandle(nullptr);
    VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface));
    return surface;
}

void PlatformShowErrorDialog(const char *message)
{
    MessageBoxA(0, message, 0, MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
}

#undef global
#undef internal
#undef persistent