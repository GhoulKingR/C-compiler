#pragma once

#include <string>

struct Token
{
    enum Type
    {
        IDENTIFIER, CONSTANT, END,

        // keywords
        INT, VOID, RETURN,

        // symbols
        OPEN_PARENTHESIS, CLOSE_PARENTHESIS,
        OPEN_BRACE, CLOSE_BRACE, SEMICOLON
    } type;
    std::string value;
    int line;

    Token(Type t, std::string v, int line)
    : type(t), value(v), line(line) {}
};
