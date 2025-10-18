#include "interface.hpp"

#include <deque>
#include <sstream>
#include <string>

class ArmTarget
: public Visitor
{
    std::stringstream ss;

    struct Var {
        int size;
        std::string name;
        std::string value;

        Var (int size, std::string &name, std::string &value)
        : size(size), name(name), value(value) {}
    };
    // global scope for now
    std::deque<Var> variables;      // push front and pop back to make stack calculations easier

public:

    /***
     * program.accept()
     * 
     * convert program object to arm assembly
     ***/
    void visit(Program &p) override
    {
        for (Declaration& d : p.declarations) {
            d.accept(*this);
            ss << '\n';
        }
    }

    /***
     * declaration.accept()
     *
     * convert declaration to assembly
     ***/
    void visit(Declaration &d) override
    {
        if (d.type == Declaration::FUNCTION) {
            std::get<Function>(d.obj).accept(*this);
        } else if (d.type == Declaration::VARIABLE) {
            std::get<VariableDecl>(d.obj).accept(*this);
        }
    }

    /***
     * function.accept()
     *
     * convert function declaration to assembly
     ***/
    void visit(Function &f) override
    {
        ss  << ".globl _" << f.name << '\n'
            << ".p2align 2\n"
            << "_" << f.name << ":\n"
            << "    .cfi_startproc\n";

        for (Statement st : f.statements) {
            st.accept(*this);
        }
        
        ss  << "    .cfi_endproc\n";
    }

    /***
     * variabledecl.accept()
     *
     * convert variable declaration to assembly
     ***/
    void visit(VariableDecl &vd) override
    {
        // add variable to `variables` for now
        // I say for now because with this, all variables
        // declared are in the global scope
        // So TODO: Design a scope mechanism for managing scopes
        variables.push_front(
            Var(vd.type.size() * 2, vd.name, vd.value.value));
        
        ss  << "    sub sp, sp, #" << vd.type.size() * 2 << '\n'    // each address stores a word (4-bits)
            << "    mov w0, #" << vd.value.value << '\n'
            << "    str w0, [sp]\n";
    }

    /***
     * statement.accept()
     *
     * convert statements to assembly
     ***/
    void visit(Statement &s) override
    {
        switch (s.type) {
            case Statement::RETURN:
                std::get<Return>(s.obj).accept(*this);
                ss << '\n';
                break;
            case Statement::VARIABLE_DECL:
                std::get<VariableDecl>(s.obj).accept(*this);
                break;

            default:
                break;
        }
    }

    /***
     * return.accept();
     *
     * convert return statement to assembly
     ***/
    void visit(Return &r) override
    {
        // return value
        if (r.value.type == Token::CONSTANT)
            ss  << "    mov x0, #" << r.value.value << '\n';
        else if (r.value.type == Token::IDENTIFIER) {
            int i = 0;  // stack position
            for (Var v : variables ) {
                if (v.name == r.value.value)
                    break;

                i += v.size;
            }
            ss  << "    ldr w0, [sp, #" << i << "]\n";
        }

        // restore all stack allocated variables
        int offset = 0;
        for (Var v : variables) {
            ss  << "    add sp, sp, #" << offset + v.size << '\n';
            offset += v.size;
        }

        ss  << "    ret";
    }

    std::string result()
    {
        return ss.str();
    }
};
