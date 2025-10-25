#include "helpers.h"
#include "nodes.h"
#include "token.h"
#include <stdio.h>

struct var {
    size_t size;
    const char* name;
};

struct var var_at(struct m_vector *vec, int pos) {
    return ((struct var*) vec->_data)[pos];
}

static void var_insert(struct m_vector* vec, struct var data) {
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
};

static void arm_program_checkandresize(struct arm_program_global_var *var, size_t size) {
    while (size >= var->_capacity) {
        char *newaddr = (char*) malloc(var->_capacity + 100);
        memcpy(newaddr, var->memory, var->_capacity);
        var->_capacity += 100;
        free(var->memory);
        var->memory = newaddr;
    }
}

static void arm_program_append(struct arm_program_global_var *var, const char *text, size_t size) {
    arm_program_checkandresize(var, var->_size + size + 1);
    memcpy(var->memory + var->_size, text, size);
    var->_size += size;
    var->memory[var->_size] = '\0';
}

static char* itoa(int num) {
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

static int get_stack_pos(struct arm_program_global_var *vars, const char *name) {
    int j = 0;

    for (int i = 0; i < vars->_variables->_size; i++) {
        struct var v = var_at(vars->_variables, i);

        if (strcmp(v.name, name) == 0) return j;
        else j += v.size;
    }

    return -1;
}

/***
 * compiles an expression and stores its results at w0/x0
 **/
static bool arm_compile_expression(struct arm_program_global_var *var, struct Expr* expr) {
    // mov w0, #23
    if (expr->type == EXPR_CONSTANT) {
        arm_program_append(var, "    mov w0, #", 13);
        arm_program_append(var, expr->value, strlen(expr->value));
        arm_program_append(var, "\n", 1);
    } else if (expr->type == EXPR_IDENTIFIER) {
        arm_program_append(var, "    mov w0, [x12, #", 19);

        int pos = get_stack_pos(var, expr->value);
        if (pos == -1) {
            fprintf(stderr, "Variable %s is not defined on line %d\n", expr->value, expr->prefix.tk.line);
            return false;
        }

        char* pos_str = int_to_str(pos);
        arm_program_append(var, pos_str, strlen(pos_str));
        arm_program_append(var, "]\n", 2);
        free(pos_str);
    }

    // prefix operations happen here
    if (expr->prefix.with_operation) {
        if (expr->prefix.tk.type == TOKEN_MINUS) {
            // neg w0, w0
            arm_program_append(var, "    neg w0, w0\n", 15);
        } else if (expr->prefix.tk.type == TOKEN_TILDA) {
            // mvn w0, w0
            arm_program_append(var, "    mvn w0, w0\n", 15);
        } else {
            fprintf(stderr, "Unsupported unary operation '%s'\n", expr->prefix.tk.value);
            return false;
        }
    }

    return true;
}

static bool arm_compile_variabledecl(struct arm_program_global_var *var, struct variable_decl *vd) {
    const int size = datatype_size(vd->type);

    // compile expression here
    if (!arm_compile_expression(var, vd->value))
        return false;

    arm_program_append(var, "    ", 4);
    if (datatype_size(vd->type) == 2) arm_program_append(var, "strb", 4);
    else arm_program_append(var, "str", 3);

    // find stack position of variable && this can be trusted to never return -1
    char* pos = int_to_str(get_stack_pos(var, vd->name));
    arm_program_append(var, " w0, [x12, #", 12);
    arm_program_append(var, pos, strlen(pos));
    arm_program_append(var, "]\n", 2);
    free(pos);

    return true;
}

static bool arm_compile_return(struct arm_program_global_var *vars, struct Return r)
{
    arm_compile_expression(vars, &r.value);

    // restore all stack allocated variables
    /**
     * with 11 as the allocated stack space:
     *     add sp, sp, #11
     *     ret
     */
    const char* stack_size_str = itoa(vars->stack_size);
    arm_program_append(vars, "    add sp, sp, #", 17);
    arm_program_append(vars, stack_size_str, strlen(stack_size_str));
    arm_program_append(vars, "\n    ret\n", 9);
    free((void*) stack_size_str);
    return true;
}

static bool arm_compile_statements(struct arm_program_global_var *vars, struct statement s)
{
    switch (s.type) {
        case STATEMENT_RETURN:
            return arm_compile_return(vars, s.obj.ret);
        case STATEMENT_VARIABLE_DECL:
            return arm_compile_variabledecl(vars, &s.obj.var);

        default:
            fprintf(stderr, "[INTERNAL] Unsupported statement type\n");
            return false;
    }
}

static bool arm_compile_functiondecl(struct arm_program_global_var *vars, struct function *f)
{
    /**
     * .globl _func_name
     * .p2align 2
     * _func_name:
     *     .cfi_startproc
     */
    arm_program_append(vars, ".globl _", 8);
    arm_program_append(vars, f->name, strlen(f->name));
    arm_program_append(vars, "\n.p2align 2\n_", 13);
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
    arm_program_append(vars, "    sub sp, sp, #", 17);
    arm_program_append(vars, stack_size_str, strlen(stack_size_str));
    arm_program_append(vars, "\n    mov x12, sp\n", 17);
    free((void*) stack_size_str);

    for (int i = 0; i < f->statements->_size; i++) {
        if (!arm_compile_statements(vars, statement_at(f->statements, i)))
            return false;
    }
    
    arm_program_append(vars, "    .cfi_endproc\n", 17);
    return true;
}

static bool arm_compile_declaration(struct arm_program_global_var *vars, struct declaration d) {
    if (d.type == DECLARATION_FUNCTION) {
        return arm_compile_functiondecl(vars, &d.obj.func);
    } else if (d.type == DECLARATION_VARIABLE) {
        return arm_compile_variabledecl(vars, &d.obj.var);
    }
    return false;
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
        if (!arm_compile_declaration(
            &var, declaration_at(p->declarations, i))) {
            goto error_cleanup;
        }

        arm_program_append(&var, "\n", 1);
        var.stack_size = 0;
        var._size = 0;
    }

    vector_free(var._variables);
    return var.memory;
    
error_cleanup:
    free(var.memory);
    vector_free(var._variables);
    return NULL;
}
