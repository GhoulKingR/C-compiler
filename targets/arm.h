#include "../nodes.h"

struct var {
    size_t size;
    const char* name;
};

struct var var_at(struct m_vector *vec, int pos) {
    return ((struct var*) vec->_data)[pos];
}

void var_insert(struct m_vector* vec, struct var data) {
    if (vec->_size + 1 >= vec->_capacity)
        vector_resize(vec, vec->_capacity + 100);

    ((struct var*) vec->_data)[vec->_size] = data;
    vec->_size++;
}

struct arm_program_global_var {
    char *memory;
    int _size; 
    int _capacity;

    int stack_size;
    struct m_vector *_variables;
    // std::deque<Var> variables;
};

void arm_program_checkandresize(struct arm_program_global_var *var, size_t size) {
    while (size >= var->_capacity) {
        char *newaddr = (char*) malloc(var->_capacity + 100);
        memcpy(newaddr, var->memory, var->_capacity);
        var->_capacity += 100;
        free(var->memory);
        var->memory = newaddr;
    }
}

void arm_program_append(struct arm_program_global_var *var, const char *text, size_t size) {
    arm_program_checkandresize(var, var->_size + size + 1);
    memcpy(var->memory + var->_size, text, size);
    var->_size += size;
    var->memory[var->_size] = '\0';
}

char* itoa(int num) {
    // get number of digits
    int digit = 0;
    if (num == 0) digit = 1;
    else
        for (int n = num; n > 0; n /= 10) {
            digit++;
        }
    
    char* result = (char*) malloc(digit + 1);

    for (int i = digit - 1; i >= 0; i--) {
        result[i] = (num % 10) + '0';
        num /= 10;
    }

    result[digit] = '\0';
    return result;
}

void arm_compile_variabledecl(struct arm_program_global_var *var, struct variable_decl *vd) {
    // add variable to `variables` for now
    // I say for now because with this, all variables
    // declared are in the global scope
    const int size = datatype_size(vd->type);

    arm_program_append(var, "   mov w0, #", 12);
    arm_program_append(var, vd->value.value, strlen(vd->value.value));
    arm_program_append(var, "\n", 1);

    // find stack position of variable and store in `j`
    int j = 0;
    for (int i = 0; i < var->_variables->_size; i++) {
        struct var v = var_at(var->_variables, i);
        if (v.name == vd->name) {
            char* pos_str = itoa(j);
            switch (v.size) {
                case 2:     // char
                    arm_program_append(var, "    strb w0, [x12, #", 20);
                    arm_program_append(var, pos_str, strlen(pos_str));
                    arm_program_append(var, "]\n", 2);
                    break;
                
                default:
                    arm_program_append(var, "    str w0, [x12, #", 19);
                    arm_program_append(var, pos_str, strlen(pos_str));
                    arm_program_append(var, "]\n", 2);
                    break;
            }
            free(pos_str);
            break;
        }
    }
}

bool arm_compile_return(struct arm_program_global_var *vars, struct Return r)
{
    // return value
    if (r.value.type == EXPR_CONSTANT) {
        /**
         * with 12 as example constant:
         *     mov x0, #12
         */
        arm_program_append(vars, "    mov x0, #", 13);
        arm_program_append(vars, r.value.value, strlen(r.value.value));
        arm_program_append(vars, "\n", 1);
    } else if (r.value.type == EXPR_IDENTIFIER) {
        int j = 0;
        for (int i = 0; i < vars->_variables->_size; i++) {
            struct var v = var_at(vars->_variables, i);
            if (v.name == r.value.value) {
                char* pos_str = itoa(j);
                switch (v.size)
                {
                    case DATATYPE_CHAR:
                        arm_program_append(vars, "    ldrb w0, [x12, #", 20);
                        arm_program_append(vars, pos_str, strlen(pos_str));
                        arm_program_append(vars, "]\n", 2);
                        break;
                    
                    default:
                        arm_program_append(vars, "    ldrb w0, [x12, #", 19);
                        arm_program_append(vars, pos_str, strlen(pos_str));
                        arm_program_append(vars, "]\n", 2);
                        break;
                }
                free(pos_str);
                break;
            }

            j += v.size;
        }
    } else return false;

    // restore all stack allocated variables
    /**
     * with 11 as the allocated stack space:
     *     add sp, sp, #11
     *     ret
     */
    const char* stack_size_str = itoa(vars->stack_size);
    arm_program_append(vars, "    add sp, sp, #", 17);
    arm_program_append(vars, stack_size_str, strlen(stack_size_str));
    arm_program_append(vars, "\nret\n", 5);
    free((void*) stack_size_str);
    return true;
}

bool arm_compile_statements(struct arm_program_global_var *vars, struct statement s)
{
    switch (s.type) {
        case STATEMENT_RETURN:
            return arm_compile_return(vars, s.obj.ret);
        case STATEMENT_VARIABLE_DECL:
            arm_compile_variabledecl(vars, &s.obj.var);
            return true;

        default:
            return false;
    }
}

bool arm_compile_functiondecl(struct arm_program_global_var *vars, struct function *f)
{
    /**
     * .globl _func_name
     * .p2align 2
     * _func_name:
     *     .cfi_startproc
     */
    arm_program_append(vars, ".globl _", 8);
    arm_program_append(vars, f->name, strlen(f->name));
    arm_program_append(vars, "\n.p2align 2\n_", 12);
    arm_program_append(vars, f->name, strlen(f->name));
    arm_program_append(vars, ":\n    .cfi_startproc\n", 21);

    // calculate the amount of stack memory used and
    // pre-allocate that. Make sure the stack is 4-byte aligned (8 nibbles)
    for (int i = 0; i < f->statements->_size; i++) {
        struct statement st = statement_at(f->statements, i);
        if (st.type == STATEMENT_VARIABLE_DECL) {
            struct variable_decl vd = st.obj.var;
            size_t size = datatype_size(vd.type) * 2;

            var_insert(vars->_variables, (struct var) {
                .name = vd.name,
                .size = size,
            });
            vars->stack_size += size;
        }
    }

    /**
     * Allocate the stack using using x12 as internal stack register
     * with 8 as example the asm would look like this:
     * 
     *     sub sp, sp, #8
     *     mov x12, sp
     */
    vars->stack_size = vars->stack_size - (vars->stack_size % 8) + 8;
    const char* stack_size_str = itoa(vars->stack_size);
    arm_program_append(vars, "    sub sp, sp, #", 18);
    arm_program_append(vars, stack_size_str, strlen(stack_size_str));
    arm_program_append(vars, "\n    mov x12, sp\n", 17);
    free((void*) stack_size_str);

    for (int i = 0; i < f->statements->_size; i++) {
        if (!arm_compile_statements(vars, statement_at(f->statements, i)))
            return false;
    }
    
    vars->stack_size = 0;
    vars->_size = 0;
    arm_program_append(vars, "    .cfi_endproc\n", 17);
    return true;
}

void arm_compile_declaration(struct arm_program_global_var *vars, struct declaration d) {
    if (d.type == DECLARATION_FUNCTION) {
        arm_compile_functiondecl(vars, &d.obj.func);
    } else if (d.type == DECLARATION_VARIABLE) {
        arm_compile_variabledecl(vars, &d.obj.var);
    }
}

const char* arm_compile(struct program *p) {
    struct arm_program_global_var var = {
        .memory = (char*) malloc(100),
        ._size = 0,
        ._capacity = 100,
        .stack_size = 0,
        ._variables = vector_init(sizeof(struct var)),
    };
    memset(var.memory, 0, 100);

    for (int i = 0; i < p->declarations->_size; i++) {
        arm_compile_declaration (
            &var,
            declaration_at(p->declarations, i)
        );
        arm_program_append(&var, "\n", 1);
    }

    vector_free(var._variables);
    return var.memory;
}
