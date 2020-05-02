#include "ccdl.h"

#include <unicorn/unicorn.h>

#include <stdio.h>

enum { OFFSET = 0x52B0 };
enum { LOAD_ADDRESS = 0x10100000 };
enum { LOAD_SIZE = 0xDF1B4 };
enum { ALLOC_SIZE = 0xE9AE0 };
enum { ENTRY_ADDRESS = 0x10100150 };
enum { APPMAIN_ADDRESS = 0x101000B4 };

enum { STACK_BOTTOM = 0x1FF00000 };
enum { STACK_TOP =    0x20000000 };

// callback for tracing invalid memory access (READ/WRITE/EXEC)
static bool hook_mem_invalid(uc_engine* uc, uc_mem_type type,
    uint64_t addr, int size, int64_t value, void* user_data)
{
    switch (type) {
    default:
        printf("not ok - UC_HOOK_MEM_INVALID type: %d at 0x%" PRIx64 "\n", type, addr);
        return false;
    case UC_MEM_READ_UNMAPPED:
        printf("not ok - Read from invalid memory at 0x%"PRIx64 ", data size = %u\n", addr, size);
        return false;
    case UC_MEM_WRITE_UNMAPPED:
        printf("not ok - Write to invalid memory at 0x%"PRIx64 ", data size = %u, data value = 0x%"PRIx64 "\n", addr, size, value);
        return false;
    case UC_MEM_FETCH_PROT:
        printf("not ok - Fetch from non-executable memory at 0x%"PRIx64 "\n", addr);
        return false;
    case UC_MEM_WRITE_PROT:
        printf("not ok - Write to non-writeable memory at 0x%"PRIx64 ", data size = %u, data value = 0x%"PRIx64 "\n", addr, size, value);
        return false;
    case UC_MEM_READ_PROT:
        printf("not ok - Read from non-readable memory at 0x%"PRIx64 ", data size = %u\n", addr, size);
        return false;
    }
}

#define ERR_CHECK() if (err != UC_ERR_OK) {\
        printf("Failed with error returned: %u: %s\n", err, uc_strerror(err));\
        return -1;\
    }

int load_rom(const char* path) {
    FILE* f = fopen(path, "rb");

    fseek(f, OFFSET, SEEK_SET);
    uint8_t buffer[LOAD_SIZE];
    fread(buffer, 1, LOAD_SIZE, f);
    fclose(f);

    uc_engine* uc;
    uc_err err;
    
    int r_edx = 0x7890;     // EDX register

    printf("Emulate ARMv5 code\n");

    // Initialize emulator in X86-32bit mode
    err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);
    ERR_CHECK();

    // allocate some space for stack
    err = uc_mem_map(uc, STACK_BOTTOM, STACK_TOP - STACK_BOTTOM, UC_PROT_READ | UC_PROT_WRITE);
    ERR_CHECK();

    // map memory for this emulation
    int alloc_size = (ALLOC_SIZE + 0x3fff) & ~0x3fff;

    err = uc_mem_map(uc, LOAD_ADDRESS, alloc_size, UC_PROT_ALL);
    ERR_CHECK();

    // write machine code to be emulated to memory
    err = uc_mem_write(uc, LOAD_ADDRESS, buffer, sizeof(buffer));
    ERR_CHECK();

    // intercept invalid memory events
    uc_hook trace1;
    err = uc_hook_add(uc, &trace1, UC_HOOK_MEM_INVALID, hook_mem_invalid, NULL, 1, 0);
    ERR_CHECK();

    // Set stack pointer
    int sp = STACK_TOP;
    uc_reg_write(uc, UC_ARM_REG_SP, &sp);

    // Execute binary init (R0=??, R1=0)
    int r1 = 0;
    uc_reg_write(uc, UC_ARM_REG_R1, &r1);

    err = uc_emu_start(uc, ENTRY_ADDRESS, 0x1010019C, 0, 0);
    ERR_CHECK();

    // Execute AppMain (R0=Dl)

    err = uc_emu_start(uc, APPMAIN_ADDRESS, LOAD_ADDRESS + LOAD_SIZE, 0, 0);
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
}
