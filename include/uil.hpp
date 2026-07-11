#ifndef __UIL_H
#define __UIL_H

#include "instruction.hpp"
#include "kit_params.hpp"
#include "lexer.hpp"
#include "symbols.hpp"
#include "syntax_tree.hpp"
#include "token.hpp"
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace uil {
    struct CompilerContext {
        std::vector<instruction> instructions;
        SymbolTable* symbol_table;

        uint32_t stack_top;
    };

    class CompilerInstance {
        private:
        struct uil::kit_params* params;
        std::string input_contents;

        std::vector<token> tokens;
        
        SymbolTable symbol_table;
        CompilerContext ctx;

        std::vector<syntax_tree_node_ptr> ast_owned;
        size_t pos;

        std::vector<std::unique_ptr<type>> types_owned;

        public:
        CompilerInstance(struct uil::kit_params* params);

        void compile();

        void emit(instruction_opcode opcode, const instruction_operand* operands, size_t operand_count);

        // in syntax_tree.cpp
        void build_syntax_tree();
        instruction_operand compile_tree_node(syntax_tree_node* node, std::vector<instruction>& out);
        const type* get_type(syntax_tree_node* node);
        
        syntax_tree_node* parse_decl();
        syntax_tree_node* parse_var_decl(const type* var_type, const token& tok_identifier);
        syntax_tree_node* parse_function_decl(const type* return_type, const token& tok_identifier);
        syntax_tree_node* parse_expr(int min_precedence = 0);
        syntax_tree_node* parse_primary();
        syntax_tree_node* parse_postfix();

        syntax_tree_node* parse_expr_statement();
        syntax_tree_node* parse_statement();

        syntax_tree_node* parse_function_call(syntax_tree_node* calle);
        syntax_tree_node* parse_function_argument_pass();
        syntax_tree_node* parse_function_return();

        private:
        const token& except_token(token_type type) {
            if(this->tokens[this->pos].type != type) {
                throw std::runtime_error("Unexcepted token: " + this->tokens[this->pos].text);
            }

            return this->tokens[this->pos++];
        }

        const token& except_token(const std::string& token) {
            if(this->pos >= this->tokens.size() && this->tokens[this->pos].text != token) {
                throw std::runtime_error("Unexcepted token: " + this->tokens[this->pos].text);
            }

            return this->tokens[this->pos++];
        }

        private:
        // in serialize.cpp
        std::vector<uint8_t> serialize_program(const std::vector<instruction>& program, size_t initial_reserve_size = 256);
        void serialize_instruction(instruction& ins, std::vector<uint8_t>& bytecode);
    };

    void disassembler(struct uil::kit_params* params);
    void symbol_dump(struct uil::kit_params* params);
};

#endif
