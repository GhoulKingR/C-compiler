#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "exceptions.hpp"
#include "nodes.hpp"
#include "token.h"

struct global_vars {
    struct m_vector *tokens; /* struct token */
    int progress;
};

// checkes whether the token at the current position matches 
// the expected type
bool check(enum token_type expected, struct global_vars *vars) {
    return (
        expected == token_at(vars->tokens, vars->progress).type
    );
}

/****
 * parse the current token for the data type
 * type :: "void" | "int" | "char"
 */
struct datatype parseDataType(struct global_vars *vars)
{
    if (check(TOKEN_INT, vars) || check(TOKEN_VOID, vars) || check(TOKEN_CHAR, vars)) {
        enum datatype_type type = token_to_datatype(
            token_at(vars->tokens, vars->progress).type);

        vars->progress++;
        return (struct datatype) { .type = type };
    }

    fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
        token_at(vars->tokens, vars->progress).line,
        token_at(vars->tokens, vars->progress).value);
    exit(EXIT_FAILURE);
}

/****
 * statement :: <return> | <variable_decl>
 */
struct m_vector *parseStatements(struct global_vars *vars) /* Statement */
{
    struct m_vector *statements = vector_init(sizeof(struct statement));

    while (!check(TOKEN_CLOSE_BRACE, vars))
    {
        /* return :: "return " (CONSTANT | IDENTIFIER) ";" */
        if (check(TOKEN_RETURN, vars)) {
            vars->progress++;


            if (check(TOKEN_CONSTANT, vars)) {
                statement_insert(statements, (struct statement) {
                    .type = STATEMENT_RETURN,
                    .obj = {
                        .ret = {
                            .value = {
                                .type = EXPR_CONSTANT,
                                .value = token_at(vars->tokens, vars->progress).value,
                            }
                        }
                    }
                });
            } else if (check(TOKEN_IDENTIFIER, vars)) {
                statement_insert(statements, (struct statement) {
                    .type = STATEMENT_RETURN,
                    .obj = {
                        .ret = {
                            .value = {
                                .type = EXPR_IDENTIFIER,
                                .value = token_at(vars->tokens, vars->progress).value,
                            }
                        }
                    }
                });
            }
            vars->progress++;

            if (check(TOKEN_SEMICOLON, vars)) vars->progress++;
                else goto syntax_error;

        /* variable_decl :: <type> IDENTIFIER "=" CONSTANT ";" */
        } else {

            // get variable declaration data type
            struct datatype type = parseDataType(vars);

            // get variable declaration identifier (variable name)
            struct token identifier;
            if (check(TOKEN_IDENTIFIER, vars)) {
                identifier = token_at(vars->tokens, vars->progress);
                vars->progress++;
            } else goto syntax_error;

            // '='
            if (check(TOKEN_EQUAL, vars)) vars->progress++;
                else goto syntax_error;
            
            // get constant to store the variable name in 
            struct token constant;
            if (check(TOKEN_CONSTANT, vars)) {
                constant = token_at(vars->tokens, vars->progress);
                vars->progress++;
            } else goto syntax_error;

            if (check(TOKEN_SEMICOLON, vars)) vars->progress++;
                else goto syntax_error;
            
            statement_insert(statements, (struct statement) {
                .type = STATEMENT_VARIABLE_DECL,
                .obj = {
                    .var = {
                        .name = identifier.value,
                        .type = type,
                        .value = {
                            .type = EXPR_CONSTANT,
                            .value = constant.value,
                        },
                    }
                }
            });
        }
    }

    return statements;

syntax_error:
    fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
        token_at(vars->tokens, vars->progress).line,
        token_at(vars->tokens, vars->progress).value);
    exit(EXIT_FAILURE);
}

/****
 * function_declaration
 * :: <type> IDENTIFIER "(void){" <statement>* "}"
 * :: Function ( type = <type>, name = IDENTIFIER, body = [ <statement>* ] )
 */
struct m_vector *parseDeclarations(struct global_vars *vars)
{
    struct m_vector *declarations /* Declaration */;
    declarations = vector_init(sizeof(struct declaration));

    while (!check(TOKEN_EOF, vars))
    {
        // get datatype of the declaration
        struct datatype type = parseDataType(vars);

        // get declaration name/identifier
        struct token identifier;
        if (check(TOKEN_IDENTIFIER, vars)) {
            identifier = token_at(vars->tokens, vars->progress);
            vars->progress++;
        } else goto syntax_error;

        if (check(TOKEN_OPEN_PARENTHESIS, vars)) {
            vars->progress++;
            // function declaration
            // (void){
            if (check(TOKEN_VOID, vars)) vars->progress++;      // optional `void`
            if (check(TOKEN_CLOSE_PARENTHESIS, vars)) vars->progress++;
                else goto syntax_error;
            if (check(TOKEN_OPEN_BRACE, vars)) vars->progress++;
                else goto syntax_error;

            // parse all statements inside the function
            struct m_vector *stmts = parseStatements(vars);

            // }
            if (check(TOKEN_CLOSE_BRACE, vars)) vars->progress++;
                else goto syntax_error;

            declaration_insert(declarations, (struct declaration) {
                .type = DECLARATION_FUNCTION,
                .obj = {
                    .func = {
                        .name = identifier.value,
                        .type = type,
                        .statements = stmts,
                    }
                }
            });
        } else if (check(TOKEN_EQUAL, vars)) {
            // variable declaration
            vars->progress++;

            // get number after `=`
            struct token constant;
            if (check(TOKEN_CONSTANT, vars)) {
                constant = token_at(vars->tokens, vars->progress);
                vars->progress++;
            } else goto syntax_error;

            // check for semicolon
            if (check(TOKEN_SEMICOLON, vars)) vars->progress++;
                else goto syntax_error;

            declaration_insert(declarations, (struct declaration) {
                .type = DECLARATION_VARIABLE,
                .obj = {
                    .var = {
                        .type = type,
                        .value = {
                            .value = constant.value,
                            .type = EXPR_CONSTANT,
                        },
                        .name = identifier.value,
                    }
                }
            });
        } else goto syntax_error;
    }

    return declarations;

syntax_error:
    fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
        token_at(vars->tokens, vars->progress).line,
        token_at(vars->tokens, vars->progress).value);
    exit(EXIT_FAILURE);
}

/****
 *  program ::  <declaration>*  :: Program( declarations = [ <declaration>* ] )
 */
struct program parse(struct m_vector /* struct token */*tokens) {
    struct global_vars vars = {
        .tokens = tokens,
        .progress = 0,
    };

    return (struct program) {
        .declarations = parseDeclarations(&vars),
    };
}
