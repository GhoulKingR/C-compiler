#include "parser.h"

struct Expr *parseExpression(struct global_vars *vars) {
    struct Expr *result = malloc(sizeof(struct Expr));

    if (check(TOKEN_MINUS, vars) || check(TOKEN_BANG, vars)  || check(TOKEN_TILDA, vars)) {
        result->obj.sin.prefix.with_operation = true;
        result->obj.sin.prefix.tk = token_at(vars->tokens, vars->progress);
        vars->progress++;
    } else {
        result->obj.sin.prefix.with_operation = false;
    }
    
    if (check(TOKEN_CONSTANT, vars)) {
        struct token constant = token_at(vars->tokens, vars->progress);
        result->type = EXPR_CONSTANT;
        result->obj.sin.value = constant.value;
        vars->progress++;
    } else if (check(TOKEN_IDENTIFIER, vars)) {
        struct token constant = token_at(vars->tokens, vars->progress);
        result->type = EXPR_IDENTIFIER;
        result->obj.sin.value = constant.value;
        vars->progress++;
    } else goto syntax_error;

    return result;

syntax_error:
    fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
        token_at(vars->tokens, vars->progress).line,
        token_at(vars->tokens, vars->progress).value);
    free(result);
    return NULL;
}
