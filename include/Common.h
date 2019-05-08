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
    KEY_CTRL,
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_Q,
    KEY_E,
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