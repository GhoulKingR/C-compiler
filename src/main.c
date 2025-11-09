#include <stdlib.h>

#include "lexer/token.h"
#include "parser/nodes.h"
#include "helpers.h"

// from lexer/lexer.c
struct m_vector *lexer(const char *content);

// from parser/parser.c
void parser_cleanup(struct program *p);
struct program *parse(struct m_vector *tokens);

// from target_arm.c
const char* arm_compile(struct program *p);

char* read_file(const char *c_file) {
    FILE *fptr = fopen(c_file, "r");
    char buffer[100];
    memset(buffer, 0, 100);

    char *result = (char *) malloc(100);
    int capacity = 100;
    int size = 0;

    while (fgets(buffer, 100, fptr)) {
        int bytes = strlen(buffer);

        while (size + bytes >= capacity) {
            char *newaddr = (char*) malloc(capacity + 100);
            memcpy(newaddr, result, capacity);
            capacity += 100;
            free(result);
            result = newaddr;
        }

        memcpy(result + size, buffer, bytes);
        size += bytes;
        memset(buffer, 0, 100);
    }

    result[size] = '\0';
    fclose(fptr);
    return result;
}

int main(int argc, char** argv)
{
    // get c file from arguments
#ifndef DEBUG
    if (argc < 2)
    {
        std::cerr << "You need to provide a c path." << std::endl;
        return EXIT_FAILURE;
    }
    const char* c_file = argv[argc - 1];
#else
    const char* c_file = argc < 2 ? "./sample/main.c" : argv[argc - 1];
#endif

    int exit_signal = EXIT_SUCCESS;
    const char* assembly;
    struct program *p;

    // read c file
    char* content = read_file(c_file);

    // pass it to the lexer
    struct m_vector *tokens = lexer(content);
    if (tokens == NULL) {
        exit_signal = EXIT_FAILURE;
        goto file_cleanup;
    }

    // parse the tokens
    p = parse(tokens);
    if (p == NULL) {
        exit_signal = EXIT_FAILURE;
        goto lexer_cleanup;
    }

    // convert program to assembly
    assembly = arm_compile(p);
    if (assembly == NULL) goto parser_clean;
    
    // write assembly to file
    FILE *fptr = fopen("./sample/output.s", "w");
    fputs(assembly, fptr);
    fclose(fptr);

    // cleanup process
    free((void*) assembly);

parser_clean:
    parser_cleanup(p);
lexer_cleanup:
    token_cleanup(tokens);
file_cleanup:
    free(content);
    return exit_signal;
}
