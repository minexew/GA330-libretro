#include "ccdl.h"
#include "svc_handlers.h"

#include <unicorn/unicorn.h>

#include <stdio.h>

enum { RAM_START = 0x10000000 };
enum { RAM_SIZE = 0x04000000 };

enum { OFFSET = 0x52B0 };
enum { LOAD_ADDRESS = 0x10100000 };
enum { LOAD_SIZE = 0xDF1B4 };
enum { ALLOC_SIZE = 0xE9AE0 };

enum { STACK_BOTTOM = 0x1FF00000 };
enum { STACK_TOP =    0x20000000 };

enum { MINISYS_ADDR = 0x20000000 };
enum { MINISYS_SIZE = 0x40000 };

static char const* get_string(uc_engine* uc, uint32_t addr) {
    static char buf[0x1000];

    for (int i = 0;; i++) {
        uc_mem_read(uc, addr + i, &buf[i], 1);
        if (!buf[i]) {
            break;
        }
    }

    return buf;
}

// ccpmp.bin
int Svc_GetDLHandle(uc_engine* uc, int arg0, int arg1, int arg2, int arg3) {
    return LOAD_ADDRESS;
}

// ?
int Svc_get_dl_handle(uc_engine* uc, int arg0, int arg1, int arg2, int arg3) {
    return LOAD_ADDRESS;
}

// ccpmp.bin
int Svc_printf(uc_engine* uc, int arg0, int arg1, int arg2, int arg3) {
    printf("%s", get_string(uc, arg0));
}

void Svc_GemeiEmu_putc(uc_engine* uc, int c) {
    putc(c, stdout);
}

#define LOG_STUB(name_) do { printf("STUB %s\n", (name_)); } while (0)

#define STUB(name_) int name_(uc_engine* uc, int arg0, int arg1, int arg2, int arg3) {\
    LOG_STUB(#name_);\
    return 0;\
}

STUB(Svc_get_current_language)  // ()
STUB(Svc_TaskMediaFunStop)      // ()
STUB(Svc_cmGetSysModel)         // void(char* buffer_out)
STUB(Svc_cmGetSysVersion)       // void(char* buffer_out)
STUB(Svc_PMSetMode)             // ccpmp.bin

static void hook_intr(uc_engine *uc, uint32_t intno, void *user_data) {
    int regs[4], pc;
    uc_reg_read(uc, UC_ARM_REG_R0, &regs[0]);
    uc_reg_read(uc, UC_ARM_REG_R1, &regs[1]);
    uc_reg_read(uc, UC_ARM_REG_R2, &regs[2]);
    uc_reg_read(uc, UC_ARM_REG_R3, &regs[3]);
    uc_reg_read(uc, UC_ARM_REG_PC, &pc);

    uint32_t num = 0xcccccccc;
    uc_mem_read(uc, pc - 4, &num, 4);
    num &= 0xffff;

    //printf("hook_intr[%d](%08X, %08X, %08X, %08X, %08X)\n", num, pc, regs[0], regs[1], regs[2], regs[3]);
    uint32_t ret = svc_handler_table[num](uc, regs[0], regs[1], regs[2], regs[3]);
    //printf("    -> %08X\n", ret);
    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
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

    // map memory for this emulation
    err = uc_mem_map(uc, RAM_START, RAM_SIZE, UC_PROT_ALL);
    ERR_CHECK();

    // write machine code to be emulated to memory
    err = uc_mem_write(uc, LOAD_ADDRESS, buffer, sizeof(buffer));
    ERR_CHECK();

    // intercept invalid memory events
    uc_hook trace1, trace2;
    err = uc_hook_add(uc, &trace1, UC_HOOK_MEM_INVALID, hook_mem_invalid, NULL, 1, 0);
    ERR_CHECK();
    err = uc_hook_add(uc, &trace2, UC_HOOK_INTR, hook_intr, NULL, 1, 0);
    ERR_CHECK();

    // Set stack pointer
    int sp = STACK_TOP;
    uc_reg_write(uc, UC_ARM_REG_SP, &sp);

    printf("Emulation start\n");

    // Execute binary init (R0=??, R1=0)
    int r1 = 0;
    /*uc_reg_write(uc, UC_ARM_REG_R1, &r1);

    err = uc_emu_start(uc, ENTRY_ADDRESS, 0x1010019C, 0, 0);
    ERR_CHECK();

    // Execute AppMain (R0=Dl)

    err = uc_emu_start(uc, APPMAIN_ADDRESS, LOAD_ADDRESS + LOAD_SIZE, 0, 0);*/

    // Jump to minisys
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
}
