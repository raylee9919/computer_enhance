/* ========================================================================

   (C) Copyright 2025 by Sung Woo Lee, All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   ======================================================================== */




struct Memory_Arena
{
    u8 *base;
    mmm size;
    mmm used;
};

static void
init_arena(Memory_Arena *arena, mmm size)
{
    arena->base = (u8 *)malloc(size);
    arena->size = size;
    arena->used = 0;
    memset(arena->base, 0, size);
}

#define push_struct(ARENA, STRUCT) (STRUCT *)push_size(ARENA, sizeof(STRUCT))
#define push_array(ARENA, STRUCT, COUNT) (STRUCT *)push_size(ARENA, sizeof(STRUCT)*COUNT)
static void *
push_size(Memory_Arena *arena, mmm size)
{
    assert(arena->used + size <= arena->size);
    void *result = (arena->base + arena->used);
    arena->used += size;
    return result;
}
