#include <deque>
#include <sstream>
#include <stack>
#include <string>

class ArmTarget
{
    std::stringstream ss;
    int stack_size = 0;

    struct Var {
        int size;
        std::string name;

        Var (int size, std::string &name)
        : size(size), name(name) {}
    };

    // global scope for now
    std::deque<Var> variables;

public:
    std::string compile(Program &p) {
        for (Declaration& d : p.declarations) {
            compile(d);
            ss << '\n';
        }

        return ss.str();
    }

private:
    void compile(Declaration &d)
    {
        if (d.type == Declaration::FUNCTION) {
            compile(std::get<Function>(d.obj));
        } else if (d.type == Declaration::VARIABLE) {
            compile(std::get<VariableDecl>(d.obj));
        }
    }

    void compile(Function &f)
    {
        ss  << ".globl _" << f.name << '\n'
            << ".p2align 2\n"
            << "_" << f.name << ":\n"
            << "    .cfi_startproc\n";

        // calculate the amount of stack memory used and
        // pre-allocate that. Make sure the stack is 4-byte aligned (8 nibbles)

        stack_size = 0;
        
        for (Statement st: f.statements) {
            if (st.type == Statement::VARIABLE_DECL) {
                VariableDecl& vd = std::get<VariableDecl>(st.obj);
                size_t size = vd.type.size() * 2;

                variables.push_front(Var(size, vd.name));
                stack_size += size;
            }
        }

        stack_size = stack_size - (stack_size % 8) + 8;
        ss  << "    sub sp, sp, #" << stack_size << '\n'
            << "    mov x12, sp\n";

        for (Statement st : f.statements) {
            compile(st);
        }
        
        stack_size = 0;
        variables.clear();
        ss  << "    .cfi_endproc\n";
    }

    void compile(VariableDecl &vd)
    {
        // add variable to `variables` for now
        // I say for now because with this, all variables
        // declared are in the global scope
        const int size = vd.type.size();

        ss << "    mov w0, #" << vd.value.getData<std::string>() << '\n';

        // find stack position of variable and store in `i`
        int i = 0;
        for (Var v : variables ) {
            if (v.name == vd.name) {
                switch (v.size)
                {
                case 2:     // char
                    ss  << "    strb w0, [x12, #" << i << "]\n";
                    break;
                
                default:
                    ss  << "    str w0, [x12, #" << i << "]\n";
                    break;
                }
                break;
            }

            i += v.size;
        }
    }

    void compile(Statement &s)
    {
        switch (s.type) {
            case Statement::RETURN:
                compile(std::get<Return>(s.obj));
                ss << '\n';
                break;
            case Statement::VARIABLE_DECL:
                compile(std::get<VariableDecl>(s.obj));
                break;

            default:
                break;
        }
    }

    void compile(Return &r)
    {
        // return value
        if (r.value.type == Expr::CONSTANT)
            ss  << "    mov x0, #" << r.value.getData<std::string>() << '\n';
        else if (r.value.type == Expr::IDENTIFIER) {
            int i = 0;
            for (Var v : variables ) {
                if (v.name == r.value.getData<std::string>()) {
                    switch (v.size)
                    {
                        case DataType::CHAR:
                            ss  << "    ldrb w0, [x12, #" << i << "]\n";
                            break;
                        
                        default:
                            ss  << "    ldr w0, [x12, #" << i << "]\n";
                            break;
                    }
                    break;
                }

                i += v.size;
            }
        }

        // restore all stack allocated variables
        ss  << "    add sp, sp, #" << stack_size << '\n'
            << "    ret";
    }
};
