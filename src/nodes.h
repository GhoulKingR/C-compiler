#pragma once
#include "token.h"

enum datatype {
    DATATYPE_INT,
    DATATYPE_CHAR,
    DATATYPE_VOID,
    DATATYPE_ERR,
};

int datatype_size (enum datatype type);
enum datatype token_to_datatype(enum token_type tokenType);

enum expr_type {
    EXPR_CONSTANT,
    EXPR_IDENTIFIER,
};

struct Expr
{
    enum expr_type type;

    struct {
        bool with_operation;
        struct token tk;
    } prefix;

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
    struct Expr *value;
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
};

void statement_insert(struct m_vector* vec, struct statement data);
struct statement statement_at(struct m_vector *vec, int pos);
/* statement end */

struct function
{
    enum datatype type;
    const char* name;
    struct m_vector *statements; //: struct statement
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

void declaration_insert(struct m_vector* vec, struct declaration data);
struct declaration declaration_at(struct m_vector *vec, int pos);
/* declarations end */

struct program
{
    struct m_vector *declarations;     //: struct declaration
};

// <identifier> : IDENTIFIER
// <int>        : CONSTANT

/** AST nodes definitions end **/
