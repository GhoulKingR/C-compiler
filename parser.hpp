#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "exceptions.hpp"
#include "nodes.hpp"
#include "token.hpp"


class Parser
{
    std::vector<Token>& tokens;
    int progress = 0;

public:
    Parser(std::vector<Token>& tokens)
    : tokens(tokens) {}

    Program parse()
    {
        return parseProgram();
    }

private:
    /****
     *  program ::  <declaration>*  :: Program( declarations = [ <declaration>* ] )
     */
    Program parseProgram()
    {
        std::vector<Declaration> declarations = parseDeclarations();
        return Program(declarations);
    }

    /****
     * function_declaration
     * :: <type> IDENTIFIER "(void){" <statement>* "}"
     * :: Function ( type = <type>, name = IDENTIFIER, body = [ <statement>* ] )
     */
    std::vector<Declaration> parseDeclarations()
    {
        std::vector<Declaration> declarations;

        while (!check(Token::END))
        {
            expect(Token::INT);     // only int for now

            Token identifier = accept(Token::IDENTIFIER);

            // (void){
            expect(Token::OPEN_PARENTHESIS);
            expect(Token::VOID);
            expect(Token::CLOSE_PARENTHESIS);
            expect(Token::OPEN_BRACE);

            std::vector<Statement> stmts = parseStatements();

            // }
            expect(Token::CLOSE_BRACE);

            declarations.push_back(
                Declaration(Function(
                    Token::INT, identifier.value, stmts)));
        }

        return declarations;
    }

    /****
     *  <statement>     ::= "return" <exp> ";"
     */
    std::vector<Statement> parseStatements()
    {
        std::vector<Statement> statements;

        while (tokens[progress].type != Token::CLOSE_BRACE)
        {
            switch (tokens[progress].type) {
                case Token::RETURN:
                    progress++;
                    statements.push_back(
                            Statement(Return(accept(Token::CONSTANT))));
                    expect(Token::SEMICOLON);
                    break;

                default:
                    std::stringstream ss;
                    ss  << "Unexpected token '" << tokens[progress].value
                        << "' on line " << tokens[progress].line;
                    throw syntax_error(ss.str());
                    break;
            }
        }

        return statements;
    }

    // return the token at the current progress if it matches 
    // the expectected type or throw an error
    Token accept(Token::Type expected)
    {
        if (expected == tokens[progress].type)
        {
            Token returnVal = tokens[progress];
            progress++;
            return returnVal;
        }
        else
        {
            std::stringstream ss;
            ss  << "Syntax error on line " << tokens[progress].line
                << ", unexpected '" << tokens[progress].value << "'";
            throw syntax_error(ss.str());
        }
    }

    // checkes whether the token at the current position matches 
    // the expected type throwing an error
    void expect(Token::Type expected) {
        if (expected == tokens[progress].type)
            progress++;
        else
        {
            std::stringstream ss;
            ss  << "Syntax error on line " << tokens[progress].line
                << ", unexpected '" << tokens[progress].value << "'";
            throw syntax_error(ss.str());
        }
    }

    // checkes whether the token at the current position matches 
    // the expected type
    bool check(Token::Type expected) {
        return expected == tokens[progress].type;
    }
};
