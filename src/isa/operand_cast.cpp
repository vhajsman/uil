#include "instruction.hpp"
#include "registers.hpp"
#include "virtual_machine.hpp"
#include <cstdint>

namespace uil {
    uint64_t* VirtualMachine::cast_operand(instruction_operand& op) {
        switch(op.type) {
            case instruction_operand_type::REGISTER: {
                if(op.data >= sizeof(register_file)/sizeof(register_t))
                    throw std::runtime_error("Invalid register");

                return &regs.raw[op.data].value;
            }

            case instruction_operand_type::ADDRESS: {
                if(op.data + sizeof(uint64_t) > memory.size())
                    throw std::runtime_error("Invalid memory address");

                return reinterpret_cast<uint64_t*>(memory.data() + op.data);
            }

            default:
                throw std::runtime_error("Operand is not reference");
        }
    }

    uint64_t VirtualMachine::read_operand_imm(instruction_operand& op) {
        switch(op.type) {
            case instruction_operand_type::REGISTER:
            case instruction_operand_type::ADDRESS:
                return *this->cast_operand(op);

            case instruction_operand_type::IMMEDIATE:
            case instruction_operand_type::CONST:
                return op.data;

            case instruction_operand_type::NULLOP:
            default:
                return 0;
        }
    }
};
