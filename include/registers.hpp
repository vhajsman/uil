#ifndef __REGISTERS_H
#define __REGISTERS_H

#include "instruction.hpp"
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

        register_t frv; // function return value

        register_t ip;
        register_t sp;
        register_t fp;
        register_t flags;
    };

    #define REG_FRV   (2 * GENERAL_PURPOSE_REGISTER_COUNT + 1)
    #define REG_IP    (2 * GENERAL_PURPOSE_REGISTER_COUNT + 2)
    #define REG_SP    (2 * GENERAL_PURPOSE_REGISTER_COUNT + 3)
    #define REG_FP    (2 * GENERAL_PURPOSE_REGISTER_COUNT + 4)
    #define REG_FLAGS (2 * GENERAL_PURPOSE_REGISTER_COUNT + 5)

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

    inline bool check_temp_register(instruction_operand* operand) {
        return  operand->type == instruction_operand_type::REGISTER && 
                operand->data >= TEMP_REGISTER_FIRST                && 
                operand->data <= TEMP_REGISTER_LAST;
    }
}

#endif
