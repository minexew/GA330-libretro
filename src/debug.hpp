#ifndef GA330_DEBUG_HPP
#define GA330_DEBUG_HPP

#include <cstdint>
#include <functional>
#include <optional>

void aarch32_walk_stack(uint32_t pc, uint32_t lr, uint32_t sp,
                        std::function<std::optional<uint32_t>(uint32_t addr)> read_mem32,
                        std::function<bool(uint32_t addr)> is_code_address,
                        std::function<bool(uint32_t addr)> callback
);

#endif
