#include "helpers.h"

int digit_count(int d) {
    int i;
    for (i = 0; d > 0; i++, d /= 10); 
    return i == 0 ? 1 : i;
}

/**
 * Convert int to string. Result is allocated so remember
 * to free.
 */
char *int_to_str(int value) {
    int count = digit_count(value) + 1;
    char *result = (char*) malloc(count);
    memset(result, 0, count);
    snprintf(result, count, "%d", value);
    return result;
}

char *str_sub(const char *root, int start, size_t size) {
    char *result = (char*) malloc(size + 1);
    memcpy(result, root + start, size);
    result[size] = '\0';
    return result;
}

bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_alnum(char c) {
    return is_digit(c) || is_alpha(c);
}

/**
 * Copies a char array. Result is allocated so remember 
 * to free.
 */
char *str_copy(const char *from) {
    int memsize = strlen(from) + 1;
    char *result = (char*) malloc(memsize);
    memcpy(result, from, memsize);
    return result;
}

/** initialize a vector instance */
struct m_vector *vector_init(int _membsize) {
    struct m_vector *arr = (struct m_vector*) malloc(sizeof(struct m_vector));
    arr->_size = 0;
    arr->_membsize = _membsize;
    arr->_capacity = 100;
    arr->_data = malloc(100 * _membsize);
    return arr;
}

/**
 * Free the vector in memory.
 * Note: this function doesn't free internally allocated data.
 **/
void vector_free(struct m_vector *vec) {
    free(vec->_data);
    free(vec);
}

void vector_resize(struct m_vector *vec, size_t size) {
    size_t min = size < vec->_capacity ? size : vec->_capacity;
    void* new_data = malloc(vec->_membsize * size);
    memcpy(new_data, vec->_data, min);
    free(vec->_data);
    vec->_data = new_data;
}
