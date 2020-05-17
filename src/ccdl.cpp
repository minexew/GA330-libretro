#include "ccdl.hpp"
#include "filesystem.hpp"
#include "svc_handlers.hpp"

#include <unicorn/unicorn.h>

#include <stdio.h>

//#define TRACE_INSTRUCTIONS

Filesystem fs;

enum { RAM_START = 0x10000000 };
enum { RAM_SIZE = 0x04000000 };

enum { STACK_BOTTOM = 0x1FF00000 };
enum { STACK_TOP =    0x20000000 };

enum { MINISYS_ADDR = 0x20000000 };
enum { MINISYS_SIZE = 0x40000 };

enum { FRAMEBUF_ADDR = 0x80000000 };
enum { FRAMEBUF_SIZE = (320 * 240 * 2 + 4095) & ~4095 };

// only the file is loaded here -- CCDL parsing is handled by miniSYS
enum { APP_LOAD_ADDRESS = 0x30000000 };
enum { APP_MAX_SIZE = 0x04000000 };

static void hook_code(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
    printf("TRACE %08X\n", address);
}

static void hook_intr(uc_engine *uc, uint32_t intno, void *user_data) {
    uint32_t pc;
    uc_reg_read(uc, UC_ARM_REG_PC, &pc);

    uint32_t num = 0xcccccccc;
    uc_mem_read(uc, pc - 4, &num, 4);
    num &= 0xffff;

    //printf("hook_intr[%d, pc=%08X]\n", num, pc);
    svc_handler_table[num](uc);
}

// callback for tracing invalid memory access (READ/WRITE/EXEC)
static bool hook_mem_invalid(uc_engine* uc, uc_mem_type type,
    uint64_t addr, int size, int64_t value, void* user_data)
{
    switch (type) {
    default:
        printf("not ok - UC_HOOK_MEM_INVALID type: %d at 0x%" PRIx64 "\n", type, addr);
        return false;
    case UC_MEM_READ_UNMAPPED:
        printf("not ok - Read from invalid memory at 0x%" PRIx64 ", data size = %u\n", addr, size);
        return false;
    case UC_MEM_WRITE_UNMAPPED:
        printf("not ok - Write to invalid memory at 0x%" PRIx64 ", data size = %u, data value = 0x%" PRIx64 "\n", addr, size, value);
        return false;
    case UC_MEM_FETCH_PROT:
        printf("not ok - Fetch from non-executable memory at 0x%" PRIx64 "\n", addr);
        return false;
    case UC_MEM_WRITE_PROT:
        printf("not ok - Write to non-writeable memory at 0x%" PRIx64 ", data size = %u, data value = 0x%" PRIx64 "\n", addr, size, value);
        return false;
    case UC_MEM_READ_PROT:
        printf("not ok - Read from non-readable memory at 0x%" PRIx64 ", data size = %u\n", addr, size);
        return false;
    }
}

#define ERR_CHECK() if (err != UC_ERR_OK) {\
        printf("Failed with error returned: %u: %s\n", err, uc_strerror(err));\
        return -1;\
    }

int load_rom(const char* path) {
    FILE* f = fopen(path, "rb");

    uint8_t* buffer = (uint8_t*) malloc(APP_MAX_SIZE);
    size_t app_size = fread(buffer, 1, APP_MAX_SIZE, f);
    fclose(f);

    f = fopen(getenv("MINISYS_BIN"), "rb");
    uint8_t msbuffer[MINISYS_SIZE];
    fread(msbuffer, 1, MINISYS_SIZE, f);
    fclose(f);

    uc_engine* uc;
    uc_err err;
    
    // Initialize emulator in X86-32bit mode
    err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);
    ERR_CHECK();

    // insert MINIsys
    err = uc_mem_map(uc, MINISYS_ADDR, MINISYS_SIZE, UC_PROT_ALL);
    ERR_CHECK();
    err = uc_mem_write(uc, MINISYS_ADDR, msbuffer, sizeof(msbuffer));
    ERR_CHECK();

    // allocate some space for stack
    err = uc_mem_map(uc, STACK_BOTTOM, STACK_TOP - STACK_BOTTOM, UC_PROT_READ | UC_PROT_WRITE);
    ERR_CHECK();

    // map RAM
    err = uc_mem_map(uc, RAM_START, RAM_SIZE, UC_PROT_ALL);
    ERR_CHECK();

    // map framebuf
    err = uc_mem_map(uc, FRAMEBUF_ADDR, FRAMEBUF_SIZE, UC_PROT_WRITE);
    ERR_CHECK();

    // map application
    err = uc_mem_map(uc, APP_LOAD_ADDRESS, APP_MAX_SIZE, UC_PROT_ALL);
    ERR_CHECK();
    err = uc_mem_write(uc, APP_LOAD_ADDRESS, buffer, app_size);
    ERR_CHECK();
    free(buffer);

    // memory-mapped I/O that we don't care about
    err = uc_mem_map(uc, 0x04088000, 0x1000, UC_PROT_READ | UC_PROT_WRITE);

    // intercept invalid memory events
    uc_hook trace1, trace2;
    err = uc_hook_add(uc, &trace1, UC_HOOK_MEM_INVALID, (void*) hook_mem_invalid, NULL, 1, 0);
    ERR_CHECK();
    err = uc_hook_add(uc, &trace2, UC_HOOK_INTR, (void*) hook_intr, NULL, 1, 0);
    ERR_CHECK();
#ifdef TRACE_INSTRUCTIONS
    uc_hook code_hook;
    err = uc_hook_add(uc, &code_hook, UC_HOOK_CODE, (void*) hook_code, NULL, 1, 0);
    ERR_CHECK();
#endif

    // Set stack pointer
    int sp = STACK_TOP;
    uc_reg_write(uc, UC_ARM_REG_SP, &sp);

    printf("Emulation start\n");

    // Jump to minisys
    // void _start(uint8_t const* file_bytes, size_t file_size)
    uint32_t r0 = APP_LOAD_ADDRESS;
    uint32_t r1 = app_size;

    uc_reg_write(uc, UC_ARM_REG_R0, &r0);
    uc_reg_write(uc, UC_ARM_REG_R1, &r1);

    err = uc_emu_start(uc, MINISYS_ADDR, 0, 0, 0);
    if (err) {
        int pc;
        uc_reg_read(uc, UC_ARM_REG_PC, &pc);
        printf(">>> PC = 0x%x\n", pc);

        printf("Failed on uc_emu_start() with error returned %u: %s\n",
            err, uc_strerror(err));
        return -1;
    }

    // now print out some registers
    printf("Emulation done. Below is the CPU context\n");

    uc_reg_read(uc, UC_ARM_REG_R1, &r1);
    printf(">>> R1 = 0x%x\n", r1);

    uc_close(uc);
    return 0;
}
