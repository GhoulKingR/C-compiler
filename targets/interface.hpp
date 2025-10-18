#pragma once

#include "../nodes.hpp"

struct Program;
struct Declaration;
struct Function;
struct Statement;
struct Return;
struct VariableDecl;

class Visitor
{
public:
    virtual void visit(Program& p) = 0;
    virtual void visit(Declaration& d) = 0;
    virtual void visit(Function& f) = 0;
    virtual void visit(Statement& s) = 0;
    virtual void visit(Return& s) = 0;
    virtual void visit(VariableDecl &vd) = 0;
};
