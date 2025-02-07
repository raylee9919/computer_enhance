/* ========================================================================

   (C) Copyright 2025 by Sung Woo Lee, All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   ======================================================================== */


#include "core.h"

#ifndef __PROFILER
  #define __PROFILER 0
#endif

#if __PROFILER
  #define time_block(name) Profile_Block CONCAT(block, __LINE__)(name, __COUNTER__ + 1)
  #define time_function() time_block(__func__)

  struct Profile_Anchor
  {
      u64 tsc_elapsed_exclusive;
      u64 tsc_elapsed_inclusive;
      u64 hit_count;
      char const *label;
  };
  
  struct Profiler
  {
      Profile_Anchor anchors[4096];
  
      u64 start_tsc;
      u64 end_tsc;
  };
  static Profiler g_profiler;
  static u32 g_profiler_parent;
  
  struct Profile_Block
  {
      Profile_Block(char const *label_, u32 anchor_index_)
      {
          parent_index = g_profiler_parent;
  
          anchor_index = anchor_index_;
          label = label_;
  
          Profile_Anchor *anchor = g_profiler.anchors + anchor_index;
          old_tsc_elapsed_inclusive = anchor->tsc_elapsed_inclusive;
  
          g_profiler_parent = anchor_index;
          tsc_start = read_cpu_timer();
      }
  
      ~Profile_Block()
      {
          u64 elapsed = read_cpu_timer() - tsc_start;
          g_profiler_parent = parent_index;
  
          Profile_Anchor *parent = g_profiler.anchors + parent_index;
          Profile_Anchor *anchor = g_profiler.anchors + anchor_index;
  
          parent->tsc_elapsed_exclusive -= elapsed;
          anchor->tsc_elapsed_exclusive += elapsed;
          anchor->tsc_elapsed_inclusive = old_tsc_elapsed_inclusive + elapsed;
          ++anchor->hit_count;
  
          /* NOTE: This write happens every time solely because there is no
             straightforward way in C++ to have the same ease-of-use. In a better programming
             language, it would be simple to have the anchor points gathered and labeled at compile
             time, and this repetative write would be eliminated. */
          anchor->label = label;
      }
  
      char const *label;
      u64 old_tsc_elapsed_inclusive;
      u64 tsc_start;
      u32 parent_index;
      u32 anchor_index;
  };
  
  static void
  begin_profile(void)
  {
      g_profiler.start_tsc = read_cpu_timer();
  }
  
  static void
  end_and_print_profile(void)
  {
      g_profiler.end_tsc = read_cpu_timer();
      u64 cpu_frequency = estimate_cpu_frequency();
  
      u64 total_cpu_elapsed = (g_profiler.end_tsc - g_profiler.start_tsc);
  
      if (cpu_frequency)
          printf("\nTotal time: %.4fms (CPU freq %llu = %.2fGHz)\n", 1000.0 * (f64)total_cpu_elapsed / (f64)cpu_frequency, cpu_frequency, (f64)cpu_frequency / (f64)(1'000'000'000));
  
      for (u32 anchor_index = 0; anchor_index < array_count(g_profiler.anchors); ++anchor_index)
      {
          Profile_Anchor *anchor = g_profiler.anchors + anchor_index;
          if (anchor->tsc_elapsed_inclusive)
          {
              f64 percent = 100.0 * ((f64)anchor->tsc_elapsed_exclusive / (f64)total_cpu_elapsed);
              printf("  %s[%llu]: %llu (%.2f%%", anchor->label, anchor->hit_count, anchor->tsc_elapsed_exclusive, percent);
              if (anchor->tsc_elapsed_inclusive != anchor->tsc_elapsed_exclusive)
              {
                  percent = 100.0 * ((f64)anchor->tsc_elapsed_inclusive / (f64)total_cpu_elapsed);
                  printf(", %.2f%% w/children", percent);
              }
              printf(")\n");
          }
      }
  }
#else
  #define time_block(...)
  #define time_function(...)
  static void begin_profile(void) {}
  static void end_and_print_profile(void) {}
#endif
