#include "executable.hpp"
#include "instruction.hpp"
#include "print_formatter.hpp"
#include "uil.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>

namespace uil {
    static const char* input_contents;
    static BytecodeStream* stream;

    static uint32_t code_start;

    inline static std::string format_offset(uint32_t value) {
        std::stringstream ss;
        ss << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << value;
        return ss.str();
    }

    static void print_operand(instruction_operand& operand, bool color = true, bool en_lookup = true) {
        switch(operand.type) {
            case instruction_operand_type::NULLOP:
                return;

            case instruction_operand_type::REGISTER: {
                if(color) std::cout << make_ansi_code(COLOR_IDX_YELLOW);
                std::cout << "r:" << operand.data << " "; // TODO: register names
                if(color) std::cout << ansi_reset();

                break;
            }

            case instruction_operand_type::IMMEDIATE: {
                if(color) std::cout << make_ansi_code(COLOR_IDX_PURPLE);
                std::cout << "#" << operand.data << " ";
                if(color) std::cout << ansi_reset();

                break;
            }

            case instruction_operand_type::CONST: {
                if(color) std::cout << make_ansi_code(COLOR_IDX_CYAN);
                std::cout << "$" << operand.data << " ";
                if(color) std::cout << ansi_reset();

                break;
            }
            
            case instruction_operand_type::ADDRESS: {
                if(color) std::cout << make_ansi_code(COLOR_IDX_GREEN);
                std::cout << "[" << operand.data << "] "; // TODO: resolve symbol name
                if(color) std::cout << ansi_reset();

                break;
            }
        }
    }

    static void print_instruction(instruction& ins, uint32_t offset, bool color = true, bool en_lookup = true) {
        std::cout << format_offset(offset) << "  | ";

        // opcode
        if(color) std::cout << make_ansi_code(COLOR_IDX_WHITE);
        std::cout << instruction_opcode_to_string(static_cast<instruction_opcode>(ins.opcode)) << "\t ";
        if(color) std::cout << ansi_reset();
        
        for(int i = 0; i < ins.operands.size(); i++) {
            print_operand(ins.operands[i], color, en_lookup);
        }

        std::cout << std::endl;
    }

    void disassembler(struct uil::kit_params* params) {
        if(!params)
            return;

        if(params->input_file.empty())
            throw std::runtime_error("No input file specified");

        executable_image image = load_executable(params->input_file);

        std::cout << "Loaded instructions: " << image.code.size() << std::endl << std::endl;
        code_start = image.header.code_offset;
        
        int i = 0;
        for(instruction& ins: image.code) {
            print_instruction(ins, i * INSTRUCTION_SIZE);
            i++;
        }
    }
};