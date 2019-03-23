// Contains global definitions and function that make life a little convienent

#pragma once
#include <stdint.h>

#define local_persist static
// TODO(Matt): These are broken, for some reason. MSVC has some functions
// called "global" and "internal" way over in some IOS files or something.
// Unsure how to handle that.
//#define global static
//#define internal static

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
// TODO(Dustin/Matthew): Temporary solution for material id
// Unique material id
//extern unsigned int material_id = 0;

// TODO(Dustin/Matthew): Temporary solution for model id
// Unique Model id
// extern unsigned int model_id = 0;