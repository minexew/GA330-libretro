#ifndef MINISYS_THUNKSDEF_H
#define MINISYS_THUNKSDEF_H

// printf
#include <stdio.h>

// malloc
#include <stdlib.h>

#include <libccos/ccos_host.h>
#include <libccos/dl.h>
#include <libccos/malloc.h>
#include <libccos/ucos_ii.h>

void* fsys_fopen(const char* path, const char* mode);
void* GetDLHandle(void);
void* get_dl_handle(void);
int __to_locale_ansi(void);

char* LCDGetFB(void);
int LCDGetHeight(void);
int LCDGetWidth(void);

typedef struct {
    const char* name;
    int (*addr)(int,  int,  int,  int);
} OsFunction;

#endif
