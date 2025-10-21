#pragma once

#include <string>
#include <variant>
#include <vector>

#include "exceptions.hpp"
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

// TODO: Implement this
struct Expr
{
    enum Type {
        CONSTANT, IDENTIFIER,
    } type;
    std::variant<std::string> value;

    Expr(Token t) : value(t.value) {
        switch (t.type) {
            case Token::CONSTANT:
                type = CONSTANT;
                break;
            case Token::IDENTIFIER:
                type = IDENTIFIER;
                break;

            default:
                // error handling
                break;
        }
    }

    template<typename T>
    T& getData() {
        return std::get<T>(value);
    }
};

/** return <expr>; */
struct Return
{
    Expr value;
    Return (Expr v) : value(v) {}
};

/** <datatype> variable_name = <expr> */
struct VariableDecl
{
    DataType type;
    Expr value;
    std::string name;

    VariableDecl(DataType type, Expr value, std::string name)
    : type(type), value(value), name(name) {}
};

struct Statement
{
    enum Type { RETURN, VARIABLE_DECL } type;
    std::variant<Return, VariableDecl> obj;

    Statement (Return rtn)
    : obj(rtn), type(RETURN) {}

    Statement (VariableDecl vb)
    : obj(vb), type(VARIABLE_DECL) {}
};

struct Function
{
    DataType type;
    std::string name;
    std::vector<Statement> statements;

    Function (DataType t, std::string n, std::vector<Statement> stmts)
    : name(n), statements(stmts), type(t) {}
};

struct Declaration
{
    enum Type { FUNCTION, VARIABLE } type;
    std::variant<Function, VariableDecl> obj;

    Declaration(Function f)
    : obj(f), type(FUNCTION) {}

    Declaration(VariableDecl vb)
    : obj(vb), type(FUNCTION) {}
};

struct Program
{
    std::vector<Declaration> declarations;
    Program (std::vector<Declaration> d) : declarations(d) {}
};

// <identifier> : IDENTIFIER
// <int>        : CONSTANT

/** AST nodes definitions end **/
