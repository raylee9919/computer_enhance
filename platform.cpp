/* ========================================================================

   (C) Copyright 2025 by Sung Woo Lee, All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   ======================================================================== */

#include "core.h"

#ifdef _MSC_VER
  #include <windows.h>
  
  static u64
  get_os_timer_frequency(void)
  {
      LARGE_INTEGER value;
      QueryPerformanceFrequency(&value);
      return value.QuadPart;
  }
  
  static u64
  read_os_timer(void)
  {
      LARGE_INTEGER value;
      QueryPerformanceCounter(&value);
      return value.QuadPart;
  }
#else
  static_assert(0, "no MSVC found.");
#endif


static u64
read_cpu_timer(void)
{
    // @NOTE: If you were on ARM, you would need to replace __rdtsc
    // with one of their performance counter read instructions, depending
    // on which ones are available on your platform.

    return __rdtsc();
}

static u64
estimate_cpu_frequency(void)
{
    u64 ms_to_wait = 100;
    u64 os_freq = get_os_timer_frequency();

    u64 cpu_start = read_cpu_timer();
    u64 os_start = read_os_timer();
    u64 os_end = 0;
    u64 os_elapsed = 0;
    u64 os_wait_time = os_freq * ms_to_wait / 1000;
    while (os_elapsed < os_wait_time) {
        os_end = read_os_timer();
        os_elapsed = os_end - os_start;
    }

    u64 cpu_end = read_cpu_timer();
    u64 cpu_elapsed = cpu_end - cpu_start;

    u64 cpu_freq = 0;
    if (os_elapsed) {
        cpu_freq = os_freq * cpu_elapsed / os_elapsed;
    }
    
    return cpu_freq;
}
