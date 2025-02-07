/* ========================================================================

   (C) Copyright 2025 by Sung Woo Lee, All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   ======================================================================== */

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "core.h"
#include "platform.cpp"
#include "memory.cpp"
#include "profiler.cpp"
#include "haversine_shared.cpp"
#include "json_parser.cpp"

static f64
abs(f64 x)
{
    return ((x > 0) ? x : -x);
}

internal Buffer
read_entire_file_and_null_terminate(const char *filename, Memory_Arena *arena)
{
    time_function();

    Buffer result = {};

    FILE *file = fopen(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        result.size = (mmm)ftell(file);
        result.data = (u8 *)push_size(arena, result.size + 1);
        fseek(file, 0, SEEK_SET);
        fread(result.data, result.size, 1, file);
        result.data[result.size] = 0;
        fclose(file);
    }
    else
    {
        invalid_code_path;
    }
    return result;
}

//
// OBJECT
// { STRING : VALUE }
// { STRING : VALUE , STRING : VALUE , STRING : VALUE }
//
// ARRAY
// [ VALUE ]
// [ VALUE, VALUE, VALUE ]
//
// VALUE
// STRING|NUMBER|OBJECT|ARRAY|TRUE|FALSE|NULL
//

struct Haversine_Pair
{
    f64 x0, y0, x1, y1;
};

static f64
get_haversine_sum_from_json(Json_Object object, Memory_Arena *arena)
{
    time_function();
    f64 result = 0.0;
 
    if (object.strings[0] == "pairs")
    {
        Json_Value val = object.values[0];
        u64 pairs_count = val.array.used;
        Haversine_Pair *pairs = push_array(arena, Haversine_Pair, pairs_count);
        for (u32 idx = 0; idx < pairs_count; ++idx)
        {
            Json_Object pair = val.array.values[idx].object;
            for (u32 i = 0; i < 4; ++i)
            {
                if (pair.strings[i] == "x0")
                {
                    pairs[idx].x0 = pair.values[i].number;
                }
                else if (pair.strings[i] == "y0")
                {
                    pairs[idx].y0 = pair.values[i].number;
                }
                else if (pair.strings[i] == "x1")
                {
                    pairs[idx].x1 = pair.values[i].number;
                }
                else if (pair.strings[i] == "y1")
                {
                    pairs[idx].y1 = pair.values[i].number;
                }
                else
                {
                    invalid_code_path;
                }
            }
        }

        for (u32 idx = 0; idx < pairs_count; ++idx)
        {
            Haversine_Pair pair = pairs[idx];
            result += haversine(pair.x0, pair.y0, pair.x1, pair.y1);
        }
    }
    else
    {
        invalid_code_path;
    }

    return result;
}

int main(void)
{
    begin_profile();

    Memory_Arena file_arena = {};
    init_arena(&file_arena, MB(500));

    Buffer json_file = read_entire_file_and_null_terminate(haversine_json_filename, &file_arena);
    if (json_file.data)
    {
        Memory_Arena token_arena = {};
        Memory_Arena literal_arena = {};
        Memory_Arena data_arena = {};
        Memory_Arena haversine_arena = {};

        init_arena(&token_arena, GB(1));
        init_arena(&literal_arena, MB(50));
        init_arena(&data_arena, GB(1));
        init_arena(&haversine_arena, MB(50));

        tokenize(json_file, &token_arena, &literal_arena);
        // DEBUG_print_tokens(&token_arena);

        Json_Object root_object = parse_json(&token_arena, &data_arena);
        f64 haversine_sum = get_haversine_sum_from_json(root_object, &haversine_arena);

        Buffer answer_file = read_entire_file_and_null_terminate(haversine_answer_filename, &file_arena);
        Stream answer_stream = {};
        answer_stream.at = answer_file.data;
        f64 expected_haversine_sum = json_get_number_from_stream(&answer_stream);

        printf("Expected: %.16f km\nActual  : %.16f km\nError   : %.16f km\n", expected_haversine_sum, haversine_sum, abs(haversine_sum - expected_haversine_sum));
    }
    else
    {
        invalid_code_path;
    }

    end_and_print_profile();
}
