#include "instruction.hpp"
#include "lexer.hpp"
#include "syntax_tree.hpp"
#include "uil.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

namespace uil {
    CompilerInstance::CompilerInstance(struct uil::kit_params* params) : params(params) {
        //this->lexer = Lexer(this->input_contents);
    }

    void CompilerInstance::compile() {
        if(params->input_file.empty())
            throw std::runtime_error("No input file specified");

        std::ifstream input_file;
        input_file.open(params->input_file);
        if(!input_file.is_open())
            throw std::runtime_error("Failed to open input file: " + params->input_file);

        this->input_contents = std::string(
            (std::istreambuf_iterator<char>(input_file)), 
            std::istreambuf_iterator<char>()
        );

        std::cout << "Compiling " << params->input_file << " (" << this->input_contents.size() << " bytes)" << std::endl;

        try {
            Lexer lexer(this->input_contents);
            this->tokens = lexer.tokenize();
            for(const auto& token : this->tokens) {
                std::cout << "Token: " << token_type_to_string(token.type) << " (" << token.text << ")" << std::endl;
            }

            this->build_syntax_tree();

            this->ctx.symbol_table = &this->symbol_table;
            this->ctx.stack_top = 0;

            uint32_t jmp_pos = this->ctx.instructions.size();

            instruction_operand operand_jmp[] = {
                {instruction_operand_type::IMMEDIATE, 0}
            };
            this->emit(JMP, operand_jmp, 1); // patch below

            // TOOD: functions

            // patch jmp operand
            this->ctx.instructions[jmp_pos].operands[0] = {
                .type = instruction_operand_type::IMMEDIATE,
                static_cast<uint32_t>(this->ctx.instructions.size())
            };

            this->emit(NOP, nullptr, 0);

            for(const auto& node: this->ast_owned) {
                syntax_tree_node* nodeg = node.get();
                if(nodeg->type == syntax_tree_node_type::FN_DEF)
                    continue;

                // std::cout << "Normal syntax tree node type: " << static_cast<int>(node->type) << std::endl;
                // std::cout << "    node.get() = " << (uintptr_t) nodeg << std::endl;
                // std::cout << "    node.get()->symbol = " << (uintptr_t) nodeg->symbol << std::endl;

                this->compile_tree_node(nodeg, ctx.instructions);
            }
        } catch(const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            exit(1);
        }
    }

    void CompilerInstance::emit(instruction_opcode opcode, const instruction_operand* operands, size_t operand_count) {
        if(operand_count > INSTRUCTION_MAX_OPERANDS)
            throw std::runtime_error("Too many operands for instruction");

        instruction ins;
        ins.opcode = static_cast<uint16_t>(opcode);
        ins.offset = static_cast<uint32_t>(this->ctx.instructions.size() * INSTRUCTION_SIZE);
        
        if(operand_count > 0)
            ins.operands = std::vector<instruction_operand>(operands, operands + operand_count);

        std::cout << "Emit: " << instruction_to_string(&ins) << std::endl;
        this->ctx.instructions.push_back(ins);
    }
};