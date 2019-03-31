#ifndef PLATFORM_H
#define PLATFORM_H

// Platform specific window info.
struct PlatformWindow;
// Platform specific timer info.
struct PlatformGlobalTimer;

// Called when the OS window is resized, including minimize/maximize.
// Parameters are the new width and height of the client area, in pixels.
typedef void (*PlatformResizeCallback)(const PlatformWindow *window, u32 width, u32 height);

// Called when a mouse button is pressed. Button parameter is 1 for left,
// 2 for right, 3 for middle, and 4 and 5 for thumb buttons.
typedef void (*PlatformMouseButtonCallback)(const PlatformWindow *window, u32 button, EButtonState state);

// Called when the mouse wheel is scrolled. Amount parameter is positive
// if scrolled away from the user, and negative if towards.
typedef void (*PlatformMouseWheelCallback)(const PlatformWindow *window, s32 scroll_amount);

// Called when a keyboard key is pressed, released, or is being held.
// Parameters are the code of the modified key, and the new state.
typedef void (*PlatformKeyCallback)(const PlatformWindow *window, EKeyCode key, EButtonState state);

// TODO(Matt): Callback for double click, and mouse capture handling.

// Creates, but does not show the platform window. Does not start the
// message loop, and does not initialize rendering or input.
void PlatformCreateWindow(PlatformWindow *window);

// Checks the OS message queue, and processes all non-input events before
// returning, unless a quit is requested. Does not wait for new events.
// Returns -1 if the application should exit, or the number of processed
// messages otherwise. Note that user callbacks are executed for every 
// message. This prevents inputs from being dropped, but means that if 
// messages are enqueued faster than they are processed, the application
// can freeze!
// TODO(Matt): This isn't implemented properly yet - only one event is
// processed at a time. Once implemented, it may make sense to merge this
// with PlatformPeekInputEvents().
s32 PlatformPeekEvents();

// Checks the OS message queue, and processes the first event found before
// returning, unless a quit is requested. If no messages are found, will
// wait for new events. Returns -1 if the application should exit, or the
// number of processed messages otherwise.
s32 PlatformPollEvents();

// Checks the OS message queue, and processes all input events before
// returning, unless a quit is requested. Does not wait for new events.
// Returns -1 if a quit is requested, or the number of processed messages
// otherwise. Note that user callbacks are executed for every message. This
// prevents inputs from being dropped, but means that if messages are
// enqueued faster than they are processed, the application can freeze!
s32 PlatformPeekInputEvents();

// Sets up platform code for mouse/keyboard devices.
void PlatformSetupInputDevices(PlatformWindow *window);

// Gets the client area of the window. Returns false if either measure is zero (usually implies that the window is minimized).
bool PlatformGetWindowSize(const PlatformWindow *window, u32 *width, u32 *height);

// Shows the platform window.
void PlatformShowWindow(PlatformWindow *window);

// Hides the platform window.
void PlatformHideWindow(PlatformWindow *window);

// Dynamically loads the vulkan library, if it exists.
void PlatformLoadVulkanLibrary();

// Frees the vulkan library.
void PlatformFreeVulkanLibrary();

// Initializes the global timer. The resolution of the timer is platform-
// dependent.
void PlatformInitializeGlobalTimer(PlatformGlobalTimer *timer);

// Gets the number of seconds passed since the last call to
// PlatformGetTimerDelta(), or if it hasn't been called before, the time
// since the global timer was initialized.
double PlatformGetTimerDelta(PlatformGlobalTimer *timer);

// Gets the total number of seconds since the global timer was initialized.
double PlatformGetTimerTotalSeconds(PlatformGlobalTimer *timer);

// Gets the current coordinates of the cursor in pixels, measured from the
// top left of the client window. For normalized coordinates, use
// PlatformGetCursorLocationNormalized().
void PlatformGetCursorLocation(const PlatformWindow *window, int32_t *x, int32_t *y);

// Gets the current coordinates of the cursor as normalized coordinates,
// measured from the top-left of the client window. These range from
// 0..1 across the client window, although if the cursor is outside the
// client window, it's possible for the coordinates to be negative or
// greater than 1. For pixel coordinates, use PlatformGetCursorLocation().
void PlatformGetCursorLocationNormalized(const PlatformWindow *window, float *x, float *y);

// Gets the raw change in cursor location since the last call to
// PlatformPeekInputEvents(). This delta is screen-resolution INDEPENDENT,
// and dpi DEPENDENT. Use for processing cursor movement, not for updating
// cursor location.
void PlatformGetCursorDelta(s32 *x, s32 *y);

// Register a function to be called on window resize.
void PlatformRegisterResizeCallback(PlatformWindow *window, PlatformResizeCallback callback);

// Register a function to be called when a mouse button is modified.
void PlatformRegisterMouseButtonCallback(PlatformWindow *window, PlatformMouseButtonCallback callback);

// Register a function to be called when the mouse wheel is rotated.
void PlatformRegisterMouseWheelCallback(PlatformWindow *window, PlatformMouseWheelCallback callback);

// Register a function to be called a keyboard key is modified.
void PlatformRegisterKeyCallback(PlatformWindow *window, PlatformKeyCallback callback);

// Attempts to capture the mouse cursor, restricting it to the client area.
// Often called in tandem with PlatformHideCursor().
void PlatformCaptureCursor(const PlatformWindow *window);

// Releases the mouse cursor, if it has been captured. Often called in
// tandem with PlatformShowCursor().
void PlatformReleaseCursor();

// Moves the mouse cursor to the location at which it was hidden, and shows
// it. Often called in tandem with PlatformReleaseCursor().
void PlatformShowCursor();

// Hides the mouse cursor, and saves the current location, so that it can
// be restored in the same location as it was hidden. Often called in tandem
// with PlatformCaptureCursor().
void PlatformHideCursor();

// Creates the vulkan window surface, using the vulkan library calls for the
// platform.
VkSurfaceKHR PlatformCreateSurface(const PlatformWindow *window, VkInstance instance);

// Sets the mouse cursor to a specified type.
void PlatformSetCursor(ECursor cursor);

// Pulls up a message dialog indicating that an error (usually a fatal one) has occured.
void PlatformShowErrorDialog(const char *message);

#endif