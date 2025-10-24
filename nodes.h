#pragma once
#include "token.h"

/** AST nodes definitions ***/
enum datatype {
    DATATYPE_INT,
    DATATYPE_CHAR,
    DATATYPE_VOID,
    DATATYPE_ERR,
};

int datatype_size (enum datatype type) {
    switch (type) {
        case DATATYPE_INT:
            return 4;
        case DATATYPE_CHAR:
            return 1;

        default:
            return 0;   // error value
    }
}

enum datatype token_to_datatype(enum token_type tokenType) {
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

enum expr_type {
    EXPR_CONSTANT,
    EXPR_IDENTIFIER,

    // operations
    // EXPR_UNARY_OPERATION,
};

struct Expr
{
    enum expr_type type;
    const char* value;
};

/** return <expr>; */
struct Return
{
    struct Expr value;
};

/** <datatype> variable_name = <expr> */
struct variable_decl
{
    enum datatype type;
    struct Expr value;
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

struct statement statement_at(struct m_vector *vec, int pos) {
    return ((struct statement*) vec->_data)[pos];
}
/* statement end */

struct function
{
    enum datatype type;
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

struct declaration declaration_at(struct m_vector *vec, int pos) {
    return ((struct declaration*) vec->_data)[pos];
}
/* declarations end */

struct program
{
    struct m_vector *declarations;     // type: <Declaration>
};

// <identifier> : IDENTIFIER
// <int>        : CONSTANT

/** AST nodes definitions end **/
