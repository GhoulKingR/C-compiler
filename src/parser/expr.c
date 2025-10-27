#include "nodes.h"
#include "parser.h"

void cleanupExpression(struct Expr *expr) {
    switch (expr->type) {
        case EXPR_UNARY_OPERATION:
            cleanupExpression(expr->obj.unary.value);
            free(expr);
            break;

        case EXPR_PRIMARY:
            free(expr);
            break;

        case EXPR_BINARY_OPERATION:
            cleanupExpression(expr->obj.binary.left);
            cleanupExpression(expr->obj.binary.right);
            free(expr);
            break;
    }
}

/**
 * Grammar for expressions:
 * expression     → equality ;
 * equality       → comparison ( ( "!=" | "==" ) comparison )* ;
 * comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
 * term           → factor ( ( "-" | "+" ) factor )* ;
 * factor         → unary ( ( "/" | "*" ) unary )* ;
 * unary          → ( "!" | "-" ) unary
 *                  | primary ;
 * primary        → NUMBER | STRING | "true" | "false" | "nil"
 *                  | IDENTIFIER | "(" expression ")" ;
 */

static struct Expr *parsePrimary(struct global_vars *vars) {
    struct Expr *result = NULL;

    if (check(TOKEN_CONSTANT, vars) || check(TOKEN_IDENTIFIER, vars)) {
        result = malloc(sizeof(struct Expr));
        result->type = EXPR_PRIMARY;
        result->obj.primary.value = token_at(vars->tokens, vars->progress);
        vars->progress++;
        return result;
    } else if (check(TOKEN_OPEN_PARENTHESIS, vars)) {
        vars->progress++;

        result = parseExpression(vars);
        if (result == NULL) return NULL;

        if (check(TOKEN_CLOSE_PARENTHESIS, vars)) {
            vars->progress++;
        } else {
            cleanupExpression(result);
            fprintf(stderr, "Syntax error on line %d, missing closing parenthesis ')'\n",
                token_at(vars->tokens, vars->progress).line);
            return NULL;
        }

        return result;
    } else {
        fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
            token_at(vars->tokens, vars->progress).line,
            token_at(vars->tokens, vars->progress).value);
        return NULL;
    }
}

static struct Expr *parseUnary(struct global_vars *vars) {
    if (check(TOKEN_MINUS, vars) || check(TOKEN_BANG, vars)  || check(TOKEN_TILDA, vars)) {
        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_UNARY_OPERATION;
        result->obj.unary.prefix = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *value = parseUnary(vars);
        if (result == NULL) {
            free(result);
            return NULL;
        }

        result->obj.unary.value = value;
        return result;
    } else {
        return parsePrimary(vars);
    }
}

static struct Expr *parseFactor(struct global_vars *vars) {
    struct Expr *left = parseUnary(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_SLASH, vars) || check(TOKEN_STAR, vars)) {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = parseUnary(vars);
        if (right == NULL) goto cleanup;

        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_BINARY_OPERATION;
        result->obj.binary.left = left;
        result->obj.binary.right = right;
        result->obj.binary.operation = operation;

        left = result;
    }

    return left;

cleanup:
    cleanupExpression(left);
    return NULL;
}

static struct Expr *parseTerm(struct global_vars *vars) {
    struct Expr *left = parseFactor(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_MINUS, vars) || check(TOKEN_PLUS, vars)) {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = parseFactor(vars);
        if (right == NULL) goto cleanup;

        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_BINARY_OPERATION;
        result->obj.binary.left = left;
        result->obj.binary.right = right;
        result->obj.binary.operation = operation;

        left = result;
    }

    return left;

cleanup:
    cleanupExpression(left);
    return NULL;
}

static struct Expr *parseComparison(struct global_vars *vars) {
    struct Expr *left = parseTerm(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_GREATER_THAN, vars) ||
        check(TOKEN_GREATER_THAN_EQUAL, vars) ||
        check(TOKEN_LESS_THAN, vars) ||
        check(TOKEN_LESS_THAN_EQUAL, vars))
    {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = parseTerm(vars);
        if (right == NULL) goto cleanup;

        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_BINARY_OPERATION;
        result->obj.binary.left = left;
        result->obj.binary.right = right;
        result->obj.binary.operation = operation;

        left = result;
    }

    return left;

cleanup:
    cleanupExpression(left);
    return NULL;
}

static struct Expr *parseEquality(struct global_vars *vars) {
    struct Expr *left = parseComparison(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_BANG_EQUAL, vars) || check(TOKEN_EQUAL_EQUAL, vars))
    {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = parseComparison(vars);
        if (right == NULL) goto cleanup;

        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_BINARY_OPERATION;
        result->obj.binary.left = left;
        result->obj.binary.right = right;
        result->obj.binary.operation = operation;

        left = result;
    }

    return left;

cleanup:
    cleanupExpression(left);
    return NULL;
}

static struct Expr *parseBitOpers(struct global_vars *vars) {
    struct Expr *left = parseEquality(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_AND, vars) || check(TOKEN_PIPE, vars)) {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = parseEquality(vars);
        if (right == NULL) goto cleanup;

        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_BINARY_OPERATION;
        result->obj.binary.left = left;
        result->obj.binary.right = right;
        result->obj.binary.operation = operation;

        left = result;
    }

    return left;

cleanup:
    cleanupExpression(left);
    return NULL;
}

struct Expr *parseExpression(struct global_vars *vars) {
    return parseEquality(vars);
}
