#include "filesystem.hpp"
#include "svc_handlers.hpp"

#include <sstream>
#include <string>

extern Filesystem fs;

//static char const* get_string(uc_engine* uc, uint32_t addr) {
//    static char buf[0x1000];
//
//    for (int i = 0;; i++) {
//        uc_mem_read(uc, addr + i, &buf[i], 1);
//        if (!buf[i]) {
//            break;
//        }
//    }
//
//    return buf;
//}

static std::string get_string(uc_engine* uc, uint32_t addr) {
    std::stringstream ss;

    for (int i = 0;; i++) {
        char c;
        uc_mem_read(uc, addr + i, &c, sizeof(c));

        if (!c) {
            break;
        }

        ss << c;
    }

    return ss.str();
}

static std::wstring get_wstring(uc_engine* uc, uint32_t addr) {
    std::wstringstream ss;

    for (int i = 0;; i++) {
        uint16_t c;
        uc_mem_read(uc, addr + i * 2, &c, sizeof(c));

        if (!c) {
            break;
        }

        ss << (wchar_t) c;
    }

    return ss.str();
}

void Svc_GemeiEmu_fopen(uc_engine* uc) {
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);
    uint32_t r1; uc_reg_read(uc, UC_ARM_REG_R1, &r1);

    auto path = get_string(uc, r0);
    auto mode = get_string(uc, r1);

    printf("FOPEN(%s, %s)\n", path.c_str(), mode.c_str());
    
    uint32_t ret = fs.open_file(path);

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_fopenW(uc_engine* uc) {
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);
    uint32_t r1; uc_reg_read(uc, UC_ARM_REG_R1, &r1);

    auto path = get_wstring(uc, r0);
    auto mode = get_string(uc, r1);

    printf("fopenW('%ls', '%s')\n", path.c_str(), mode.c_str());

    uint32_t ret = fs.open_file(path);

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_panic(uc_engine* uc) {
    int pc;
    uc_reg_read(uc, UC_ARM_REG_PC, &pc);
    printf("PANIC AT PC = 0x%x\n", pc);

    uc_emu_stop(uc);
}

void Svc_GemeiEmu_putc(uc_engine* uc) {
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);

    char c = r0;

    putc(c, stdout);
}
