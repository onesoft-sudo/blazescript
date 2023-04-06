#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "eval.h"
#include "scope.h"
#include "ast.h"
#include "blaze.h"
#include "runtimevalues.h"
#include "xmalloc.h"

#define NUM(node) (node.is_float ? node.floatval : (node.type == VAL_BOOLEAN ? node.boolval : node.intval))

static size_t line = 0;

static inline void update_line(ast_stmt astnode)
{
    line = astnode.line;
}

static bool is_float(long double val) 
{
    return ceill(val) != floorl(val);
}

void eval_error(bool should_exit, const char *fmt, ...)
{
    va_list args;
    char fmt_processed[strlen(fmt) + 50];
    va_start(args, fmt);

    sprintf(fmt_processed, "\033[1;31mRuntime error\033[0m: %s at line %lu\n", fmt, line);
    vfprintf(stderr, fmt_processed, args);

    va_end(args);

    if (should_exit)
        exit(EXIT_FAILURE);
}

runtime_val_t eval_object_expr(ast_stmt object, scope_t *scope)
{
    map_t properties = MAP_INIT(identifier_t *, 4096);

    for (size_t i = 0; i < object.properties.length; i++)
    {
        ast_stmt prop = VEC_GET(object.properties, i, ast_stmt);
        assert(prop.type == NODE_PROPERTY_LITERAL);

        identifier_t *val;

        if (prop.propval == NULL)
        {
            identifier_t *identifier = scope_resolve_identifier(scope, prop.key);

            if (identifier == NULL)
                eval_error(true, "Undefined identifier '%s' in the current scope", prop.key);

            val = identifier;
        }
        else 
        {
            val = xmalloc(sizeof (identifier_t));

            runtime_val_t eval_result = eval(*prop.propval, scope);
            
            identifier_t i = {
                .is_const = false,
                .name = prop.key
            };

            memcpy(val, &i, sizeof i);

            val->value = xmalloc(sizeof (runtime_val_t));

            memcpy(val->value, &eval_result, sizeof eval_result);
            1;
        }

        map_set(&properties, prop.key, val);
    }
    
    runtime_val_t obj = {
        .type = VAL_OBJECT,
        .properties = properties,
    };

    return obj;
}

runtime_val_t eval_assignment(ast_stmt expr, scope_t *scope)
{
    if (expr.assignee->type != NODE_IDENTIFIER)
        eval_error(true, "Cannot assign a value to a non-modifiable expression");
    
    char *varname = expr.assignee->symbol;
    
    identifier_t *identifier = scope_resolve_identifier(scope, varname);

    update_line(expr);

    if (identifier == NULL)
        eval_error(true, "Undefined identifier '%s'", varname);
    else if (identifier->is_const)    
        eval_error(true, "Cannot re-assign a value to constant '%s'", varname);
    
    runtime_val_t val = eval(*expr.assignment_value, scope);    

    // return (runtime_val_t) { .type = VAL_NULL };
    runtime_val_t result = *scope_assign_identifier(scope, varname, &val);

    return result;
}

runtime_val_t eval_numeric_binop(runtime_val_t left, runtime_val_t right, ast_operator_t operator)
{
    long double result;

    if (operator == OP_PLUS)
        result = NUM(left) + NUM(right);
    else if (operator == OP_MINUS)
        result = NUM(left) - NUM(right);
    else if (operator == OP_TIMES)
        result = NUM(left) * NUM(right);
    else if (operator == OP_DIVIDE)
    {
        if (NUM(right) == 0)
            eval_error(true, "Result of divison by zero is undefined");

        result = NUM(left) / NUM(right); 
    }
    else if (operator == OP_MOD)
    {
        if (left.is_float || right.is_float)
            eval_error(true, "modulus operator requires the operands to be int, float given");

        if (NUM(right) == 0)
            eval_error(true, "Result of divison by zero is undefined");
        
        result = (long long int) NUM(left) % (long long int) NUM(right); 
    }
    else
        blaze_error(true, "invalid binary operator: %d", operator);
    
    runtime_val_t val = {
        .type = VAL_NUMBER,
        .is_float = is_float(result)
    };

    if (val.is_float)
        val.floatval = result;
    else 
        val.intval = (long long int) result;

    return val;
}

runtime_val_t eval_binop(ast_stmt binop, scope_t *scope)
{
    if (binop.type != NODE_EXPR_BINARY)
    {
        blaze_error(true, "invalid binop found");
    }

    update_line(binop);

    runtime_val_t right = eval(*binop.right, scope);
    runtime_val_t left = eval(*binop.left, scope);

    if ((right.type == VAL_NUMBER && left.type == VAL_NUMBER) ||
        ((right.type == VAL_BOOLEAN || left.type == VAL_BOOLEAN) && 
        (right.type == VAL_NUMBER || left.type == VAL_NUMBER)))
    {
        update_line(binop);
        return eval_numeric_binop(left, right, binop.operator);
    }

    printf("%d %d\n", left.type, right.type);

    return (runtime_val_t) {
        .type = VAL_NULL,
    };
}

runtime_val_t eval_program(ast_stmt prog, scope_t *scope)
{
    runtime_val_t last_eval = { .type = VAL_NULL };

    for (size_t i = 0; i < prog.size; i++)
    {
        last_eval = eval(prog.body[i], scope);
    }

    return last_eval;
}

runtime_val_t eval_var_decl(ast_stmt decl, scope_t *scope)
{
#ifndef _NODEBUG
#ifdef _DEBUG
    printf("decl.has_val: %d\n", decl.has_val);
#endif
#endif

    runtime_val_t *value_heap = xmalloc(sizeof (runtime_val_t));

    assert(value_heap != NULL);

    if (decl.has_val)
    {
        runtime_val_t value = eval(*(decl.varval), scope);
        memcpy(value_heap, &value, sizeof value);
    }
    else 
        value_heap->type = VAL_NULL;

    return *(scope_declare_identifier(scope, decl.identifier, value_heap, decl.is_const)->value);
}

runtime_val_t eval_identifier(ast_stmt identifier, scope_t *scope)
{
    identifier_t *identifier_ = scope_resolve_identifier(scope, identifier.symbol);
    return *identifier_->value;
}

runtime_val_t eval(ast_stmt astnode, scope_t *scope)
{
    runtime_val_t val;
    update_line(astnode);

    switch (astnode.type)
    {
        case NODE_NUMERIC_LITERAL:
            val.type = VAL_NUMBER;
            val.is_float = is_float(astnode.value);

            if (val.is_float)
                val.floatval = astnode.value;
            else
                val.intval = astnode.value;
        break;

        case NODE_DECL_VAR:
            return eval_var_decl(astnode, scope);

        case NODE_IDENTIFIER:
        {
            runtime_val_t val = eval_identifier(astnode, scope);
            return val;
        }

        case NODE_OBJECT_LITERAL:
            return eval_object_expr(astnode, scope);

        case NODE_PROGRAM:
            return eval_program(astnode, scope);

        case NODE_EXPR_ASSIGNMENT:
            return eval_assignment(astnode, scope);

        case NODE_EXPR_BINARY:
            return eval_binop(astnode, scope);

        default:
            fprintf(stderr, "Eval error: this AST node is not supported\n");
            exit(EXIT_FAILURE);
    }

    return val;
}
