#include "virtual_machine.hpp"
#include "executable.hpp"
#include "instruction.hpp"
#include "registers.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
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

    void VirtualMachine::run() {
        uint64_t ip = this->image.header.code_offset;
        this->register_set(this->regs.ip, ip);

        this->register_set(this->regs.sp, this->stack_end);
        this->register_set(this->regs.fp, this->stack_end);

        this->memory_size = 4 * 1024; // 4 Kb
        this->memory.resize(this->memory_size);
        
        this->halt = false;

        while(!this->halt) {
            //this->run_instruction(this->image.code[this->regs.ip.value / INSTRUCTION_SIZE]);

            instruction& ins = this->image.code[(this->regs.ip.value - this->image.header.code_offset) / INSTRUCTION_SIZE];
            this->run_instruction(ins);
        }
    }

    void VirtualMachine::run_instruction(instruction& ins) {
        ins.offset = this->regs.ip.value - this->image.header.code_offset;
        std::cout << "[VirtualMachine] running instruction " << instruction_to_string(&ins) << std::endl;

        this->jumped = false;

        switch(ins.opcode) {
            case NOP:
                break;

            case MOV: {
                if(ins.operands.size() < 2 || ins.operands[0].type != instruction_operand_type::REGISTER)
                    throw std::runtime_error("Invalid operand");

                this->cast_operand_write(ins.operands[0], this->cast_operand_read(ins.operands[1]));
                break;
            }

            case ADD: {
                if(ins.operands.size() < 3)
                    throw std::runtime_error("Invalid operand");

                uint64_t result = this->cast_operand_read(ins.operands[1]) + this->cast_operand_read(ins.operands[2]);
                this->cast_operand_write(ins.operands[0], result);

                break;
            }

            case SUB: {
                if(ins.operands.size() < 3)
                    throw std::runtime_error("Invalid operand");

                uint64_t result = this->cast_operand_read(ins.operands[1]) - this->cast_operand_read(ins.operands[2]);
                this->cast_operand_write(ins.operands[0], result);

                break;
            }

            case MUL: {
                if(ins.operands.size() < 3)
                    throw std::runtime_error("Invalid operand");

                uint64_t result = this->cast_operand_read(ins.operands[1]) * this->cast_operand_read(ins.operands[2]);
                this->cast_operand_write(ins.operands[0], result);

                break;
            }

            case DIV: {
                if(ins.operands.size() < 3)
                    throw std::runtime_error("Invalid operand");

                if(this->cast_operand_read(ins.operands[2]) == 0)
                    throw std::runtime_error("Division by zero");

                uint64_t result = this->cast_operand_read(ins.operands[1]) / this->cast_operand_read(ins.operands[2]);
                this->cast_operand_write(ins.operands[0], result);

                break;
            }

            case LDM: {
                if(ins.operands.size() < 2)
                    throw std::runtime_error("Invalid operand");

                this->cast_operand_write(ins.operands[0], this->read_memory(ins.operands[1].data));
                break;
            }

            case STM: {
                if(ins.operands.size() < 2)
                    throw std::runtime_error("Invalid operand");

                this->write_memory(ins.operands[0].data, this->cast_operand_read(ins.operands[1]));
                break;
            }

            case LDI: {
                if(ins.operands.size() < 2)
                    throw std::runtime_error("Invalid operand");

                uint64_t address = this->cast_operand_read(ins.operands[1]);
                uint64_t value = this->read_memory(address);

                this->cast_operand_write(ins.operands[0], value);
                break;
            }

            case STI: {
                if(ins.operands.size() < 2)
                    throw std::runtime_error("Invalid operand");

                uint64_t address = this->cast_operand_read(ins.operands[0]);
                uint64_t value = this->cast_operand_read(ins.operands[1]);

                this->write_memory(address, value);
                break;
            }

            case POP: {
                this->pop();
                break;
            }

            case PUSH: {
                if(ins.operands.empty() || ins.operands[0].type == instruction_operand_type::NULLOP)
                    throw std::runtime_error("Invalid operand");

                this->push(this->cast_operand_read(ins.operands[0]));
                break;
            }
        
            case JMP: {
                if(ins.operands.empty())
                    throw std::runtime_error("Invalid operand");
                if(ins.operands[0].type != instruction_operand_type::ADDRESS)
                    throw std::runtime_error("JMP expects ADDRESS");

                this->jump(ins.operands[0].data + sizeof(executable_header));
                break;
            }

            case HALT: {
                this->halt = true;
                break;
            }
        }

        if(!this->jumped)
            this->regs.ip.value += INSTRUCTION_SIZE;
    }

    void virtual_machine_main(kit_params* params) {
        VirtualMachine* vm = new VirtualMachine();
        vm->boot(params->input_file);
        vm->run();
    }
};
