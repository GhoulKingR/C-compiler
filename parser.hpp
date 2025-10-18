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
            DataType type = parseDataType();

            Token identifier = accept(Token::IDENTIFIER);

            if (check(Token::OPEN_PARENTHESIS)) {
                // function declaration
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
                        type, identifier.value, stmts)));
            } else if (check(Token::EQUAL)) {
                // variable declaration
                expect(Token::EQUAL);
                Token constant = accept(Token::CONSTANT);
                expect(Token::SEMICOLON);

                declarations.push_back(
                    Declaration(VariableDecl(
                            type, constant, identifier.value)));
            }
        }

        return declarations;
    }

    /****
     * parse the current token for the data type
     * type :: "void" | "int" | "char"
     */
    DataType parseDataType()
    {
        if (check(Token::INT) || check(Token::VOID) || check(Token::CHAR)) {
            DataType result(tokens[progress].type);
            progress++;
            return result;
        }

        std::stringstream ss;
        ss  << "Unexpected token '" << tokens[progress].value
            << "' on line " << tokens[progress].line;
        throw syntax_error(ss.str());
    }

    /****
     * statement :: <return> | <variable_decl>
     */
    std::vector<Statement> parseStatements()
    {
        std::vector<Statement> statements;

        while (tokens[progress].type != Token::CLOSE_BRACE)
        {
            /* return :: "return " (CONSTANT | IDENTIFIER) ";" */
            if (check(Token::RETURN)) {
                progress++;

                if (check(Token::CONSTANT))
                    statements.push_back(
                        Statement(Return(accept(Token::CONSTANT))));
                else 
                    statements.push_back(
                        Statement(Return(accept(Token::IDENTIFIER))));

                expect(Token::SEMICOLON);

            /* variable_decl :: <type> IDENTIFIER "=" CONSTANT ";" */
            } else {
                DataType type = parseDataType();        // throws error if it couldn't recognize a type

                Token identifier = accept(Token::IDENTIFIER);
                expect(Token::EQUAL);
                Token constant = accept(Token::CONSTANT);
                expect(Token::SEMICOLON);

                statements.push_back(
                    Statement(VariableDecl(
                        type, constant, identifier.value)));
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
