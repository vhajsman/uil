#include "executable.hpp"
#include "instruction.hpp"
#include "registers.hpp"
#include "virtual_machine.hpp"
#include <cstdint>
#include <iostream>

namespace uil {
    void VirtualMachine::jump(uint32_t addr) {
        std::cout << "[VirtualMachine] perform jump to " << format_hex(addr - sizeof(executable_header)) << std::endl;
        std::cout << "[VirtualMachine] * write register r:" << "IP" << ": " << std::hex << this->regs.ip.value << " -> " << addr << std::endl;
        this->regs.ip.value = addr;
        this->jumped = true;
    }

    void VirtualMachine::jump_if_zero(uint32_t addr) {
        if(this->regs.flags.value & FLAG_ZERO)
            this->jump(addr);
    }

    void VirtualMachine::jump_if_not_zero(uint32_t addr) {
        if(!(this->regs.flags.value & FLAG_ZERO))
            this->jump(addr);
    }
};
