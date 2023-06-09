#ifndef __AST_H__
#define __AST_H__

#include <sys/types.h>
#include <stdbool.h>
#include "vector.h"

typedef enum {
    NODE_PROGRAM,
    NODE_EXPR_BINARY,
    NODE_NUMERIC_LITERAL,
    NODE_IDENTIFIER,
    NODE_EXPR_CALL,
    NODE_EXPR_UNARY,
    NODE_DECL_FUNCTION,
    NODE_DECL_VAR,
    NODE_EXPR_ASSIGNMENT,
    NODE_OBJECT_LITERAL,
    NODE_PROPERTY_LITERAL,
    NODE_EXPR_MEMBER_ACCESS,
    NODE_STRING,
    NODE_CTRL_IF,
    NODE_CTRL_WHILE,
    NODE_CTRL_FOR,
    NODE_BLOCK,
    NODE_CTRL_LOOP,
    NODE_CTRL_BREAK,
    NODE_CTRL_CONTINUE,
    NODE_RETURN,
    NODE_UNKNOWN
} ast_nodetype_t;

typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIVIDE,
    OP_MOD,
    OP_LOGICAL_AND,
    OP_LOGICAL_OR,
    OP_LOGICAL_NOT,
    OP_CMP_EQUALS,
    OP_CMP_EQUALS_STRICT,
    OP_CMP_GREATER_THAN,
    OP_CMP_LESS_THAN,
    OP_CMP_GREATER_THAN_EQUALS,
    OP_CMP_LESS_THAN_EQUALS,
    OP_PRE_INCREMENT,
    OP_PRE_DECREMENT,
    OP_POST_INCREMENT,
    OP_POST_DECREMENT
} ast_operator_t;

typedef enum {
    DT_UNKNOWN,
    DT_INT,
    DT_FLOAT,
    DT_STRING,
    DT_NULL
} data_type_t;

typedef struct ast_stmt {
    ast_nodetype_t type;                            /* Type of this statement. */
    size_t line;

    union {
        /* if (type == NODE_PROGRAM || NODE_DECL_FUNCTION || NODE_BLOCK) */  
        struct {
            struct ast_stmt *body;                  /* Array of statements. */
            size_t size;                            /* Size of the array. */
            char *fn_name;                          /* Function name. */
            vector_t argnames;
        };
        /* endif */     

        /* if (type == NODE_CTRL_IF || type == NODE_CTRL_WHILE || type == NODE_CTRL_LOOP) */
        struct {
            struct ast_stmt *else_body;             /* Single statement. */
            struct ast_stmt *ctrl_body;             /* Single statement. */
            struct ast_stmt *ctrl_cond;
            char *ctrl_loop_identifier;
        };
        /* endif */ 

        /* if (type == NODE_CTRL_FOR) */                
        struct {
            struct ast_stmt *for_body;              /* Single statement. */
            struct ast_stmt *for_init;
            struct ast_stmt *for_cond;              
            struct ast_stmt *for_incdec;
        };
        /* endif */     

        /* if (type == NODE_EXPR_BINARY) || (type == NODE_EXPR_UNARY) */
        struct {
            struct ast_stmt *left;                  /* Pointer to the statement at left. */
            struct ast_stmt *right;                 /* Pointer to the statement at right. */
            ast_operator_t operator;                /* The operator type. */
        };
        /* endif */                
        
        /* if (type == NODE_IDENTIFIER) */
        char *symbol;                               /* The identifier symbol. */
        /* endif */                
        
        /* if (type == NODE_STRING) */
        char *strval;                               /* The string value. */
        /* endif */                
        
        /* if (type == NODE_NUMERIC_LITERAL) */
        struct {
            long double value;                          /* The actual value of the numeric literal. */
            bool is_float;
        };
        /* endif */                
        
        /* if (type == NODE_DECL_VAR) */
        struct {
            char *identifier;
            bool is_const;
            struct ast_stmt *varval;
            bool has_val;
        };                          
        /* endif */                
        
        /* if (type == NODE_EXPR_ASSIGNMENT) */
        struct {
            struct ast_stmt *assignee;
            struct ast_stmt *assignment_value;
        };                          
        /* endif */                
        
        /* if (type == NODE_EXPR_CALL) */
        struct {
            struct ast_stmt *callee;
            vector_t args; /* Vector of struct ast_stmt */
        };                          
        /* endif */                
        
        /* if (type == NODE_PROPERTY_LITERAL) */
        struct {
            char *key;
            struct ast_stmt *propval;       
        };                          
        /* endif */                
        
        /* if (type == NODE_OBJECT_LITERAL) */
        struct {
            vector_t properties; /* Vector of struct ast_stmt */
        };                          
        /* endif */         

        /* if (type == NODE_EXPR_MEMBER_ACCESS) */
        struct {
            struct ast_stmt *object;
            struct ast_stmt *prop;
            bool computed;
        };                          
        /* endif */     

        /* if (type == NODE_RETURN) */      
        struct ast_stmt *return_expr;
        /* endif */     
    };
} ast_stmt;

#endif
