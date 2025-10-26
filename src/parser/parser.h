#pragma once

#include "nodes.h"

/**
 * Only used by the parser code. This should not be imported anywhere else
 */

struct global_vars {
    struct m_vector *tokens; /* struct token */
    int progress;
};

// source: parser.c
bool check(enum token_type expected, struct global_vars *vars);

// source: expr.c
extern struct Expr *parseExpression(struct global_vars *vars);
void cleanupExpression(struct Expr *expr);
