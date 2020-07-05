#include "debug.hpp"

// AAPCS stack cannot be reconstructed without debug info, so this is an estimation
// It can be improved for example by checking which addresses have been executed, narrowing down the set of potential call sites
// Or by inspecting candidate call sites for branch instructions
void aarch32_walk_stack(uint32_t pc, uint32_t lr, uint32_t sp,
                        std::function<std::optional<uint32_t>(uint32_t addr)> read_mem32,
                        std::function<bool(uint32_t addr)> is_code_address,
                        std::function<bool(uint32_t addr)> callback
) {
    callback(pc);
    callback(lr);

    for (size_t i = 0; i < 1024; i++) {
        auto rd = read_mem32(sp);

        if (!rd.has_value()) {
            break;
        }

        //printf("[%08Xh] %08X\n", sp, *rd);

        if (is_code_address(*rd)) {
            if (!callback(*rd)) {
                break;
            }
        }

        sp += 4;
    }
}

