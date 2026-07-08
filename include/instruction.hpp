#ifndef __INSTRUCTION_H
#define __INSTRUCTION_H

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
namespace uil {
    enum class instruction_operand_type {
        NULLOP    = 0b0000,
        REGISTER  = 0b0001,
        IMMEDIATE = 0b0010,
        ADDRESS   = 0b0100,
        CONST     = 0b1000
    };

    struct instruction_operand {
        enum instruction_operand_type type;
        uint32_t data;
    };

    #define OPERAND_NULL                                    \
        (instruction_operand) {                             \
            .type = instruction_operand_type::NULLOP,       \
            .data = 0                                       \
        }

    inline std::string instruction_operand_to_string(instruction_operand* op) {
        switch(op->type) {
            case instruction_operand_type::NULLOP:
                return "...";
            case instruction_operand_type::REGISTER:
                return "r:" + std::to_string(op->data); // TODO: register names
            case instruction_operand_type::IMMEDIATE:
            case instruction_operand_type::CONST:
                return "$" + std::to_string(op->data);
            case instruction_operand_type::ADDRESS:
                return "[" + std::to_string(op->data) + "]";
            default: 
                return "?";
        }
    }

    enum instruction_opcode: uint16_t {
        NOP  = 0x0000,
        MOV  = 0x0001,
        ADD  = 0x0002,
        SUB  = 0x0003,
        MUL  = 0x0004,
        DIV  = 0x0005,
        AND  = 0x0006,
        OR   = 0x0007,
        XOR  = 0x0008,
        NOT  = 0x0009,
        SHL  = 0x000A,
        SHR  = 0x000B,
        JMP  = 0x0010,
        JZ   = 0x0011,
        JNZ  = 0x0012,
        LDI  = 0x0013,
        STI  = 0x0014,
        LDM  = 0x0015,
        STM  = 0x0016,
        CALL = 0x0020,
        RET  = 0x0021,
        PUSH = 0x0030,
        POP  = 0x0031,
        HALT = 0xFFFF
    };

    inline std::string instruction_opcode_to_string(enum instruction_opcode opcode) {
        switch(opcode) {
            case instruction_opcode::NOP: return "NOP";
            case instruction_opcode::MOV: return "MOV";
            case instruction_opcode::ADD: return "ADD";
            case instruction_opcode::SUB: return "SUB";
            case instruction_opcode::MUL: return "MUL";
            case instruction_opcode::DIV: return "DIV";
            case instruction_opcode::AND: return "AND";
            case instruction_opcode::OR:  return "OR";
            case instruction_opcode::XOR: return "XOR";
            case instruction_opcode::NOT: return "NOT";
            case instruction_opcode::SHL: return "SHL";
            case instruction_opcode::SHR: return "SHR";
            case instruction_opcode::JMP: return "JMP";
            case instruction_opcode::JZ:  return "JZ";
            case instruction_opcode::JNZ: return "JNZ";
            case instruction_opcode::LDI: return "LDI";
            case instruction_opcode::STI: return "STI";
            case instruction_opcode::LDM: return "LDM";
            case instruction_opcode::STM: return "STM";
            case instruction_opcode::CALL:return "CALL";
            case instruction_opcode::RET: return "RET";
            case instruction_opcode::PUSH:return "PUSH";
            case instruction_opcode::POP: return "POP";

            case instruction_opcode::HALT: return "HALT";
        }

        return "?";
    }

    #define INSTRUCTION_MAX_OPERANDS 4
    #define INSTRUCTION_SIZE (sizeof(uint16_t) + (sizeof(instruction_operand) * INSTRUCTION_MAX_OPERANDS))

    struct instruction {
        uint16_t opcode;
        std::vector<instruction_operand> operands;
        
        size_t offset;
    };

    inline std::string format_hex(uint32_t value) {
        std::stringstream ss;

        ss  << "0x"
            << std::uppercase
            << std::hex
            << std::setw(8)
            << std::setfill('0')
            << value;

        return ss.str();
    }

    inline std::string instruction_to_string(instruction* ins) {
        std::string str = format_hex(ins->offset) + " : " + instruction_opcode_to_string(static_cast<instruction_opcode>(ins->opcode)) + " ";

        for(size_t i = 0; i < ins->operands.size(); i++) {
            str += instruction_operand_to_string(&(ins->operands[i]));
            if(i < ins->operands.size() - 1)
                str += ", ";
        }

        return str;
    }
};

#endif
