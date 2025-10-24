#pragma once

#include "nodes.h"
#include "token.h"

void parser_cleanup(struct program *p);
struct program *parse(struct m_vector *tokens);
