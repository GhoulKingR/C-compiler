#include "helpers.h"
#include "parser/nodes.h"
#include "lexer/token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    struct m_vector *_variables;    //: struct var
    int l_pos;
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

int max(int a, int b) {
    return a > b ? a : b;
}

static void arm_program_append(struct arm_program_global_var *var, const char *text, size_t size) {
    arm_program_checkandresize(var, var->_size + size + 1);
    memcpy(var->memory + var->_size, text, size);
    var->_size += size;
    var->memory[var->_size] = '\0';
}



static int get_stack_pos(struct arm_program_global_var *vars, const char *name) {
    int j = 0;

    for (int i = vars->_variables->_size - 1; i >= 0; i--) {
        struct var v = var_at(vars->_variables, i);

        if (strcmp(v.name, name) == 0) return j;
        else j += max(v.size, 8);
    }

    return -1;
}

static struct var *get_var(struct arm_program_global_var *vars, const char *name) {
    for (int i = 0; i < vars->_variables->_size; i++) {
        struct var *v = &((struct var*) vars->_variables->_data)[i];

        if (strcmp(v->name, name) == 0) return v;
    }

    return NULL;
}

/**
 * Store whatever is in the w0 register to the stack position of name
 */
static bool store_variable(struct arm_program_global_var *var, const char *name) {
    struct var *v = get_var(var, name);
    char *stack_pos = int_to_str(get_stack_pos(var, name));

    if (v == NULL) {
        fprintf(stderr, "Variable %s does not exist\n", name);
        goto error;
    }

    switch (v->size) {
        case 8:
            arm_program_append(var, "    str w0, [x12, #", 19);
            arm_program_append(var, stack_pos, strlen(stack_pos));
            arm_program_append(var, "]\n", 2);
            break;

        default:
            fprintf(stderr, "Variable %s does not support assignment operations\n", name);
            goto error;
    }

    free(stack_pos);
    return true;

error:
    free(stack_pos);
    return false;
}

/**
 * Load the data from stack position to the w0 register
 */
static bool load_variable(struct arm_program_global_var *var, const char *name) {
    struct var *v = get_var(var, name);
    char *stack_pos = int_to_str(get_stack_pos(var, name));

    if (v == NULL) {
        fprintf(stderr, "Variable %s does not exist\n", name);
        goto error;
    }

    switch (v->size) {
        case 8:
            arm_program_append(var, "    ldr w0, [x12, #", 19);
            arm_program_append(var, stack_pos, strlen(stack_pos));
            arm_program_append(var, "]\n", 2);
            break;

        default:
            fprintf(stderr, "Variable %s does not support assignment operations\n", name);
            goto error;
    }

    free(stack_pos);
    return true;

error:
    free(stack_pos);
    return false;
}

/***
 * compiles an expression and stores its results at w0/x0
 **/
static bool arm_compile_expression(struct arm_program_global_var *var, struct Expr* expr) {
    int stack_pos = 0;
    char* stack_pos_str = NULL;
    int stack_pos_str_count = 0;

    if (expr->type == EXPR_UNARY_OPERATION) {
        arm_compile_expression(var, expr->obj.unary.value);

        // prefix operations happen here
        switch (expr->obj.unary.prefix.type) {
            case TOKEN_PLUS:
                break;
            case TOKEN_MINUS:
                arm_program_append(var, "    neg x0, x0\n", 15);
                break;
            case TOKEN_TILDA:
                arm_program_append(var, "    mvn x0, x0\n", 15);
                break;
            case TOKEN_BANG:
                arm_program_append(var, "    cmp x0, #0\n", 15);
                arm_program_append(var, "    cset x0, eq\n", 16);
                break;
            case TOKEN_PLUS_PLUS:
                if (expr->obj.unary.value->type != EXPR_PRIMARY &&
                    expr->obj.unary.value->obj.primary.value.type != TOKEN_IDENTIFIER)
                {
                    fprintf(stderr, "Error on line %d, you can only increment identifiers",
                        expr->obj.unary.prefix.line);
                    return false;
                }

                arm_program_append(var, "    add x0, x0, #1\n", 19);
                store_variable(var, expr->obj.unary.value->obj.primary.value.value);
                break;
            case TOKEN_MINUS_MINUS:
                if (expr->obj.unary.value->type != EXPR_PRIMARY &&
                    expr->obj.unary.value->obj.primary.value.type != TOKEN_IDENTIFIER)
                {
                    fprintf(stderr, "Error on line %d, you can only decrement identifiers",
                        expr->obj.unary.prefix.line);
                    return false;
                }

                arm_program_append(var, "    sub x0, x0, #1\n", 19);
                store_variable(var, expr->obj.unary.value->obj.primary.value.value);
                break;

            default:
                fprintf(stderr, "Unsupported unary operation '%s'\n", expr->obj.unary.prefix.value);
                return false;
        }
        return true;
    } else if (expr->type == EXPR_PRIMARY) {
        // mov w0, #23
        if (expr->obj.primary.value.type == TOKEN_CONSTANT) {
            arm_program_append(var, "    mov x0, #", 13);
            arm_program_append(var, expr->obj.primary.value.value, strlen(expr->obj.primary.value.value));
            arm_program_append(var, "\n", 1);
        } else if (expr->obj.primary.value.type == TOKEN_IDENTIFIER) {
            return load_variable(var, expr->obj.primary.value.value);
        }

        return true;
    } else if (expr->type == EXPR_BINARY_OPERATION) {
        // compile the left side of the expression.
        arm_compile_expression(var, expr->obj.binary.left);

        // This stores its results in w0/x0 register so 
        // move it to stack before running the next side of the operation
        arm_program_append(var, "    sub sp, sp, #16\n", 20);
        arm_program_append(var, "    str x0, [sp]\n", 17);

        // compile the right side of the expression.
        arm_compile_expression(var, expr->obj.binary.right);

        arm_program_append(var, "    ldr x1, [sp]\n", 17);
        arm_program_append(var, "    add sp, sp, #16\n", 20);   // restore stack pointer

        struct {
            char* str;
            int   count;
        } l_pos[2] = { 0 };

        switch (expr->obj.binary.operation.type) {
            case TOKEN_PLUS:
                arm_program_append(var, "    add x0, x1, x0\n", 19);
                break;
            case TOKEN_MINUS:
                arm_program_append(var, "    sub x0, x1, x0\n", 19);
                break;
            case TOKEN_STAR:
                arm_program_append(var, "    mul x0, x1, x0\n", 19);
                break;
            case TOKEN_SLASH:
                arm_program_append(var, "    sdiv x0, x1, x0\n", 20);
                break;
            case TOKEN_PERCENT:
                arm_program_append(var, "    sdiv x3, x1, x0\n", 20);
                arm_program_append(var, "    mul x2, x3, x0\n", 19);
                arm_program_append(var, "    sub x0, x1, x2\n", 19);
                break;
            case TOKEN_AND:
                arm_program_append(var, "    and x0, x1, x0\n", 19);
                break;
            case TOKEN_AND_AND:
                l_pos[0].str = int_to_str(var->l_pos);
                l_pos[0].count = digit_count(var->l_pos);

                // cmp x1, #0
                // beq L1
                arm_program_append(var, "    cmp x1, #0\n", 15);
                arm_program_append(var, "    beq L", 9);
                arm_program_append(var, l_pos[0].str, l_pos[0].count);
                arm_program_append(var, "\n", 1);

                // cmp x0, #0
                // beq L2
                arm_program_append(var, "    cmp x0, #0\n", 15);
                arm_program_append(var, "    beq L", 9);
                arm_program_append(var, l_pos[0].str, l_pos[0].count);
                arm_program_append(var, "\n", 1);

                arm_program_append(var, "    mov x0, #1\n", 15);

                l_pos[1].str = int_to_str(var->l_pos + 1);
                l_pos[1].count = digit_count(var->l_pos + 1);
                arm_program_append(var, "    b L", 7);
                arm_program_append(var, l_pos[1].str, l_pos[1].count);
                arm_program_append(var, "\n", 1);

                arm_program_append(var, "L", 1);
                arm_program_append(var, l_pos[0].str, l_pos[0].count);
                arm_program_append(var, ":\n", 2);
                arm_program_append(var, "    mov x0, #0\n", 15);

                arm_program_append(var, "L", 1);
                arm_program_append(var, l_pos[1].str, l_pos[1].count);
                arm_program_append(var, ":\n", 2);

                var->l_pos += 2;
                free(l_pos[0].str);
                free(l_pos[1].str);
                l_pos[0].str = NULL;
                l_pos[0].count = 0;
                l_pos[1].str = NULL;
                l_pos[1].count = 0;
                break;
            case TOKEN_PIPE_PIPE:
                l_pos[0].str = int_to_str(var->l_pos);
                l_pos[0].count = digit_count(var->l_pos);

                // cmp x1, #0
                // beq L1
                arm_program_append(var, "    cmp x1, #0\n", 15);
                arm_program_append(var, "    bne L", 9);
                arm_program_append(var, l_pos[0].str, l_pos[0].count);
                arm_program_append(var, "\n", 1);

                // cmp x0, #0
                // beq L2
                arm_program_append(var, "    cmp x0, #0\n", 15);
                arm_program_append(var, "    bne L", 9);
                arm_program_append(var, l_pos[0].str, l_pos[0].count);
                arm_program_append(var, "\n", 1);

                arm_program_append(var, "    mov x0, #0\n", 15);

                l_pos[1].str = int_to_str(var->l_pos + 1);
                l_pos[1].count = digit_count(var->l_pos + 1);
                arm_program_append(var, "    b L", 7);
                arm_program_append(var, l_pos[1].str, l_pos[1].count);
                arm_program_append(var, "\n", 1);

                arm_program_append(var, "L", 1);
                arm_program_append(var, l_pos[0].str, l_pos[0].count);
                arm_program_append(var, ":\n", 2);
                arm_program_append(var, "    mov x0, #1\n", 15);

                arm_program_append(var, "L", 1);
                arm_program_append(var, l_pos[1].str, l_pos[1].count);
                arm_program_append(var, ":\n", 2);

                var->l_pos += 2;
                free(l_pos[0].str);
                free(l_pos[1].str);
                l_pos[0].str = NULL;
                l_pos[0].count = 0;
                l_pos[1].str = NULL;
                l_pos[1].count = 0;
                break;
            case TOKEN_CARET:
                arm_program_append(var, "    eor x0, x1, x0\n", 19);
                break;
            case TOKEN_PIPE:
                arm_program_append(var, "    orr x0, x1, x0\n", 19);
                break;
            case TOKEN_BANG_EQUAL:
                arm_program_append(var, "    cmp x1, x0\n", 15);
                arm_program_append(var, "    cset x0, ne\n", 16);
                break;
            case TOKEN_EQUAL_EQUAL:
                arm_program_append(var, "    cmp x1, x0\n", 15);
                arm_program_append(var, "    cset x0, eq\n", 16);
                break;
            case TOKEN_GREATER_THAN:
                arm_program_append(var, "    cmp x1, x0\n", 15);
                arm_program_append(var, "    cset x0, gt\n", 16);
                break;
            case TOKEN_GREATER_THAN_EQUAL:
                arm_program_append(var, "    cmp x1, x0\n", 15);
                arm_program_append(var, "    cset x0, ge\n", 16);
                break;
            case TOKEN_LESS_THAN:
                arm_program_append(var, "    cmp x1, x0\n", 15);
                arm_program_append(var, "    cset x0, lt\n", 16);
                break;
            case TOKEN_LESS_THAN_EQUAL:
                arm_program_append(var, "    cmp x1, x0\n", 15);
                arm_program_append(var, "    cset x0, le\n", 16);
                break;

            default:
                fprintf(stderr, "Unexpected operator token '%s' on like %d\n",
                        expr->obj.binary.operation.value,
                        expr->obj.binary.operation.line);
                return false;
        }

        return true;
    } else if (expr->type == EXPR_ASSIGNMENT) {
        int stack_pos = get_stack_pos(var, expr->obj.assignment.name.value);
        if (stack_pos < 0) {
            fprintf(stderr, "Variable %s not defined on line %d\n",
                expr->obj.assignment.name.value,
                expr->obj.assignment.name.line);
            return false;
        }

        if (!arm_compile_expression(var, expr->obj.assignment.right)) {
            return false;
        }

        char *pos_str = int_to_str(stack_pos);
        arm_program_append(var, "    str w0, [x12, #", 19);
        arm_program_append(var, pos_str, digit_count(stack_pos));
        arm_program_append(var, "]\n", 2);
        
        return false;
    } else {
        fprintf(stderr, "Unsupported expression type\n");
        return false;
    }
}

static bool arm_compile_variabledecl(struct arm_program_global_var *var, struct variable_decl *vd) {
    const int size = datatype_size(vd->type);

    // compile expression here
    if (!arm_compile_expression(var, vd->value))
        return false;
    
    // make sure the variable hasn't already been declared before
    if (get_var(var, vd->name) != NULL) {
        fprintf(stderr, "Redeclaration of variable %s on line %d\n", vd->name, vd->line);
        return false;
    }

    // allocate stack memory here
    if (size <= 4) {
        arm_program_append(var, "    sub x12, x12, #8\n", 21);
        var_insert(var->_variables, (struct var) {
            .size = 8,
            .name = vd->name,
        });
    } else {
        char *s = int_to_str(size * 2);
        arm_program_append(var, "    sub x12, x12, #", 19); 
        arm_program_append(var, s, strlen(s));
        arm_program_append(var, "\n", 1);
        free(s);
        var_insert(var->_variables, (struct var) {
            .size = size * 2,
            .name = vd->name,
        });
    }

    return store_variable(var, vd->name);
}

static bool arm_compile_return(struct arm_program_global_var *vars, struct Return r)
{
    arm_compile_expression(vars, r.value);

    /**
     * restore all stack allocated variables
     * with 11 as the allocated stack space:
     *     add sp, sp, #11
     *     ret
     */
    const char* stack_size_str = int_to_str(vars->stack_size);
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
        case STATEMENT_EXPRESSION:
            return arm_compile_expression(vars, s.obj.expr);
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
    // pre-allocate that. Make sure the stack is 16-byte aligned in the end
    for (int i = 0; i < f->statements->_size; i++) {
        struct statement st = statement_at(f->statements, i);
        if (st.type == STATEMENT_VARIABLE_DECL) {
            struct variable_decl vd = st.obj.var;
            size_t size = datatype_size(vd.type) * 2;
            vars->stack_size += max(size, 8);
        }
    }

    /**
     * Allocate the stack using using x12 as internal stack register
     * with 8 as example the asm would look like this:
     * 
     *     mov x12, sp
     *     sub sp, sp, #8
     */
    if (vars->stack_size % 16 != 0)
        vars->stack_size = vars->stack_size - (vars->stack_size % 16) + 16;
    const char* stack_size_str = int_to_str(vars->stack_size);
    arm_program_append(vars, "    mov x12, sp\n", 16);
    arm_program_append(vars, "    sub sp, sp, #", 17);
    arm_program_append(vars, stack_size_str, strlen(stack_size_str));
    arm_program_append(vars, "\n", 1);
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
        .l_pos = 0,
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
