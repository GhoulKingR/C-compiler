#pragma once

#include <string>
#include <variant>
#include <vector>

#include "exceptions.hpp"
#include "token.h"

class Visitor;

/** AST nodes definitions ***/
enum datatype_type {
    DATATYPE_INT,
    DATATYPE_CHAR,
    DATATYPE_VOID,
    DATATYPE_ERR,
};

struct datatype {
    enum datatype_type type;

    // int size() {
    //     switch (type) {
    //         case INT:
    //             return 4;
    //         case CHAR:
    //             return 1;

    //         default:
    //             throw syntax_error("Unexpected error occured");
    //             break;
    //     }

    // }
};

enum datatype_type token_to_datatype(enum token_type tokenType) {
    switch (tokenType) {
        case TOKEN_INT:
            return DATATYPE_INT;
        case TOKEN_CHAR:
            return DATATYPE_CHAR;
        case TOKEN_VOID:
            return DATATYPE_VOID;

        default:        // unexpected error
            return DATATYPE_ERR;
    }
}

// TODO: Implement this
enum expr_type {
    EXPR_CONSTANT,
    EXPR_IDENTIFIER,
    
    EXPR_ERR,
};

struct Expr
{
    enum expr_type type;
    const char* value;
};

/** return <expr>; */
struct Return
{
    Expr value;
};

/** <datatype> variable_name = <expr> */
struct variable_decl
{
    struct datatype type;
    Expr value;
    const char* name;
};

/* statement start */
enum statement_type {
    STATEMENT_RETURN,
    STATEMENT_VARIABLE_DECL
};

struct statement
{
    enum statement_type type;
    union {
        struct Return ret;
        struct variable_decl var;
    } obj;

    // Statement (Return rtn)
    // : obj(rtn), type(RETURN) {}

    // Statement (VariableDecl vb)
    // : obj(vb), type(VARIABLE_DECL) {}
};

void statement_insert(struct m_vector* vec, struct statement data) {
    if (vec->_size + 1 >= vec->_capacity)
        vector_resize(vec, vec->_capacity + 100);

    ((struct statement*) vec->_data)[vec->_size] = data;
    vec->_size++;
}
/* statement end */

struct function
{
    struct datatype type;
    const char* name;
    struct m_vector *statements /* Statement */;
};

/* declarations start */
enum declaration_type {
    DECLARATION_FUNCTION,
    DECLARATION_VARIABLE
};

struct declaration
{
    enum declaration_type type;
    union {
        struct function func;
        struct variable_decl var;
    } obj;
};

void declaration_insert(struct m_vector* vec, struct declaration data) {
    if (vec->_size + 1 >= vec->_capacity)
        vector_resize(vec, vec->_capacity + 100);

    ((struct declaration*) vec->_data)[vec->_size] = data;
    vec->_size++;
}
/* declarations end */

struct program
{
    struct m_vector *declarations;     // type: <Declaration>
};

// <identifier> : IDENTIFIER
// <int>        : CONSTANT

/** AST nodes definitions end **/
