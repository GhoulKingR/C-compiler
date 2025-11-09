#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/**
 * Convert int to string. Result is allocated so remember
 * to free.
 */
char *int_to_str(int value);
char *str_sub(const char *root, int start, size_t size);
bool is_digit(char c);
bool is_hex(char c);
bool is_alpha(char c);
bool is_alnum(char c);
int digit_count(int d);

/**
 * Copies a char array. Result is allocated so remember 
 * to free.
 */
char *str_copy(const char *from);

/** custom vector implementation */
struct m_vector {
    void* _data;
    int _membsize;
    int _size;
    int _capacity;
};

/** initialize a vector instance */
struct m_vector *vector_init(int _membsize);

/**
 * Free the vector in memory.
 * Note: this function doesn't free internally allocated data.
 **/
void vector_free(struct m_vector *vec);
void vector_resize(struct m_vector *vec, size_t size);
