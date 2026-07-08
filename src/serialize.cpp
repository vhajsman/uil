#include "instruction.hpp"
#include "uil.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace uil {
    std::vector<uint8_t> CompilerInstance::serialize_program(const std::vector<instruction>& program, size_t initial_reserve_size) {
        std::vector<uint8_t> bytecode;
        if(initial_reserve_size > 0)
            bytecode.reserve(initial_reserve_size);

        for(const instruction& ins: program) {
            this->serialize_instruction((instruction&) ins, bytecode);
        }

        return bytecode;
    }

    static inline void insert_opcode(instruction& ins, std::vector<uint8_t>& bytecode) {
        bytecode.push_back(static_cast<uint8_t>(ins.opcode & 0xFF));
        bytecode.push_back(static_cast<uint8_t>((ins.opcode >> 8) & 0xFF));
    }

    static inline void insert_operand(instruction_operand& operand, std::vector<uint8_t>& bytecode) {
        uint8_t type = 0;
        switch(operand.type) {
            case instruction_operand_type::NULLOP:    type = 0b0000; break;
            case instruction_operand_type::REGISTER:  type = 0b0001; break;
            case instruction_operand_type::IMMEDIATE: type = 0b0010; break;
            case instruction_operand_type::ADDRESS:   type = 0b0100; break;
            case instruction_operand_type::CONST:     type = 0b1000; break;
        }

        // operand type (1 byte)
        bytecode.push_back(type);

        // operand data (4 bytes)
        bytecode.push_back(static_cast<uint8_t>( operand.data        & 0xFF));
        bytecode.push_back(static_cast<uint8_t>((operand.data >> 8)  & 0xFF));
        bytecode.push_back(static_cast<uint8_t>((operand.data >> 16) & 0xFF));
        bytecode.push_back(static_cast<uint8_t>((operand.data >> 24) & 0xFF));
    }
    
    void CompilerInstance::serialize_instruction(instruction& ins, std::vector<uint8_t>& bytecode) {
        insert_opcode(ins, bytecode);

        instruction_operand op_null = OPERAND_NULL;
        for(int i = 0; i < INSTRUCTION_MAX_OPERANDS; i++)
            insert_operand((i < ins.operands.size()) ? ins.operands[i] : op_null, bytecode);
    }
};
