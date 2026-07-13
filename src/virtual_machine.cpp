#include "virtual_machine.hpp"
#include "executable.hpp"
#include "instruction.hpp"
#include "registers.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace uil {
    VirtualMachine::VirtualMachine() {
        memset(&this->regs, 0x00, sizeof(register_file));
    }

    VirtualMachine::~VirtualMachine() {}

    void VirtualMachine::boot(const std::string& filename) {
        this->image = load_executable(filename);
        if(memcmp(this->image.header.magic, "UILUIL", 6))
            throw std::runtime_error("Image corrupted or program not loaded");
    }

    uint64_t VirtualMachine::read_memory(uint32_t address) {
        if(address + sizeof(uint64_t) > this->memory_size)
            throw std::runtime_error("memory read out of bounds");

        uint64_t value;

        memcpy(&value, &memory[address], sizeof(uint64_t));
        return value;
    }

    void VirtualMachine::write_memory(uint32_t address, uint64_t val) {
        if(address + sizeof(uint64_t) > memory_size)
            throw std::runtime_error("memory write out of bounds");

        memcpy(&memory[address], &val, sizeof(uint64_t));
    }

//    uint64_t* VirtualMachine::cast_operand(instruction_operand& op) {
//        switch(op.type) {
//            case instruction_operand_type::NULLOP:
//                return nullptr;
//            case instruction_operand_type::REGISTER: {
//                if(op.data >= sizeof(this->regs) / sizeof(register_t))
//                    throw std::runtime_error("Invalid register");
//
//                return &this->regs.raw[op.data].value;
//            }
//
//            case instruction_operand_type::ADDRESS: {
//                if(op.data >= this->memory_size)
//                    throw std::runtime_error("Invalid memory address");
//
//                return reinterpret_cast<uint64_t*>(this->memory + op.data);
//            }
//
//            case instruction_operand_type::CONST:
//            case instruction_operand_type::IMMEDIATE:
//                // ...
//        }
//s    }

    void VirtualMachine::run() {
        uint64_t ip = this->image.header.code_offset;
        this->register_set(this->regs.ip, ip);

        this->register_set(this->regs.sp, this->stack_end);
        this->register_set(this->regs.fp, this->stack_end);
        
        this->halt = false;
    }

    void VirtualMachine::run_instruction(instruction& ins) {
        switch(ins.opcode) {
            case NOP:
                break;

            case POP: {
                this->pop();
                break;
            }

            case PUSH: {
                if(ins.operands.empty() || ins.operands[0].type == instruction_operand_type::NULLOP)
                    throw std::runtime_error("Invalid operand");

                this->push(uint64_t val)
            }
        
        }
    }

    void virtual_machine_main(kit_params* params) {
        VirtualMachine* vm = new VirtualMachine();
        vm->boot(params->input_file);
        vm->run();
    }
};
