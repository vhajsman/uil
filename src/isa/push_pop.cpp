#include "virtual_machine.hpp"

namespace uil {
    void VirtualMachine::push(uint64_t val) {
        if(regs.sp.value < stack_start + sizeof(uint64_t))
            throw std::runtime_error("Stack overflow");

        regs.sp.value -= sizeof(uint64_t);
        write_memory(regs.sp.value, val);
    }

    uint64_t VirtualMachine::pop() {
        if(this->regs.sp.value + sizeof(uint64_t) > this->STACK_TOP)
            throw std::runtime_error("Stack underflow");

        uint64_t val = this->read_memory(this->regs.sp.value);
        this->regs.sp.value += sizeof(uint64_t);

        return val;
    }
};
