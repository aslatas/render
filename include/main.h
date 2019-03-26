
#ifndef MAIN_H

s32 Main();
void Shutdown();
void ExitWithError(const char *message);

// TODO(Matt): Move these into an input component.
EInputMode GetInputMode();
float GetForwardAxis();
float GetRightAxis();
float GetUpAxis();

#define MAIN_H
#endif