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

#include "emu.h"

void* fsys_fopen(const char* path, const char* mode);
void* fsys_fopenW(const char16_t* path, const char* mode);
int fsys_fread(void* arg1, int arg2, int arg3, int* p_fd);
int fsys_fwrite(void* arg1, int arg2, int arg3, int* p_fd);
int fsys_fseek(int* p_fd, int arg2, int arg3);
int fsys_ftell(int* p_fd);
void fsys_fclose(int* p_fd);

void* GetDLHandle(void);
void* get_dl_handle(void);
int __to_locale_ansi(void);

// identical
char* lcd_get_frame(void);
char* LCDGetFB(void);

int LCDGetHeight(void);
int LCDGetWidth(void);
int LCDGetFBFormat(void* unk_0, void* unk_4);

int sys_judge_event(void* cb);
void kbd_get_status(uint32_t* outputs);
int rmt_get_status(void);

void OSTimeDly(int ms);

/**
 * @return time in milliseconds or something
 */
int OSTimeGet(void);

typedef struct {
    const char* name;
    int (*addr)(int,  int,  int,  int);
} OsFunction;

#endif
