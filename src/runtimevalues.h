#ifndef __RUNTIMEVALUES_H__
#define __RUNTIMEVALUES_H__

#include <stdbool.h>
#include "map.h"
#include "vector.h"
#include "ast.h"

#define BLAZE_NULL ((runtime_val_t) { .type = VAL_NULL })
#define BLAZE_INT(n) ((runtime_val_t) { .type = VAL_NUMBER, .is_float = false, .intval = n })
#define BLAZE_TRUE ((runtime_val_t) { .type = VAL_BOOLEAN, .boolval = true })
#define BLAZE_FALSE ((runtime_val_t) { .type = VAL_BOOLEAN, .boolval = false })

typedef enum 
{
    VAL_NUMBER,
    VAL_NULL,
    VAL_BOOLEAN,
    VAL_OBJECT,
    VAL_NATIVE_FN,
    VAL_STRING,
    VAL_USER_FN,
    VAL_ANY
} runtime_valtype_t;

typedef struct runtime_val_t
{
    runtime_valtype_t type;
    bool literal;

    union {
        /* if (type == VAL_NUMBER) */
        struct {
            long long int intval;
            long double floatval;
            bool is_float;
        };
        /* endif */
        
        /* if (type == VAL_BOOLEAN) */
        bool boolval;
        /* endif */
        
        /* if (type == VAL_OBJECT) */
        map_t properties;
        /* endif */
        
        /* if (type == VAL_STRING) */
        char *strval;
        /* endif */
        
        /* if (type == VAL_NATIVE_FN) */
        struct runtime_val_t (*fn)(vector_t args, struct scope *scope);
        /* endif */
        
        /* if (type == VAL_USER_FN) */
        struct {
            char *fn_name;
            vector_t argnames; 
            ast_stmt *body;
            size_t size;
            struct scope *scope;
        };
        /* endif */
    };
} runtime_val_t;

#endif
