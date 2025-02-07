/* ========================================================================

   (C) Copyright 2025 by Sung Woo Lee, All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   ======================================================================== */




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
    Buffer buffer;
};

typedef Stream Tokenizer;

static b32
is_whitespace(char c)
{
    time_function();
    return (c == ' '  || c == '\n' || c == '\r' || c == '\t');
}

static b32
is_number(char c)
{
    time_function();
    return (c >= '0' && c <= '9');
}

static Buffer
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

static Buffer
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

static f64
json_get_number_from_stream(Stream *stream)
{
    f64 n = 0.0;
    while (is_number(*stream->at))
    {
        n = n * 10.0 + (f64)(*stream->at - '0');
        ++stream->at;
    }

    if (*stream->at == '.')
        ++stream->at;

    f64 m = 0.1;
    while (is_number(*stream->at))
    {
        n += m * (f64)(*stream->at - '0');
        ++stream->at;
        m /= 10.0;
    }

    return n;
}

static void
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
            tk->buffer = Buffer{};
        } break;;

        case Token_Type_Left_Brace:
        {
            tk->buffer = push_char('{', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Right_Brace:
        {
            tk->buffer = push_char('}', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Left_Bracket:
        {
            tk->buffer = push_char('[', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Right_Bracket:
        {
            tk->buffer = push_char(']', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Comma:
        {
            tk->buffer = push_char(',', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Colon:
        {
            tk->buffer = push_char(':', literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_String:
        {
            char* start = (char *)++tokenizer->at;
            while (*tokenizer->at != '"')
                ++tokenizer->at;
            char *end = (char *)tokenizer->at;
            tk->buffer = push_string(start, end, literal_arena);
            ++tokenizer->at;
        } break;

        case Token_Type_Number:
        {
            tk->buffer.data = (u8 *)push_struct(literal_arena, f64);
            tk->buffer.size = sizeof(f64);

            *((f64 *)tk->buffer.data) = json_get_number_from_stream(tokenizer);
        } break;

        invalid_default_case;
    }
}

static void
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

static void
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
                printf("%.*s\n", (s32)tk->buffer.size, tk->buffer.data);
            } break;
            case Token_Type_Number:
            {
                printf("%.16f\n", *(f64 *)tk->buffer.data);
            } break;
        }

        ++tk;
    }
}

struct Parser
{
    Token *at;

    Token eat()
    {
        return (*at++);
    }
};

union Json_Value;

struct Json_Object
{
    u64 size;
    u64 used;
    String *strings;
    Json_Value *values;
};

struct Json_Array
{
    u64 size;
    u64 used;
    Json_Value *values;
};

union Json_Value
{
    f64 number;
    String string;
    Json_Object object;
    Json_Array array;
};

static Json_Value parse_value(Parser *parser, Memory_Arena *token_arena, Memory_Arena *data_arena);

static Json_Object
parse_object(Parser *parser, Memory_Arena *token_arena, Memory_Arena *data_arena)
{
    time_function();

    Json_Object result = {};
    result.size = 10;
    result.strings = push_array(data_arena, String, result.size);
    result.values = push_array(data_arena, Json_Value, result.size);

    if (parser->eat().type == Token_Type_Left_Brace)
    {
        if (parser->at->type != Token_Type_Right_Brace)
        {
            for (;;)
            {
                Token string_token = parser->eat();
                if (string_token.type == Token_Type_String)
                {
                    if (parser->eat().type == Token_Type_Colon)
                    {
                        Json_Value value = parse_value(parser, token_arena, data_arena);

                        if (result.used == result.size)
                        {
                            u64 new_size = (result.size << 1);

                            String *new_strings = push_array(data_arena, String, new_size);
                            for (u32 i = 0; i < result.size; ++i)
                                new_strings[i] = result.strings[i];

                            Json_Value *new_values = push_array(data_arena, Json_Value, new_size);
                            for (u32 i = 0; i < result.size; ++i)
                                new_values[i] = result.values[i];

                            result.size = new_size;

                            result.values = new_values;
                            result.strings = new_strings;
                        }

                        result.strings[result.used] = string_token.buffer;
                        result.values[result.used] = value;
                        ++result.used;

                        Token next = parser->eat();
                        if (next.type == Token_Type_Right_Brace)
                        {
                            break;
                        }
                        else
                        {
                            assert(next.type == Token_Type_Comma);
                        }
                    }
                    else
                    {
                        invalid_code_path;
                    }
                }
                else
                {
                    invalid_code_path;
                }
            }
        }
        else
        {
            ++parser->at;
        }
    }
    else
    {
        invalid_code_path;
    }

    return result;
}

static Json_Array
parse_array(Parser *parser, Memory_Arena *token_arena, Memory_Arena *data_arena)
{
    time_function();

    Json_Array result = {};
    result.size = 10;
    result.values = push_array(data_arena, Json_Value, 10);

    if (parser->eat().type == Token_Type_Left_Bracket)
    {
        if (parser->at->type != Token_Type_Right_Bracket)
        {
            for (;;)
            {
                Json_Value value = parse_value(parser, token_arena, data_arena);

                if (result.used == result.size)
                {
                    u64 new_size = (result.size << 1);
                    Json_Value *new_values = push_array(data_arena, Json_Value, new_size);
                    for (u32 i = 0; i < result.size; ++i)
                        new_values[i] = result.values[i];
                    result.size = new_size;
                    result.values = new_values;
                }

                result.values[result.used++] = value;

                Token next = parser->eat();
                if (next.type == Token_Type_Right_Bracket)
                {
                    break;
                }
                else
                {
                    assert(next.type == Token_Type_Comma);
                }
            }
        }
        else
        {
            ++parser->at;
        }
    }
    else
    {
        invalid_code_path;
    }

    return result;
}

static Json_Value
parse_value(Parser *parser, Memory_Arena *token_arena, Memory_Arena *data_arena)
{
    time_function();

    Json_Value result = {};

    Token token = *parser->at;
    switch (token.type)
    {
        case Token_Type_String:
        {
            result.string = token.buffer;
            ++parser->at;
        } break;
        
        case Token_Type_Number:
        {
            result.number = *((f64 *)token.buffer.data);
            ++parser->at;
        } break;

        case Token_Type_Left_Brace:
        {
            result.object = parse_object(parser, token_arena, data_arena);
        } break;

        case Token_Type_Left_Bracket:
        {
            result.array = parse_array(parser, token_arena, data_arena);
        } break;

        invalid_default_case;
    }

    return result;
}

static Json_Object
parse_json(Memory_Arena *token_arena, Memory_Arena *data_arena)
{
    time_function();

    Parser parser = {};
    parser.at = (Token *)token_arena->base;

    Json_Object result = parse_object(&parser, token_arena, data_arena);

    return result;
}
