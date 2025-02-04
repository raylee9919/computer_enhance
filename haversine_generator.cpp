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
#include <time.h>

#include "core.h"
#include "haversine_shared.cpp"

internal b32
string_equal(const char *str1, const char *str2)
{
    b32 result = true;
    while (*str1 && *str2)
    {
        if ((*str1 != *str2) ||
            !*str1 ||
            !*str2)
        {
            result = false;
            break;
        }
        else
        {
            ++str1;
            ++str2;
        }
    }
    return result;
}

internal f64
random_unilateral(void)
{
    f64 result = (f64)rand() / (f64)RAND_MAX;
    return result;
}

enum Generator_Type
{
    Generator_Type_Invalid,
    Generator_Type_Uniform,
    Generator_Type_Cluster,
};

int main(int argc, char **args)
{
    if (argc == 4)
    {
        Generator_Type generator_type = Generator_Type_Invalid;

        if (string_equal(args[1], "uniform"))
            generator_type = Generator_Type_Uniform;
        else if (string_equal(args[1], "cluster"))
            generator_type = Generator_Type_Cluster;
        else
            invalid_code_path;

        if (generator_type != Generator_Type_Invalid)
        {
            u32 random_seed = atoi(args[2]);
            u32 pair_count = atoi(args[3]);

            srand(random_seed);

            f64 haversine_sum = 0.0;

            FILE *haversine_json_file = fopen(haversine_json_filename, "wb");
            if (haversine_json_file)
            {
                fprintf(haversine_json_file, "{\"pairs\":[\n");
                if (generator_type == Generator_Type_Uniform)
                {
                    for (u32 pair_index = 0; pair_index < pair_count; ++pair_index)
                    {
                        f64 x0 = random_unilateral() * 180.0;
                        f64 y0 = random_unilateral() * 180.0;
                        f64 x1 = random_unilateral() * 180.0;
                        f64 y1 = random_unilateral() * 180.0;
                        fprintf(haversine_json_file, "  {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}", x0, y0, x1, y1);
                        if (pair_index != pair_count - 1)
                            fprintf(haversine_json_file, ",\n");
                        
                        haversine_sum += haversine(x0, y0, x1, y1);
                    }
                }
                else if (generator_type == Generator_Type_Cluster)
                {
                    invalid_code_path;
                }
                else
                {
                    invalid_code_path;
                }
                fprintf(haversine_json_file, "\n]}");
                fclose(haversine_json_file);
                fprintf(stdout, "[OK]: Written %s\n", haversine_json_filename);
            }

            FILE *haversine_answer_file = fopen(haversine_answer_filename, "wb");
            if (haversine_answer_file)
            {
                fprintf(haversine_answer_file, "%.16f", haversine_sum);
                fclose(haversine_answer_file);
                fprintf(stdout, "[OK]: Written %s\n", haversine_answer_filename);
            }
            else
            {
                fprintf(stderr, "[ERROR]: Couldn't open %s\n", haversine_answer_filename);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "[ERROR]: first argument must be [uniform|cluster].\n");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "haversine_generator [uniform|cluster] [random_seed] [coordinate_pair_#]");
        return 1;
    }
}
