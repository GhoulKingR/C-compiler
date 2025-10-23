#pragma once

#include <string.h>
#include <stdbool.h>
#include "helpers.h"

enum token_type
{
    TOKEN_IDENTIFIER, TOKEN_CONSTANT, TOKEN_EOF,

    // types
    TOKEN_INT, TOKEN_VOID, TOKEN_CHAR,

    // keywords
    TOKEN_RETURN,

    // symbols
    TOKEN_OPEN_PARENTHESIS, TOKEN_CLOSE_PARENTHESIS,
    TOKEN_OPEN_BRACE, TOKEN_CLOSE_BRACE, TOKEN_SEMICOLON,
    TOKEN_EQUAL
};

struct token
{
    enum token_type type;
    char* value;
    bool allocated;
    int line;
};

void token_insert(struct m_vector* vec, struct token data) {
    if (vec->_size + 1 >= vec->_capacity)
        vector_resize(vec, vec->_capacity + 100);

    ((struct token*) vec->_data)[vec->_size] = data;
    vec->_size++;
}

struct token token_at(struct m_vector *vec, int pos) {
    return ((struct token*) vec->_data)[pos];
}

void token_cleanup(struct m_vector *tokens) {
    for (int i = 0; i < tokens->_size; i++) {
        struct token t = ((struct token*) tokens->_data)[i];
        if (t.allocated) {
            free(t.value);
        }
    }
    vector_free(tokens);
}