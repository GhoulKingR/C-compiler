#include "interface.hpp"

#include <sstream>
#include <string>

class ArmTarget
: public Visitor
{
    std::stringstream ss;

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
            ss << '\n';
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
     * statement.accept()
     *
     * convert statements to assembly
     ***/
    void visit(Statement &s) override
    {
        if (s.type == Statement::RETURN) {
            std::get<Return>(s.obj).accept(*this);
            ss << '\n';
        }
    }

    /***
     * return.accept();
     *
     * convert return statement to assembly
     ***/
    void visit(Return &r) override
    {
        ss  << "    mov w0, #" << r.value.value << '\n'
            << "    ret";
    }

    std::string result()
    {
        return ss.str();
    }
};
