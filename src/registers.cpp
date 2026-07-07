#include "registers.hpp"
#include <stdexcept>

namespace uil {
    uint32_t used = 0;

    register_id alloc_temp() {
        for(int i = TEMP_REGISTER_LAST; i >= TEMP_REGISTER_FIRST; i--) {
            uint32_t mask = 1u << i;

            if(!(used & mask)) {
                used |= mask;
                return static_cast<register_id>(i);
            }
        }

        throw std::runtime_error("Out of temporary registers");
    }

    void free_temp(register_id id) {
        if(id < TEMP_REGISTER_FIRST || id > TEMP_REGISTER_LAST)
            throw std::runtime_error("Trying to free non-temporary register");

        used &= ~(1u << id);
    }

    bool is_used(register_id id) {
        return used & (1u << id);
    }
}