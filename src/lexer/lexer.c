#include "token.h"

int seek_identifier(int start, const char *content, struct m_vector* tokens, int line)
{
    // seek to the end of the identifier/keyword
    int end = start;
    while (is_alnum(content[end]))
        end++;

    const int size = end - start;
    char *token_text = str_sub(content, start, size);
    enum token_type token_type = token_get_keyword(token_text);

    // check whether the text gotten is either a keyword or identifier
    if (token_type != TOKEN_NOT_KEYWORD)
    {
        token_insert(tokens, (struct token) {
            .type = token_type,
            .value = token_text,
            .line = line,
            .allocated = true,
        });
    }
    else
    {
        token_insert(tokens, (struct token) {
            .type = TOKEN_IDENTIFIER,
            .value = token_text,
            .line = line,
            .allocated = true
        });
    }

    return end;
}

int seek_constant(int start, const char *content, struct m_vector* tokens, int line)
{
    // seek to the end of the constant
    int end = start;
    while (is_digit(content[end]))
        end++;
    
    const int size = end - start;
    char *token_text = str_sub(content, start, size);

    // push it to the token list
    token_insert(tokens, (struct token) {
        .type = TOKEN_CONSTANT,
        .value = token_text,
        .line = line,
        .allocated = true,
    });

    return end;
}

struct m_vector *lexer(const char *content)
{
    struct m_vector *tokens = vector_init(sizeof(struct token));
    int current = 0;
    int line = 1;
    size_t size = strlen(content);

    while (current < size)
    {
        switch (content[current])
        {
            // symbols
            case '(':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_OPEN_PARENTHESIS,
                    .value = "(",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case ')':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_CLOSE_PARENTHESIS,
                    .value = ")",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '{':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_OPEN_BRACE,
                    .value = "{",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '}':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_CLOSE_BRACE,
                    .value = "{",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case ';':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_SEMICOLON,
                    .value = "{",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '^':
                current++;
                if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_CARET_EQUAL,
                        .value = "^=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_CARET,
                        .value = "^",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '%':
                current++;
                if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_PERCENT_EQUAL,
                        .value = "%=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_PERCENT,
                        .value = "%",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '&':
                current++;
                if (content[current] == '&') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_AND_AND,
                        .value = "&&",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_AND_EQUAL,
                        .value = "&=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_AND,
                        .value = "&",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '|':
                current++;
                if (content[current] == '|') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_PIPE_PIPE,
                        .value = "||",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_PIPE_EQUAL,
                        .value = "|=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_PIPE,
                        .value = "|",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '=':
                current++;
                if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_EQUAL_EQUAL,
                        .value = "==",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_EQUAL,
                        .value = "=",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '!':
                current++;
                if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_BANG_EQUAL,
                        .value = "!=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_BANG,
                        .value = "!",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '>':
                current++;
                if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_GREATER_THAN_EQUAL,
                        .value = ">=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else if (content[current] == '>') {
                    current++;
                    if (content[current] == '=') {
                        token_insert(tokens, (struct token) {
                            .type = TOKEN_GT_GT_EQUAL,
                            .value = ">>=",
                            .line = line,
                            .allocated = false,
                        });
                        current++;
                    } else {
                        token_insert(tokens, (struct token) {
                            .type = TOKEN_GT_GT,
                            .value = ">>",
                            .line = line,
                            .allocated = false,
                        });
                    }
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_GREATER_THAN,
                        .value = ">",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '<':
                current++;
                if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_LESS_THAN_EQUAL,
                        .value = "<=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else if (content[current] == '<') {
                    current++;
                    if (content[current] == '=') {
                        token_insert(tokens, (struct token) {
                            .type = TOKEN_LT_LT_EQUAL,
                            .value = "<<=",
                            .line = line,
                            .allocated = false,
                        });
                        current++;
                    } else {
                        token_insert(tokens, (struct token) {
                            .type = TOKEN_LT_LT,
                            .value = "<<",
                            .line = line,
                            .allocated = false,
                        });
                    }
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_LESS_THAN,
                        .value = "<",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '-':
                current++;
                if (content[current] == '-') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_MINUS_MINUS,
                        .value = "--",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_MINUS_EQUAL,
                        .value = "-=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_MINUS,
                        .value = "-",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '+':
                current++;
                if (content[current] == '+') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_PLUS_PLUS,
                        .value = "++",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_PLUS_EQUAL,
                        .value = "+=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_PLUS,
                        .value = "+",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '/':
                current++;
                if (content[current] == '/') {
                    current++;
                    for (; content[current] != '\n' && content[current] != '\0'; current++);
                } else if (content[current] == '*') {
                    current++;
                    int starting_line = line;

                    while (true) {
                        if (content[current] == '\0') {
                            fprintf(stderr,
                                "Unexpected EOF symbol, missing comment terminator '*/' for line %d\n",
                                starting_line);
                            goto cleanup;
                        } else if (content[current] == '\n') {
                            line++;
                            current++;
                        } else if (content[current] == '*') {
                            current++;

                            if (content[current] == '/') {
                                current++;
                                break;
                            }
                        } else current++;
                    }
                } else if (content[current] == '=') {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_SLASH_EQUAL,
                        .value = "/=",
                        .line = line,
                        .allocated = false,
                    });
                    current++;
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_SLASH,
                        .value = "/",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '~':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_TILDA,
                    .value = "~",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '?':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_QUESTION_MARK,
                    .value = "?",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case ':':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_COLON,
                    .value = ":",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '*':
                current++;
                if (content[current] == '=') {
                    current++;
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_STAR_EQUAL,
                        .value = "*=",
                        .line = line,
                        .allocated = false,
                    });
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_STAR,
                        .value = "*",
                        .line = line,
                        .allocated = false,
                    });
                }
                break;
            case '\'':
                current++;
                {
                    char toinsert;

                    if (content[current] == '\\') {
                        current++;
                        char c = content[current];

                        toinsert =  c == 'a' ? '\a' :
                                    c == 'b' ? '\b' :
                                    c == 'e' ? '\e' :
                                    c == 'f' ? '\f' :
                                    c == 'n' ? '\n' :
                                    c == 'r' ? '\r' :
                                    c == 't' ? '\t' :
                                    c == 'v' ? '\v' :
                                    c == '\\' ? '\\' :
                                    c == '\'' ? '\'' :
                                    c == '"' ? '"' :
                                    c == '?' ? '\?' :
                                    0;

                        if (toinsert == 0) {
                            if (is_digit(content[current]) && is_digit(content[current + 1]) && is_digit(content[current + 2])) {
                                char *octal = str_sub(content, current, 3);
                                toinsert = (int) strtol(octal, NULL, 8);
                                free(octal);
                                current += 3;
                            } else if (content[current] == 'x' && is_hex(content[current + 1]) && is_hex(content[current + 2])) {
                                current++;
                                char *hex = str_sub(content, current, 2);
                                toinsert = (int) strtol(hex, NULL, 16);
                                free(hex);
                                current += 2;
                            } else {
                                toinsert = ' ';
                                current++;
                            }
                        }
                    } else {
                        toinsert = content[current++];
                    }

                    token_insert(tokens, (struct token) {
                        .type = TOKEN_CONSTANT, 
                        .value = int_to_str((int) toinsert),
                        .line = line,
                        .allocated = true,
                    });
                }
                    
                if (content[current] == '\'') current++;
                else goto syntax_error;
                break;

            // skips
            case '\n':
                line++;
            case '\r':
            case '\t':
            case ' ':
                current++;
                break;

            default:
                // identifier
                if (is_alpha(content[current]))
                    current = seek_identifier(current, content, tokens, line);
                // constants
                else if (is_digit(content[current]))
                    current = seek_constant(current, content, tokens, line);
                else goto syntax_error;
                break;
        }
    }

    token_insert(tokens, (struct token) {
        .type = TOKEN_EOF,
        .value = "EOF",
        .line = line
    });

    return tokens;
    
syntax_error:
    fprintf(stderr,
        "Syntax error on line %d, unexpected character '%c'\n",
        line, content[current]
    );

cleanup:
    token_cleanup(tokens);
    return NULL;
}