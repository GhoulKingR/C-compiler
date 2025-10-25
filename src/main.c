#include "token.h"
#include "nodes.h"
#include "helpers.h"

// from parser.c
void parser_cleanup(struct program *p);
struct program *parse(struct m_vector *tokens);

// from target_arm.c
const char* arm_compile(struct program *p);

#define DEBUG

int seek_identifier(int start, const char *content, struct m_vector* tokens, int line)
{
    // seek to the end of the identifier/keyword
    int end = start;
    while (is_alnum(content[end]))
        end++;

    const int size = end - start;
    char *token_text = str_sub(content, start, size);
    enum token_type token_type = token_get_keyword(token_text);

    // check whether the text gotten is either a keyword or identifier
    if (token_type != TOKEN_NOT_KEYWORD)
    {
        token_insert(tokens, (struct token) {
            .type = token_type,
            .value = token_text,
            .line = line,
            .allocated = true,
        });
    }
    else
    {
        token_insert(tokens, (struct token) {
            .type = TOKEN_IDENTIFIER,
            .value = token_text,
            .line = line,
            .allocated = true
        });
    }

    return end;
}

int seek_constant(int start, const char *content, struct m_vector* tokens, int line)
{
    // seek to the end of the constant
    int end = start;
    while (is_digit(content[end]))
        end++;
    
    const int size = end - start;
    char *token_text = str_sub(content, start, size);

    // push it to the token list
    token_insert(tokens, (struct token) {
        .type = TOKEN_CONSTANT,
        .value = token_text,
        .line = line,
        .allocated = true,
    });

    return end;
}

struct m_vector *lexer(const char *content)
{
    struct m_vector *tokens = vector_init(sizeof(struct token));
    int current = 0;
    int line = 1;
    size_t size = strlen(content);

    while (current < size)
    {
        switch (content[current])
        {
            // symbols
            case '(':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_OPEN_PARENTHESIS,
                    .value = "(",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case ')':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_CLOSE_PARENTHESIS,
                    .value = ")",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '{':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_OPEN_BRACE,
                    .value = "{",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '}':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_CLOSE_BRACE,
                    .value = "{",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case ';':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_SEMICOLON,
                    .value = "{",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '=':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_EQUAL,
                    .value = "=",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '!':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_BANG,
                    .value = "!",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '>':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_GREATER_THAN,
                    .value = ">",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '<':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_LESS_THAN,
                    .value = "<",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '-':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_MINUS,
                    .value = "-",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '+':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_PLUS,
                    .value = "+",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '/':
                current++;
                if (content[current] == '/') {
                    current++;
                    for (; content[current] != '\n' && content[current] != '\0'; current++);
                } else if (content[current] == '*') {
                    current++;
                    int starting_line = line;

                    while (true) {
                        if (content[current] == '\0') {
                            fprintf(stderr,
                                "Unexpected EOF symbol, missing comment terminator '*/' for line %d\n",
                                starting_line);
                            goto cleanup;
                        } else if (content[current] == '\n') {
                            line++;
                            current++;
                        } else if (content[current] == '*') {
                            current++;

                            if (content[current] == '/') {
                                current++;
                                break;
                            }
                        } else current++;
                    }
                } else {
                    token_insert(tokens, (struct token) {
                        .type = TOKEN_SLASH,
                        .value = "/",
                        .line = line,
                        .allocated = false,
                    });
                    break;
                }
                break;
            case '~':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_TILDA,
                    .value = "~",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '*':
                token_insert(tokens, (struct token) {
                    .type = TOKEN_STAR,
                    .value = "*",
                    .line = line,
                    .allocated = false,
                });
                current++;
                break;
            case '\'':
                current++;
                token_insert(tokens, (struct token) {
                    .type = TOKEN_CONSTANT, 
                    .value = int_to_str((int) content[current]),
                    .line = line,
                    .allocated = true,
                });
                    
                if (content[++current] == '\'') {
                    current++;
                } else {
                    goto syntax_error;
                }
                break;

            // skips
            case '\n':
                current++;
                line++;
                break;

            case '\r':
            case '\t':
            case ' ':
                current++;
                break;

            default:
                // identifier
                if (is_alpha(content[current]))
                    current = seek_identifier(current, content, tokens, line);
                // constants
                else if (is_digit(content[current]))
                    current = seek_constant(current, content, tokens, line);
                else goto syntax_error;
                break;
        }
    }

    token_insert(tokens, (struct token) {
        .type = TOKEN_EOF,
        .value = "EOF",
        .line = line
    });

    return tokens;
    
syntax_error:
    fprintf(stderr,
        "Syntax error on line %d, unexpected character '%c'\n",
        line, content[current]
    );

cleanup:
    token_cleanup(tokens);
    return NULL;
}

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
