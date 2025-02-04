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

struct Buffer
{
    mmm size;
    u8 *data;
};

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

internal b32
is_whitespace(char c)
{
    time_function();
    return (c == ' '  || c == '\n' || c == '\r' || c == '\t');
}

internal b32
is_number(char c)
{
    time_function();
    return (c >= '0' && c <= '9');
}

enum Token_Type
{
    Token_Type_EOF,
    Token_Type_Invalid,
    Token_Type_Left_Brace,
    Token_Type_Right_Brace,
    Token_Type_Left_Bracket,
    Token_Type_Right_Bracket,
    Token_Type_Comma,
    Token_Type_Colon,
    Token_Type_String,
    Token_Type_Number,
};

struct Token
{
    Token_Type type;
    Buffer data;
};

struct Tokenizer
{
    u8 *at;
};

internal Buffer
push_char(char c, Memory_Arena *arena)
{
    time_function();

    Buffer result = {};

    char *x = push_struct(arena, char);
    *x = c;

    result.data = (u8 *)x;
    result.size = 1;
    return result;
}

internal Buffer
push_string(char *begin, char *end, Memory_Arena *arena)
{
    time_function();

    Buffer result = {};

    mmm length = (end - begin);
    char *data = (char *)push_size(arena, length);
    for (u32 i = 0; i < length; ++i)
        data[i] = begin[i];

    result.data = (u8 *)data;
    result.size = length;
    return result;
}

internal void
push_token(Tokenizer *tokenizer, Token_Type type,
           Memory_Arena *token_arena, Memory_Arena *literal_arena)
{
    time_function();

    Token *tk = push_struct(token_arena, Token);
    tk->type = type;
    switch (type)
    {
        case Token_Type_EOF:
        case Token_Type_Invalid:
        {
            tk->data = Buffer{};
        } break;;

        case Token_Type_Left_Brace:
        {
            tk->data = push_char('{', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Right_Brace:
        {
            tk->data = push_char('}', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Left_Bracket:
        {
            tk->data = push_char('[', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Right_Bracket:
        {
            tk->data = push_char(']', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Comma:
        {
            tk->data = push_char(',', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Colon:
        {
            tk->data = push_char(':', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_String:
        {
            char* start = (char *)++tokenizer->at;
            while (*tokenizer->at != '"')
                ++tokenizer->at;
            char *end = (char *)tokenizer->at;
            tk->data = push_string(start, end, literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Number:
        {
            tk->data.data = (u8 *)push_struct(literal_arena, f64);
            tk->data.size = sizeof(f64);
            f64 n = 0.0;
            while (is_number(*tokenizer->at))
            {
                n = n * 10.0 + (f64)(*tokenizer->at - '0');
                ++tokenizer->at;
            }

            if (*tokenizer->at == '.')
                ++tokenizer->at;

            f64 m = 0.1;
            while (is_number(*tokenizer->at))
            {
                n += m * (f64)(*tokenizer->at - '0');
                ++tokenizer->at;
                m /= 10.0;
            }

            *((f64 *)tk->data.data) = n;
        } break;

        invalid_default_case;
    }
}

internal void
tokenize(Buffer buffer, Memory_Arena *token_arena, Memory_Arena *literal_arena)
{
    time_function();

    Tokenizer tk = {};
    tk.at = buffer.data;

    for (;;)
    {
        switch (*tk.at)
        {
            case 0: 
            {
                push_token(&tk, Token_Type_EOF, token_arena, literal_arena);
                return;
            } break;

            case '{':
            {
                push_token(&tk, Token_Type_Left_Brace, token_arena, literal_arena);
            } break;

            case '}':
            {
                push_token(&tk, Token_Type_Right_Brace, token_arena, literal_arena);
            } break;

            case '[':
            {
                push_token(&tk, Token_Type_Left_Bracket, token_arena, literal_arena);
            } break;

            case ']':
            {
                push_token(&tk, Token_Type_Right_Bracket, token_arena, literal_arena);
            } break;

            case ',':
            {
                push_token(&tk, Token_Type_Comma, token_arena, literal_arena);
            } break;

            case ':':
            {
                push_token(&tk, Token_Type_Colon, token_arena, literal_arena);
            } break;

            default:
            {
                if (is_whitespace(*tk.at))
                {
                    ++tk.at;
                }
                else if (*tk.at == '"')
                {
                    push_token(&tk, Token_Type_String, token_arena, literal_arena);
                }
                else if (is_number(*tk.at))
                {
                    push_token(&tk, Token_Type_Number, token_arena, literal_arena);
                }
                else
                {
                    invalid_code_path;
                }
            } break;
        }
    }
}

internal void
DEBUG_print_tokens(Memory_Arena *arena)
{
    Token *tk = (Token *)arena->base;
    for (;;)
    {
        switch (tk->type)
        {
            case Token_Type_EOF:
            {
                printf("EOF\n");
                return;
            } break;
            case Token_Type_Invalid:
            {
                printf("Invalid\n");
            } break;
            case Token_Type_Left_Brace:
            case Token_Type_Right_Brace:
            case Token_Type_Left_Bracket:
            case Token_Type_Right_Bracket:
            case Token_Type_Comma:
            case Token_Type_Colon:
            case Token_Type_String:
            {
                printf("%.*s\n", (s32)tk->data.size, tk->data.data);
            } break;
            case Token_Type_Number:
            {
                printf("%.16f\n", *(f64 *)tk->data.data);
            } break;
        }

        ++tk;
    }
}

struct Parser
{
    Token *at;
};

internal void
parse(Memory_Arena *token_arena, Memory_Arena *data_arena)
{
    Parser parser = {};
    parser.at = (Token *)token_arena->base;
    for (;;)
    {
        Token tk = *parser.at;
        switch (tk.type)
        {
            case Token_Type_EOF:
            {
                return;
            } break;

            case Token_Type_Invalid:
            {
                invalid_code_path;
            } break;

            case Token_Type_Left_Brace:
            {
                parse_object(&parser);
            } break;

            case Token_Type_Left_Bracket:
            {
                parse_array(&parser);
            } break;

            case Token_Type_String:
            {
                parse_string(&parser);
            } break;

            case Token_Type_Number:
            {
                parse_number(&parser);
            } break;

            invalid_default_case;
        }

        ++parser.at;
    }
}

int main(void)
{
    begin_profile();

    Memory_Arena file_arena = {};
    init_arena(&file_arena, MB(500));

    Buffer entire_file = read_entire_file_and_null_terminate(haversine_json_filename, &file_arena);
    if (entire_file.data)
    {
        Memory_Arena token_arena = {};
        Memory_Arena literal_arena = {};
        Memory_Arena data_arena = {};

        init_arena(&token_arena, GB(1));
        init_arena(&literal_arena, MB(50));
        init_arena(&data_arena, MB(50));

        tokenize(entire_file, &token_arena, &literal_arena);
        free_arena(&file_arena);
        // DEBUG_print_tokens(&token_arena);

        parse(&token_arena, &data_arena);
    }
    else
    {
        invalid_code_path;
    }

    end_and_print_profile();
    return 0;
}
