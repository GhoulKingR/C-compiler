#include "nodes.h"

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

void statement_insert(struct m_vector* vec, struct statement data) {
    if (vec->_size + 1 >= vec->_capacity)
        vector_resize(vec, vec->_capacity + 100);

    ((struct statement*) vec->_data)[vec->_size] = data;
    vec->_size++;
}

struct statement statement_at(struct m_vector *vec, int pos) {
    return ((struct statement*) vec->_data)[pos];
}

void declaration_insert(struct m_vector* vec, struct declaration data) {
    if (vec->_size + 1 >= vec->_capacity)
        vector_resize(vec, vec->_capacity + 100);

    ((struct declaration*) vec->_data)[vec->_size] = data;
    vec->_size++;
}

struct declaration declaration_at(struct m_vector *vec, int pos) {
    return ((struct declaration*) vec->_data)[pos];
}
