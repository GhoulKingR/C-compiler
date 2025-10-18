#pragma once

#include <string>
#include <variant>
#include <vector>

#include "exceptions.hpp"
#include "targets/interface.hpp"
#include "token.hpp"

class Visitor;

/** AST nodes definitions ***/
struct DataType {
    enum Type {
        INT, CHAR, VOID,
    } type;

    int size() {
        switch (type) {
            case INT:
                return 4;
            case CHAR:
                return 1;

            default:
                throw syntax_error("Unexpected error occured");
                break;
        }

    }

    DataType(Token::Type tokenType) {
        switch (tokenType) {
            case Token::INT:
                type = INT;
                break;
            case Token::CHAR:
                type = CHAR;
                break;
            case Token::VOID:
                type = VOID;
                break;

            default:
                throw syntax_error("Unexpected error occured");
                break;
        }
    }
};

struct Return
{
    Token value;
    Return (Token t) : value(t) {}

    void accept(Visitor &visitor) {
        visitor.visit(*this);
    }
};

struct VariableDecl
{
    DataType type;
    Token value;
    std::string name;

    VariableDecl(DataType type, Token value, std::string name)
    : type(type), value(value), name(name) {}

    void accept(Visitor &visitor) {
        visitor.visit(*this);
    }
};

struct Statement
{
    enum Type { RETURN, VARIABLE_DECL } type;
    std::variant<Return, VariableDecl> obj;

    Statement (Return rtn)
    : obj(rtn), type(RETURN) {}

    Statement (VariableDecl vb)
    : obj(vb), type(VARIABLE_DECL) {}

    void accept(Visitor &visitor) {
        visitor.visit(*this);
    }
};

struct Function
{
    DataType type;
    std::string name;
    std::vector<Statement> statements;

    Function (DataType t, std::string n, std::vector<Statement> stmts)
    : name(n), statements(stmts), type(t) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

struct Declaration
{
    enum Type { FUNCTION, VARIABLE } type;
    std::variant<Function, VariableDecl> obj;

    Declaration(Function f)
    : obj(f), type(FUNCTION) {}

    Declaration(VariableDecl vb)
    : obj(vb), type(FUNCTION) {}
    
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
