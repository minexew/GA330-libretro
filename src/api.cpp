#include "debug.hpp"
#include "filesystem.hpp"
#include "svc_handlers.hpp"

#include <sstream>
#include <string>
#include <thread>

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

    uint32_t ret = fs.open_file(path, mode);

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_fopenW(uc_engine* uc) {
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);
    uint32_t r1; uc_reg_read(uc, UC_ARM_REG_R1, &r1);

    auto path = get_wstring(uc, r0);
    auto mode = get_string(uc, r1);

    printf("fopenW('%ls', '%s')\n", path.c_str(), mode.c_str());

    uint32_t ret = fs.open_file(path, mode);

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

static std::vector<std::byte> file_buffer;

void Svc_GemeiEmu_fread(uc_engine* uc) {
    // TODO: make some helpers ffs...
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);
    uint32_t r1; uc_reg_read(uc, UC_ARM_REG_R1, &r1);
    uint32_t r2; uc_reg_read(uc, UC_ARM_REG_R2, &r2);
    uint32_t r3; uc_reg_read(uc, UC_ARM_REG_R3, &r3);

    auto buffer_address = r0;
    auto size = r1;
    auto count = r2;
    auto file_handle = r3;

    file_buffer.reserve(size * count);

    size_t read = fs.read_file(file_handle, {file_buffer.data(), size * count}).size();
    uc_mem_write(uc, buffer_address, file_buffer.data(), read);    // TODO: check return & crash

    uint32_t ret = read / size;

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_fseek(uc_engine* uc) {
    // TODO: make some helpers ffs...
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);
    uint32_t r1; uc_reg_read(uc, UC_ARM_REG_R1, &r1);
    uint32_t r2; uc_reg_read(uc, UC_ARM_REG_R2, &r2);

    auto file_handle = r0;

    fs.seek_file(file_handle, r1, r2);
    uint32_t ret = 0;

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_fclose(uc_engine* uc) {
    // TODO: make some helpers ffs...
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);

    auto file_handle = r0;

    uint32_t ret = 0;
    fs.close_file(file_handle);

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_ftell(uc_engine* uc) {
    // TODO: make some helpers ffs...
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);

    auto file_handle = r0;

    uint32_t ret = fs.tell_file(file_handle);

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_fwrite(uc_engine* uc) {
    // TODO: make some helpers ffs...
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);
    uint32_t r1; uc_reg_read(uc, UC_ARM_REG_R1, &r1);
    uint32_t r2; uc_reg_read(uc, UC_ARM_REG_R2, &r2);
    uint32_t r3; uc_reg_read(uc, UC_ARM_REG_R3, &r3);

    auto buffer_address = r0;
    auto size = r1;
    auto count = r2;
    auto file_handle = r3;

    file_buffer.reserve(size * count);
    uc_mem_read(uc, buffer_address, file_buffer.data(), size * count);    // TODO: check return & crash

    size_t written = fs.write_file(file_handle, {file_buffer.data(), size * count}).size();

    uint32_t ret = written / size;

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_newFrame(uc_engine* uc) {
    uc_emu_stop(uc);
}

void Svc_GemeiEmu_panic(uc_engine* uc) {
    int pc;
    uc_reg_read(uc, UC_ARM_REG_PC, &pc);
    printf("PANIC AT PC = 0x%x\n", pc);

    int lr; uc_reg_read(uc, UC_ARM_REG_LR, &lr);
    int sp; uc_reg_read(uc, UC_ARM_REG_SP, &sp);

    printf("PC = %08X\tLR = %08X\tSP = %08X\n\nStack trace:\n", pc, lr, sp);

    aarch32_walk_stack(pc, lr, sp, [uc](uint32_t address) -> std::optional<uint32_t> {
        uint32_t val;
        if (uc_mem_read(uc, address, &val, 4) == UC_ERR_OK) {
            return val;
        }
        else {
            return std::nullopt;
        }
    },
    [uc](uint32_t address) -> bool {
        // application
        if (address >= 0x10000000 && address < 0x14000000) {
            return true;
        }
        // minisys
        if (address >= 0x20000000 && address < 0x20040000) {
            return true;
        }
        return false;
    },
    [](uint32_t address) {
        // To symbolize:
        // - load symbols from text file (APP.sym + minisys.sym)
        // - find nearest lower address
        // - done
        printf("%08Xh\n", address);
        return true;
    });

    uc_emu_stop(uc);
}

void Svc_GemeiEmu_putc(uc_engine* uc) {
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);

    char c = r0;

    putc(c, stdout);
}

int get_keypad_state();

void Svc_GemeiEmu_getKeyPad(uc_engine* uc) {
    uint32_t ret = get_keypad_state();

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_getTimeMs(uc_engine* uc) {
    uint32_t ret = clock() * 1000 / CLOCKS_PER_SEC;

    uc_reg_write(uc, UC_ARM_REG_R0, &ret);
}

void Svc_GemeiEmu_sleepMs(uc_engine* uc) {
    uint32_t r0; uc_reg_read(uc, UC_ARM_REG_R0, &r0);
    if (r0 > 100) {
        printf("Svc_GemeiEmu_sleepMs(%d)\n", (int) r0);
    }

    std::chrono::milliseconds timespan(r0);
    std::this_thread::sleep_for(timespan);
}
