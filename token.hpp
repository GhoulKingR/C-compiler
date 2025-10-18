#pragma once

#include <string>

struct Token
{
    enum Type
    {
        IDENTIFIER, CONSTANT, END,

        // types
        INT, VOID, CHAR,

        // keywords
        RETURN,

        // symbols
        OPEN_PARENTHESIS, CLOSE_PARENTHESIS,
        OPEN_BRACE, CLOSE_BRACE, SEMICOLON,
        EQUAL
    } type;
    std::string value;
    int line;

    Token(Type t, std::string v, int line)
    : type(t), value(v), line(line) {}
};
