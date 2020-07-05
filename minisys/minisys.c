#include "emu.h"
#include "thunks.h"

#include <libccos/ccos_host.h>
#include <libccos/dl.h>

#include <stdio.h>
#include <string.h>

extern unsigned long _sidata;
extern unsigned long _sdata;
extern unsigned long _edata;

static DL_t my_dl;

void _start(uint8_t const* file_bytes, size_t file_size) {
    // First of all, init .data section
    uint32_t* src = &_sidata;
    uint32_t* dst = &_sdata;

    while (dst < &_edata) {
        *(dst++) = *(src++);
    }

    // Now we can use libc normally
    printf("miniSYS _start(%p, %zd)\n", file_bytes, file_size);

    int rc = ccdl_from_memory(&my_dl, file_bytes, file_size);
    printf("ccdl_from_memory: %d\n", rc);

    if (rc < 0) {
        ccos_panic("Failed to load application");
    }

    void* appmain_address = dl_get_proc(&my_dl, "AppMain");

    printf("minSYS: AppMain=%p\n", appmain_address);
    ((int (*)(int)) appmain_address)(&my_dl);
}

ccos_proc_t ccos_get_builtin_proc(char const* name) {
    for (int i = 0; os_function_table[i].name; i++) {
        if (!strcmp(os_function_table[i].name, name)) {
            //printf("dl_get_proc(%s) -> %p\n", name, os_function_table[i].addr);
            return (ccos_proc_t) os_function_table[i].addr;
        }
    }

    printf("dl_get_proc(%s) FAIL\n", name);
    ccos_panic("Failed to link proc");
}

// TODO: better implemented in emu?
void ccos_diag_at(char const* message, char const* file, int line) {
    printf("[ccos_diag] %s at %s:%d\n", message, file, line);
}

// TODO: better implemented in emu?
__attribute__((noreturn))
void ccos_panic_at(char const* message, char const* file, int line) {
    printf("[ccos_panic] %s at %s:%d\n", message, file, line);
    GemeiEmu_panic();
}

// TODO: correct signature
void* GetDLHandle(void) {
    ccos_trace_stub(GetDLHandle);
    return &my_dl;
}

// TODO: correct signature
void* get_dl_handle(void) {
    ccos_trace_stub(get_dl_handle);
    return &my_dl;
}

char* lcd_get_frame(void) {
    ccos_trace_stub(lcd_get_frame);
    return 0x80000000;
}

char* LCDGetFB(void) {
    ccos_trace_stub(LCDGetFB);
    return 0x80000000;
}

int LCDGetFBFormat(void* unk_0, void* unk_4) {
    ccos_trace_stub(LCDGetFBFormat);
    return 0;
}

int LCDGetHeight(void) {
    enum {LCD_HEIGHT = 240};
    return LCD_HEIGHT;
}

int LCDGetWidth(void) {
    enum {LCD_WIDTH = 320};
    return LCD_WIDTH;
}

void* fsys_fopen(const char* path, const char* mode) {
    ccos_trace_stub(fsys_fopen);
    int fd = GemeiEmu_fopen(path, mode);
    //return malloc(4);
    return NULL;
}

void* fsys_fopenW(const char16_t* path, const char* mode) {
    ccos_trace_stub(fsys_fopenW);
    int fd = GemeiEmu_fopenW(path, mode);
    //return malloc(4);
    return NULL;
}

int fsys_fread(int arg1, int arg2, int arg3, int arg4) {
    ccos_trace_stub(fsys_fread);
    return 0;
}

int OSTimeGet(void) {
    ccos_trace_stub(OSTimeGet);
    return 0;
}

// TODO: correct signature
int __to_locale_ansi(void) {
    ccos_trace_stub(__to_locale_ansi);
    return "MINISYS.PLACEHOLDER";
}
