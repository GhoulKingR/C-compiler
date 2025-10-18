#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser.hpp"
#include "targets/arm.hpp"

#define DEBUG

void lexer(std::vector<Token>& tokens, std::string& content);

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
    const char* c_file = argc < 2 ? "../main.c" : argv[argc - 1];
#endif

    // read c file
    std::ifstream f(c_file);
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string content = buffer.str();
    f.close();

    // pass it to the lexer
    std::vector<Token> tokens;
    lexer(tokens, content);

    // initialize the parser
    Parser parser(tokens);
    Program program = parser.parse();

    // convert program to assembly
    ArmTarget converter;
    program.accept(converter);
    
    // write assembly to file
    std::ofstream output("../output.s");
    output << converter.result();
    output.close();

    return 0;
}

int seek_identifier(int start, std::string& content, std::vector<Token>& tokens, int line);
int seek_constant(int start, std::string& content, std::vector<Token>& tokens, int line);

void lexer(std::vector<Token>& tokens, std::string& content)
{
    int current = 0;
    int line = 1;

    while (current < content.size())
    {
        switch (content[current])
        {
            // symbols
            case '(':
                tokens.push_back(Token(Token::OPEN_PARENTHESIS, "(", line));
                current++;
                break;
            case ')':
                tokens.push_back(Token(Token::CLOSE_PARENTHESIS, ")", line));
                current++;
                break;
            case '{':
                tokens.push_back(Token(Token::OPEN_BRACE, "{", line));
                current++;
                break;
            case '}':
                tokens.push_back(Token(Token::CLOSE_BRACE, "{", line));
                current++;
                break;
            case ';':
                tokens.push_back(Token(Token::SEMICOLON, "{", line));
                current++;
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
                break;
        }
    }

    tokens.push_back(Token(Token::END, "EOF", line));
}

int seek_identifier(int start, std::string& content, std::vector<Token>& tokens, int line)
{
    // known keywords to match for
    std::unordered_map<std::string, Token::Type> keywords;
    keywords["int"] = Token::INT;
    keywords["return"] = Token::RETURN;
    keywords["void"] = Token::VOID;

    // seek to the end of the identifier/keyword
    int end = start;
    while (isalnum(content[end]))
        end++;

    const int size = end - start;
    std::string token_text = content.substr(start, size);

    // check whether the text gotten is either a keyword or identifier
    if (keywords.find(token_text) != keywords.end())
    {
        tokens.push_back(
            Token(keywords[token_text], token_text, line));
    }
    else
    {
        tokens.push_back(
            Token(Token::IDENTIFIER, token_text, line));
    }

    return end;
}

int seek_constant(int start, std::string& content, std::vector<Token>& tokens, int line)
{
    // seek to the end of the constant
    int end = start;
    while (isdigit(content[end]))
        end++;
    
    const int size = end - start;
    std::string token_text = content.substr(start, size);

    // push it to the token list
    tokens.push_back(
        Token(Token::CONSTANT, token_text, line));

    return end;
}
