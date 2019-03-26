// TODO(Matt): Literally all of this should probably go in an Input.h or
// other file. We should probably separate out input anyway.

#ifndef COMMON_H

enum EButtonState
{
    NONE, RELEASED, PRESSED, HELD
};

enum EKeyCode
{
    KEY_UNKNOWN,
    KEY_CTRL,// = VK_CONTROL,
    KEY_W,// = 0x57,
    KEY_A,// = 0x41,
    KEY_S,// = 0x53,
    KEY_D,// = 0x44,
    KEY_Q,// = 0x51,
    KEY_E,// = 0x45
};

enum EInputMode
{
    UI, VIEWPORT
};

enum ECursor
{
    ARROW, CROSS, SPINNER, CARET
};

#define COMMON_H
#endif