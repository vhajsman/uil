#include "instruction.hpp"
#include "registers.hpp"
#include "virtual_machine.hpp"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>

namespace uil {
//     uint64_t* VirtualMachine::cast_operand(instruction_operand& op) {
//         switch(op.type) {
//             case instruction_operand_type::REGISTER: {
//                 if(op.data >= sizeof(register_file)/sizeof(register_t))
//                     throw std::runtime_error("Invalid register");
// 
//                 return &regs.raw[op.data].value;
//             }
// 
//             case instruction_operand_type::ADDRESS: {
//                 if(op.data + sizeof(uint64_t) > memory.size())
//                     throw std::runtime_error("Invalid memory address");
// 
//                 return reinterpret_cast<uint64_t*>(memory.data() + op.data);
//             }
// 
//             default:
//                 throw std::runtime_error("Operand is not reference");
//         }
//     }
// 
//     uint64_t VirtualMachine::read_operand_imm(instruction_operand& op) {
//         switch(op.type) {
//             case instruction_operand_type::REGISTER:
//             case instruction_operand_type::ADDRESS:
//                 return *this->cast_operand(op);
// 
//             case instruction_operand_type::IMMEDIATE:
//             case instruction_operand_type::CONST:
//                 return op.data;
// 
//             case instruction_operand_type::NULLOP:
//             default:
//                 return 0;
//         }
//     }

    uint64_t VirtualMachine::cast_operand_read(instruction_operand& op) {
        switch(op.type) {
            case instruction_operand_type::NULLOP:
                return 0;

            case instruction_operand_type::IMMEDIATE:
            case instruction_operand_type::CONST:
                return op.data;

            case instruction_operand_type::REGISTER: {
                if(op.data >= sizeof(register_file)/sizeof(register_t))
                    throw std::runtime_error("invalid register");

                return this->regs.raw[op.data].value;
            }

            case instruction_operand_type::ADDRESS:
                return this->read_memory(static_cast<uint32_t>(op.data));

            default:
                throw std::runtime_error("Invalid operand");
        }
    }

    void VirtualMachine::cast_operand_write(instruction_operand& op, uint64_t val) {
        switch(op.type) {
            case instruction_operand_type::REGISTER: {
                if(op.data >= sizeof(register_file)/sizeof(register_t))
                throw std::runtime_error("invalid register");
            
                std::cout << "[VirtualMachine] * write register r:" << op.data << ": 0x" << std::hex << this->regs.raw[op.data].value << " -> 0x" << val << std::endl;
                this->regs.raw[op.data].value = val;
                break;
            }

            case instruction_operand_type::ADDRESS: {
                this->write_memory(static_cast<uint32_t>(op.data), val);
                break;
            }

            case instruction_operand_type::IMMEDIATE:
            case instruction_operand_type::CONST:
            case instruction_operand_type::NULLOP:
                throw std::runtime_error("Operand not writable");

            default:
                throw std::runtime_error("Invalid operand");
        }
    }
};
