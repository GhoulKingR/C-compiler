#pragma once

#include <string>
#include <variant>
#include <vector>

#include "targets/interface.hpp"
#include "token.hpp"

class Visitor;

/** AST nodes definitions ***/
struct Return
{
    Token value;
    Return (Token t) : value(t) {}

    void accept(Visitor &visitor) {
        visitor.visit(*this);
    }
};

struct Statement
{
    enum Type { RETURN } type;
    std::variant<Return> obj;

    Statement (Return rtn)
    : obj(rtn), type(RETURN) {}

    void accept(Visitor &visitor) {
        visitor.visit(*this);
    }
};

struct Function
{
    Token::Type type;
    std::string name;
    std::vector<Statement> statements;

    Function (Token::Type t, std::string n, std::vector<Statement> stmts)
    : name(n), statements(stmts), type(t) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

struct Declaration
{
    enum Type { FUNCTION } type;
    std::variant<Function> obj;

    Declaration(Function f) : obj(f), type(FUNCTION) {}
    
    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

struct Program
{
    std::vector<Declaration> declarations;
    Program (std::vector<Declaration> d) : declarations(d) {}

    void accept(Visitor& visitor) {
       visitor.visit(*this);
    }
};

// <identifier> : IDENTIFIER
// <int>        : CONSTANT

/** AST nodes definitions end **/
