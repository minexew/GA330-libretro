#ifndef MINISYS_THUNKSDEF_H
#define MINISYS_THUNKSDEF_H

// printf
#include <stdio.h>

// malloc
#include <stdlib.h>

typedef struct {
    const char* name;
    int (*addr)(int,  int,  int,  int);
} OsFunction;

#endif
