#ifndef CORE_H_
#define CORE_H_
/* ========================================================================

   (C) Copyright 2025 by Sung Woo Lee, All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   ======================================================================== */




    
#include <stdint.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef int32_t b32;

typedef size_t    mmm;
typedef uintptr_t umm;
typedef intptr_t  smm;

#define internal static
#define global   static
#define local    static

#define KB(x) (   x  * 1024LL)
#define MB(x) (KB(x) * 1024LL)
#define GB(x) (MB(x) * 1024LL)

#define CONCAT_(A, B) A##B
#define CONCAT(A, B) CONCAT_(A, B)

#define array_count(ARR) (sizeof(ARR) / sizeof(ARR[0]))

#define assert(EXP) if (!(EXP)) { *(volatile int *)0 = 0; }
#define invalid_code_path assert(!"Invalid Code Path")
#define invalid_default_case default: { invalid_code_path; } break


#endif // CORE_H_
