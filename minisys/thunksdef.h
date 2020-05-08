#ifndef MINISYS_THUNKSDEF_H
#define MINISYS_THUNKSDEF_H

// printf
#include <stdio.h>

// malloc
#include <stdlib.h>

// dl_get_sym
#include <libccos/dl.h>

void* GetDLHandle(void);
void* get_dl_handle(void);
int __to_locale_ansi(void);

typedef struct {
    const char* name;
    int (*addr)(int,  int,  int,  int);
} OsFunction;

#endif
