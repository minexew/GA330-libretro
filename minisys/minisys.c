#include "emu.h"
#include "thunks.h"

#include <libccos/ccos_host.h>
#include <libccos/dl.h>

#include <stdio.h>
#include <string.h>

// Ensure that the first instruction of the binary is a jump to _start
__asm(
    "b _start \r\n"
    "ldr pc, =0xAABBCCDD \r\n"
);

static DL_t my_dl;

void _start(uint8_t const* file_bytes, size_t file_size) {
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
    return &my_dl;
}

// TODO: correct signature
void* get_dl_handle(void) {
    return &my_dl;
}

char* LCDGetFB(void) {
    return 0x80000000;
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
    return 0xAAAAAAAA + GemeiEmu_fopen(path, mode);
}

// TODO: correct signature
int __to_locale_ansi(void) {
    return "MINISYS.PLACEHOLDER";
}
