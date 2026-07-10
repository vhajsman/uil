#include "executable.hpp"
#include "instruction.hpp"
#include "print_formatter.hpp"
#include "symbols.hpp"
#include "uil.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>

namespace uil {
    static uint32_t code_start;

    inline static std::string format_offset(uint32_t value) {
        std::stringstream ss;
        ss << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << value;
        return ss.str();
    }

    static void print_operand(const executable_meta& meta, instruction_operand& operand, bool color = true, bool en_lookup = true) {
        switch(operand.type) {
            case instruction_operand_type::NULLOP:
                return;

            case instruction_operand_type::REGISTER: {
                if(color) std::cout << make_ansi_code(COLOR_IDX_YELLOW);
                std::cout << "r:" << std::dec << operand.data << " "; // TODO: register names
                if(color) std::cout << ansi_reset();

                break;
            }

            case instruction_operand_type::IMMEDIATE: {
                if(color) std::cout << make_ansi_code(COLOR_IDX_PURPLE);
                std::cout << "#" << std::dec << operand.data << " ";
                if(color) std::cout << ansi_reset();

                break;
            }

            case instruction_operand_type::CONST: {
                if(color) std::cout << make_ansi_code(COLOR_IDX_CYAN);
                std::cout << "$" << std::dec << operand.data << " ";
                if(color) std::cout << ansi_reset();

                break;
            }
            
            case instruction_operand_type::ADDRESS: {
                if(color) std::cout << make_ansi_code(COLOR_IDX_GREEN);

                std::cout << "[";
                if(en_lookup) {
                    const executable_meta_symbol* sym = executable_meta_resolve_symbol(meta, operand.data);
                    if(sym) {
                        std::cout << "<" << executable_meta_get_string(meta, sym->name_offset) << ">";
                    } else {
                        std::cout << "0x" << std::hex << operand.data;
                    }
                } else {
                    std::cout << "0x" << std::hex << operand.data;
                }
                std::cout << "] " << std::dec;

                if(color) std::cout << ansi_reset();

                break;
            }
        }
    }

    static void print_instruction(const executable_meta& meta , instruction& ins, uint32_t offset, bool color = true, bool en_lookup = true) {
        std::cout << format_offset(offset) << "  | \t";

        // opcode
        if(color) std::cout << make_ansi_code(COLOR_IDX_WHITE);
        std::cout << instruction_opcode_to_string(static_cast<instruction_opcode>(ins.opcode)) << "\t ";
        if(color) std::cout << ansi_reset();
        
        for(int i = 0; i < ins.operands.size(); i++) {
            print_operand(meta, ins.operands[i], color, en_lookup);
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
            if(params->enable_symbol_lookup) {
                const executable_meta_symbol* sym = executable_meta_resolve_symbol(image.meta, i * INSTRUCTION_SIZE);
                if(sym && (sym->flags & SYM_FLAG_FUNCT)) {
                    uint32_t addr = i * INSTRUCTION_SIZE + image.header.code_offset;

                    std::cout << format_offset(addr) << "  | ";
                    std::cout << executable_meta_get_string(image.meta, sym->name_offset);
                    std::cout << ":    ";

                    if(params->color)
                        std::cout << make_ansi_code(COLOR_IDX_CYAN);

                    std::cout << executable_meta_get_string(
                        image.meta,
                        image.meta.types[sym->type_id].name_offset
                    );

                    if(params->color)
                        std::cout << ansi_reset();

                    std::cout << std::endl;
                }
            }
            
            print_instruction(image.meta, ins, i * INSTRUCTION_SIZE);
            i++;
        }
    }
};