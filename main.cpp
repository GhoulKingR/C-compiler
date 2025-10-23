#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser.hpp"
// #include "targets/arm.hpp"
#include "token.h"
#include "exceptions.hpp"
#include "helpers.h"

#define DEBUG

void lexer(struct m_vector* tokens, std::string& content);

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

    // read c file
    std::ifstream f(c_file);
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string content = buffer.str();
    f.close();

    // pass it to the lexer
    struct m_vector* tokens = vector_init(sizeof(struct token));
    lexer(tokens, content);

    // parse the tokens
    struct program p = parse(tokens);

    // // convert program to assembly
    // ArmTarget compiler;
    
    // // write assembly to file
    // std::ofstream output("./sample/output.s");
    // output << compiler.compile(program);
    // output.close();

    for (int i = 0; i < tokens->_size; i++) {
        struct token t = ((struct token*) tokens->_data)[i];
        if (t.allocated) {
            free(t.value);
        }
    }
    vector_free(tokens);

    return 0;
}

int seek_identifier(int start, std::string& content, struct m_vector* tokens, int line);
int seek_constant(int start, std::string& content, struct m_vector* tokens, int line);

void lexer(struct m_vector* tokens, std::string& content)
{
    int current = 0;
    int line = 1;

    while (current < content.size())
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
                    std::stringstream ss;
                    ss  << "Syntax error on line " << line
                        << ", unexpected character \"" << content[current] << "\"";
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
                if (isalpha(content[current]))
                    current = seek_identifier(current, content, tokens, line);
                // constants
                else if (isdigit(content[current]))
                    current = seek_constant(current, content, tokens, line);
                else {
                    std::stringstream ss;
                    ss  << "Syntax error on line " << line
                        << ", unexpected character '" << content[current] << "'";
                    throw syntax_error(ss.str());
                }
                break;
        }
    }

    token_insert(tokens, (struct token) {
        .type = TOKEN_EOF,
        .value = "EOF",
        .line = line
    });
}

int seek_identifier(int start, std::string& content, struct m_vector* tokens, int line)
{
    // known keywords to match for
    std::unordered_map<std::string, enum token_type> keywords;
    keywords["int"] = TOKEN_INT;
    keywords["return"] = TOKEN_RETURN;
    keywords["void"] = TOKEN_VOID;
    keywords["char"] = TOKEN_CHAR;

    // seek to the end of the identifier/keyword
    int end = start;
    while (isalnum(content[end]))
        end++;

    const int size = end - start;
    std::string token_text = content.substr(start, size);

    // check whether the text gotten is either a keyword or identifier
    if (keywords.find(token_text) != keywords.end())
    {
        token_insert(tokens, (struct token) {
            .type = keywords[token_text],
            .value = str_copy(token_text.c_str()),
            .line = line,
            .allocated = true,
        });
    }
    else
    {
        token_insert(tokens, (struct token) {
            .type = TOKEN_IDENTIFIER,
            .value = str_copy(token_text.c_str()),
            .line = line,
            .allocated = true
        });
    }

    return end;
}

int seek_constant(int start, std::string& content, struct m_vector* tokens, int line)
{
    // seek to the end of the constant
    int end = start;
    while (isdigit(content[end]))
        end++;
    
    const int size = end - start;
    std::string token_text = content.substr(start, size);

    // push it to the token list
    token_insert(tokens, (struct token) {
        .type = TOKEN_CONSTANT,
        .value = str_copy(token_text.c_str()),
        .line = line,
        .allocated = true,
    });

    return end;
}
