#include "nodes.h"
#include "parser.h"
#include <stdlib.h>

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

        case EXPR_TERNARY_OPERATION:
            cleanupExpression(expr->obj.ternary.condition);
            cleanupExpression(expr->obj.ternary.left);
            cleanupExpression(expr->obj.ternary.right);
            free(expr);
            break;

        case EXPR_ASSIGNMENT:
            cleanupExpression(expr->obj.assignment.right);
            free(expr);
            break;
    }
}

/**
 * Grammar for expressions:
 * expression      → precedence14 ;
 * precedence14    → IDENTIFIER ( "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "<<=" | ">>=" | "&=" | "^=" | "|=" ) precedence13
 *                 | precedence13 ;
 * precedence13    → precedence12 ( "?" precedence12 ":" precedence12 )* ;
 * precedence12    → precedence11 ( "||" precedence11 )* ;
 * precedence11    → precedence10 ( "&&" precedence10 )* ;
 * precedence10    → precedence9 ( "|" precedence9 )* ;
 * precedence9     → precedence8 ( "^" precedence8 )* ;
 * precedence8     → precedence7 ( "&" precedence7 )* ;
 * precedence7     → precedence6 ( ( "!=" | "==" ) precedence6 )* ;
 * precedence6     → precedence5 ( ( ">" | ">=" | "<" | "<=" ) precedence5 )* ;
 * precedence5     → precedence4 ( ( "<<" | ">>" ) precedence4 )* ;
 * precedence4     → precedence3 ( ( "+" | "-" ) precedence3 )* ;
 * precedence3     → precedence2 ( ( "/" | "*" | "%" ) precedence2 )* ;
 * precedence2     → ("++" | "--" | "+" | "-" | "!" | "~" ) precedence2
 *                 | precedence1 ;
 * precedence1     → primary ("--" | "++")* ;
 * primary         → NUMBER | CHAR | STRING | IDENTIFIER | "(" expression ")" ;
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

static struct Expr *precedence1(struct global_vars *vars) {
    struct Expr *primary = parsePrimary(vars);
    if (primary == NULL) return NULL;

    while (check(TOKEN_MINUS_MINUS, vars) || check(TOKEN_PLUS_PLUS, vars)) {
        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_UNARY_OPERATION;
        result->obj.unary.prefix = token_at(vars->tokens, vars->progress);
        result->obj.unary.value = primary;
        vars->progress++;

        primary = result;
    }

    return primary;
}

static struct Expr *precedence2(struct global_vars *vars) {
    if (check(TOKEN_MINUS, vars)        ||
        check(TOKEN_BANG, vars)         ||
        check(TOKEN_TILDA, vars)        ||
        check(TOKEN_PLUS_PLUS, vars)    ||
        check(TOKEN_MINUS_MINUS, vars)  ||
        check(TOKEN_PLUS, vars))
    {
        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_UNARY_OPERATION;
        result->obj.unary.prefix = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *value = precedence1(vars);
        if (result == NULL) {
            free(result);
            return NULL;
        }

        result->obj.unary.value = value;
        return result;
    } else {
        return precedence1(vars);
    }
}

static struct Expr *precedence3(struct global_vars *vars) {
    struct Expr *left = precedence2(vars);
    if (left == NULL) return NULL;

    while ( check(TOKEN_SLASH, vars)    ||
            check(TOKEN_STAR, vars)     ||
            check(TOKEN_PERCENT, vars))
    {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence2(vars);
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

static struct Expr *precedence4(struct global_vars *vars) {
    struct Expr *left = precedence3(vars);
    if (left == NULL) return NULL;

    while ( check(TOKEN_PLUS, vars)     ||
            check(TOKEN_MINUS, vars))
    {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence3(vars);
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

static struct Expr *precedence5(struct global_vars *vars) {
    struct Expr *left = precedence4(vars);
    if (left == NULL) return NULL;

    while ( check(TOKEN_LT_LT, vars)     ||
            check(TOKEN_GT_GT, vars))
    {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence4(vars);
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

static struct Expr *precedence6(struct global_vars *vars) {
    struct Expr *left = precedence5(vars);
    if (left == NULL) return NULL;

    while ( check(TOKEN_GREATER_THAN, vars)         ||
            check(TOKEN_GREATER_THAN_EQUAL, vars)   ||
            check(TOKEN_LESS_THAN, vars)            ||
            check(TOKEN_LESS_THAN_EQUAL, vars))
    {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence5(vars);
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

static struct Expr *precedence7(struct global_vars *vars) {
    struct Expr *left = precedence6(vars);
    if (left == NULL) return NULL;

    while ( check(TOKEN_BANG_EQUAL, vars)   ||
            check(TOKEN_EQUAL_EQUAL, vars))
    {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence6(vars);
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

static struct Expr *precedence8(struct global_vars *vars) {
    struct Expr *left = precedence7(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_AND, vars)) {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence7(vars);
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

static struct Expr *precedence9(struct global_vars *vars) {
    struct Expr *left = precedence8(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_CARET, vars)) {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence8(vars);
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

static struct Expr *precedence10(struct global_vars *vars) {
    struct Expr *left = precedence9(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_PIPE, vars)) {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence9(vars);
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

static struct Expr *precedence11(struct global_vars *vars) {
    struct Expr *left = precedence10(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_AND_AND, vars)) {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence10(vars);
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

static struct Expr *precedence12(struct global_vars *vars) {
    struct Expr *left = precedence11(vars);
    if (left == NULL) return NULL;

    while (check(TOKEN_PIPE_PIPE, vars)) {
        struct token operation = token_at(vars->tokens, vars->progress);
        vars->progress++;

        struct Expr *right = precedence11(vars);
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

static struct Expr *precedence13(struct global_vars *vars) {
    struct Expr *cond = precedence12(vars);
    if (cond == NULL) return NULL;

    while (check(TOKEN_QUESTION_MARK, vars)) {
        vars->progress++;

        struct Expr *left = precedence12(vars);
        if (left == NULL) goto cleanup;

        if (check(TOKEN_COLON, vars)) {
            vars->progress++;
        } else {
            fprintf(stderr, "Syntax error on line %d, unexpected '%s'\n",
                token_at(vars->tokens, vars->progress).line,
                token_at(vars->tokens, vars->progress).value);
            cleanupExpression(left);
            goto cleanup;
        }

        struct Expr *right = precedence12(vars);
        if (left == NULL) {
            cleanupExpression(left);
            goto cleanup;
        }

        struct Expr *result = malloc(sizeof(struct Expr));
        result->type = EXPR_TERNARY_OPERATION;
        result->obj.ternary.condition = cond;
        result->obj.ternary.left = left;
        result->obj.ternary.right = right;

        cond = result;
    }

    return cond;

cleanup:
    cleanupExpression(cond);
    return NULL;
}

static struct Expr *precedence14(struct global_vars *vars) {
    struct Expr *result = NULL;

    if (token_at(vars->tokens, vars->progress).type ==  TOKEN_IDENTIFIER) {
        switch (token_at(vars->tokens, vars->progress + 1).type) {
            case TOKEN_EQUAL:
            case TOKEN_PLUS_EQUAL:
            case TOKEN_MINUS_EQUAL:
            case TOKEN_STAR_EQUAL:
            case TOKEN_SLASH_EQUAL:
            case TOKEN_PERCENT_EQUAL:
            case TOKEN_LT_LT_EQUAL:
            case TOKEN_GT_GT_EQUAL:
            case TOKEN_CARET_EQUAL:
            case TOKEN_PIPE_EQUAL:
                result = malloc(sizeof(struct Expr));
                result->type = EXPR_ASSIGNMENT;
                result->obj.assignment.name = token_at(vars->tokens, vars->progress);
                vars->progress++;
                result->obj.assignment.operation = token_at(vars->tokens, vars->progress);
                vars->progress++;
                result->obj.assignment.right = precedence13(vars);
                if (result->obj.assignment.right == NULL) {
                    free(result);
                    return NULL;
                }
                return result;

            default:
                return precedence13(vars);
        }

    } else return precedence13(vars);
}

struct Expr *parseExpression(struct global_vars *vars) {
    return precedence14(vars);
}
