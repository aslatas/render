
#ifndef WIN32PLATFORMLAYER_H
#define WIN32PLATFORMLAYER_H

// TODO(Matt): No idea if this thread representation will work. Just testing stuff out.
struct PlatformThread
{
    EThreadType type;
    HANDLE handle;
    DWORD id;
};

struct Win32DirectoryWatcher
{
    char *directory_path;
    // NOTE(Matt): The buffers are heap allocated to guarantee DWORD alignment (necessary for
    // ReadDirectoryChangesW) and because they're kinda big. 
    BYTE *buffers[2]; // We use two buffers as a swapchain.
    HANDLE directory_handle; // Directory to watch.
    DWORD notify_flags;
    u32 buffer_index;
    bool watch_subdirectories;
};

// Platform specific window information.
struct PlatformWindow
{
    // NOTE(Matt): Window handle packs a pointer to an instance of this
    // struct as its userdata. Access with GetWindowLongPtr().
    HWND handle; // Window handle.
    RAWINPUTDEVICE input_devices[1]; // Mouse device for raw input.
    
    s32 mouse_x; // Current mouse x, in window coords (pixels). Can be < 0.
    s32 mouse_y; // Current mouse y, in window coords (pixels). Can be < 0.
    s32 mouse_x_delta; // Raw mouse x delta since last frame. DPI dependent.
    s32 mouse_y_delta; // Raw mouse y delta since last frame. DPI dependent.
    
    bool is_minimized; // True if the window is minimized, false otherwise.
    bool is_resizing; // True if being resized, false otherwise.
    // TODO(Matt): Should this be false while minimized? If so, parts of
    // WM_SIZE need to change.
    bool is_visible; // True if currently being shown, false otherwise.
    bool has_been_shown; // True if ever shown, false otherwise.
    
    // NOTE(Matt): Callbacks are valid per-window, to support many windows.
    PlatformResizeCallback resize_callback; // Called when a resize ends.
    PlatformMouseButtonCallback mouse_button_callback; // Button callback.
    PlatformMouseWheelCallback mouse_wheel_callback; // Wheel callback.
    PlatformKeyCallback key_callback; // Keyboard key callback.
    PlatformFileChangeCallback file_change_callback; // File change callback.
};

// Platform specific timer info.
struct PlatformGlobalTimer
{
    LARGE_INTEGER frequency; // Frequency of the timer, in counts / second.
    LARGE_INTEGER start; // Start time for the timer.
    LARGE_INTEGER last; // Last time measured by GetGlobalTimerDelta().
};

// Window message handling procedure. 
LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif