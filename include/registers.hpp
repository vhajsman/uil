#ifndef __REGISTERS_H
#define __REGISTERS_H

#include <cstdint>

namespace uil {
    constexpr uint8_t GENERAL_PURPOSE_REGISTER_COUNT = 32;
    constexpr uint8_t TEMP_REGISTER_FIRST = 16;
    constexpr uint8_t TEMP_REGISTER_LAST  = 31;
    constexpr uint8_t INVALID_REGISTER = 255;

    using register_id = uint8_t;

    struct register_t {
        uint64_t value = 0;
    };

    struct register_float_t {
        double value = 0.0;
    };

    struct register_file {
        register_t gpr[GENERAL_PURPOSE_REGISTER_COUNT];
        register_float_t fpr[GENERAL_PURPOSE_REGISTER_COUNT];

        register_t ip;
        register_t sp;
        register_t fp;
        register_t flags;
    };

    enum register_flags_mask : uint64_t {
        FLAG_ZERO     = 1ull << 0,
        FLAG_CARRY    = 1ull << 1,
        FLAG_OVERFLOW = 1ull << 2,
        FLAG_NEGATIVE = 1ull << 3,
        FLAG_ERROR    = 1ull << 15
    };

    register_id alloc_temp();
    void free_temp(register_id id);
    bool is_used(register_id id);
}

#endif
