#include "virtual_machine.hpp"
#include <cstdint>
#include <iostream>

namespace uil {
    void VirtualMachine::push(uint64_t val) {
        if(this->regs.sp.value < this->stack_start + sizeof(uint64_t))
            throw std::runtime_error("VM Stack overflow");

        this->regs.sp.value -= sizeof(uint64_t);
        std::cout << "[VirtualMachine] * push val=" << std::hex << val << ", new sp=" << this->regs.sp.value << std::endl;

        write_memory(this->regs.sp.value, val);
    }

    uint64_t VirtualMachine::pop() {
        if(this->regs.sp.value + sizeof(uint64_t) > this->STACK_TOP)
            throw std::runtime_error("VM Stack underflow");

        uint64_t val = this->read_memory(this->regs.sp.value);
        this->regs.sp.value += sizeof(uint64_t);

        std::cout << "[VirtualMachine] * pop val=" << std::hex << val << ", new sp=" << this->regs.sp.value << std::endl;

        return val;
    }
};
