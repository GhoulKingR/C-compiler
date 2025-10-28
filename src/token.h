#pragma once

#include <string.h>
#include <stdbool.h>
#include "helpers.h"

enum token_type
{
    TOKEN_IDENTIFIER, TOKEN_CONSTANT, TOKEN_EOF, TOKEN_NOT_KEYWORD,

    // types
    TOKEN_INT, TOKEN_VOID, TOKEN_CHAR,

    // keywords
    TOKEN_RETURN,

    // symbols
    TOKEN_OPEN_PARENTHESIS, TOKEN_CLOSE_PARENTHESIS,
    TOKEN_OPEN_BRACE, TOKEN_CLOSE_BRACE, TOKEN_SEMICOLON,
    TOKEN_BANG, TOKEN_GREATER_THAN, TOKEN_LESS_THAN,
    TOKEN_MINUS, TOKEN_PLUS, TOKEN_SLASH, TOKEN_STAR, TOKEN_TILDA,
    TOKEN_AND, TOKEN_PIPE, TOKEN_PERCENT, TOKEN_CARET,
    TOKEN_QUESTION_MARK, TOKEN_COLON,

    // double-character symbols
    TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL, TOKEN_GREATER_THAN_EQUAL,
    TOKEN_LESS_THAN_EQUAL, TOKEN_PIPE_PIPE, TOKEN_AND_AND,
    TOKEN_LT_LT /*<<*/, TOKEN_GT_GT /*>>*/, TOKEN_PLUS_PLUS,
    TOKEN_MINUS_MINUS,

    // Assignment symbols
    TOKEN_EQUAL, TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL,
    TOKEN_STAR_EQUAL, TOKEN_SLASH_EQUAL, TOKEN_PERCENT_EQUAL,
    TOKEN_LT_LT_EQUAL, TOKEN_GT_GT_EQUAL, TOKEN_AND_EQUAL,
    TOKEN_CARET_EQUAL, TOKEN_PIPE_EQUAL
};

struct token
{
    enum token_type type;
    char* value;
    bool allocated;
    int line;
};

void token_insert(struct m_vector* vec, struct token data);
enum token_type token_get_keyword(const char *str);
struct token token_at(struct m_vector *vec, int pos);
void token_cleanup(struct m_vector *tokens);

