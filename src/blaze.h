#ifndef __BLAZE_H__
#define __BLAZE_H__

#include <stdbool.h>
#include "runtimevalues.h"
#include "utils.h"

#define VERSION "1.0.0-beta1"

typedef struct {
    char *name;
    runtime_val_t (*callback)(vector_t, struct scope *);
} function_t;

void blaze_error(bool shouldexit, char *format, ...);

extern config_t config;

#endif /* __BLAZE_H__ */
 
