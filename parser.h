#pragma once

#include "nodes.h"
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
struct datatype *parseDataType(struct global_vars *vars)
{
    struct datatype *result = (struct datatype*) malloc(sizeof(struct datatype));

    if (check(TOKEN_INT, vars) || check(TOKEN_VOID, vars) || check(TOKEN_CHAR, vars)) {
        enum datatype_type type = token_to_datatype(
            token_at(vars->tokens, vars->progress).type);
        if (type == DATATYPE_ERR) goto syntax_error;

        vars->progress++;
        result->type = type;
        return result;
    }

syntax_error:
    fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
        token_at(vars->tokens, vars->progress).line,
        token_at(vars->tokens, vars->progress).value);
    free(result);
    return NULL;
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
            struct datatype *type = parseDataType(vars);
            if (type == NULL) goto cleanup;

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
                        .type = *type,
                        .value = {
                            .type = EXPR_CONSTANT,
                            .value = constant.value,
                        },
                    }
                }
            });

            free(type);
        }
    }

    return statements;

syntax_error:
    fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
        token_at(vars->tokens, vars->progress).line,
        token_at(vars->tokens, vars->progress).value);
    // cleanup statements vector

cleanup:
    vector_free(statements);
    return NULL;
}

/****
 * function_declaration
 * :: <type> IDENTIFIER "(void){" <statement>* "}"
 * :: Function ( type = <type>, name = IDENTIFIER, body = [ <statement>* ] )
 */
struct m_vector *parseDeclarations(struct global_vars *vars)
{
    struct m_vector *declarations = vector_init(sizeof(struct declaration));

    while (!check(TOKEN_EOF, vars))
    {
        // get datatype of the declaration
        struct datatype *type = parseDataType(vars);
        if (type == NULL) goto cleanup;

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
            if (stmts == NULL) goto cleanup;

            // }
            if (check(TOKEN_CLOSE_BRACE, vars)) vars->progress++;
                else goto syntax_error;

            declaration_insert(declarations, (struct declaration) {
                .type = DECLARATION_FUNCTION,
                .obj = {
                    .func = {
                        .name = identifier.value,
                        .type = *type,
                        .statements = stmts,
                    }
                }
            });
            
            free(type);
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
                        .type = *type,
                        .value = {
                            .value = constant.value,
                            .type = EXPR_CONSTANT,
                        },
                        .name = identifier.value,
                    }
                }
            });

            free(type);
        } else goto syntax_error;
    }

    return declarations;

syntax_error:
    fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
        token_at(vars->tokens, vars->progress).line,
        token_at(vars->tokens, vars->progress).value);

cleanup:
    vector_free(declarations);
    return NULL;
}

/****
 *  program ::  <declaration>*  :: Program( declarations = [ <declaration>* ] )
 */
struct program *parse(struct m_vector /* struct token */*tokens) {
    struct global_vars vars = {
        .tokens = tokens,
        .progress = 0,
    };
    struct m_vector *decls = parseDeclarations(&vars);

    if (decls == NULL) {
        return NULL;
    } else {
        struct program *result = (struct program*) malloc(sizeof(struct program));
        result->declarations = decls;
        return result;
    }
}

/** cleanup functions (only vectors are cleaned up here) */
void cleanup_declaration(struct declaration d) {
    switch (d.type)
    {
    case DECLARATION_FUNCTION:
        vector_free(d.obj.func.statements);
        break;
    
    default:
        break;
    }
}

// cleanup the parser data
void parser_cleanup(struct program *p) {
    // iterate over declarations
    for (int i = 0; i < p->declarations->_size; i++) {
        cleanup_declaration(
            ((struct declaration*) p->declarations->_data)[i]
        );
    }
    vector_free(p->declarations);
    free(p);
}