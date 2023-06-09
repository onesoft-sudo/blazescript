#ifndef __STACK_H__
#define __STACK_H__

#include "runtimevalues.h"

typedef struct {
    size_t size;
    size_t si;
    runtime_val_t *array;
} bstack_t;

bstack_t stack_create(size_t size);
void stack_free(bstack_t *stack);
void stack_push(bstack_t *stack, runtime_val_t value);
void stack_print(bstack_t *stack);
runtime_val_t stack_pop(bstack_t *stack);

#endif