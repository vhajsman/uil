#include "syntax_tree.hpp"
#include "error_handling.hpp"
#include "instruction.hpp"
#include "registers.hpp"
#include "symbols.hpp"
#include "token.hpp"
#include "types.hpp"
#include "uil.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace uil {
    syntax_tree_node::syntax_tree_node(int64_t val) 
        : type(syntax_tree_node_type::NUMBER), val(val) {}

    syntax_tree_node::syntax_tree_node(const std::string& varname) 
        : type(syntax_tree_node_type::VARIABLE), name(varname) {}

    syntax_tree_node::syntax_tree_node(syntax_tree_node_type t) 
        : type(t) {}

    const type* CompilerInstance::get_type(syntax_tree_node* node) {
        if(!node)
            return nullptr;

        switch(node->type) {
            case syntax_tree_node_type::DECLARATION:
                return node->symbol ? node->symbol->type : nullptr;
            case syntax_tree_node_type::ASSIGNMENT:
                return node->righthand ? this->get_type(node->righthand) : nullptr;
            case syntax_tree_node_type::NUMBER:
                return &TYPE_INT32;

            case syntax_tree_node_type::FN_ARG:
            case syntax_tree_node_type::VARIABLE: {
                if(node->symbol)
                    return node->symbol->type;

                throw std::runtime_error("Could not determine type for '" + node->name + "'");
            }

            case syntax_tree_node_type::BINOP: {
                const type* L = this->get_type(node->lefthand);
                const type* R = this->get_type(node->righthand);

                if((L->flags & SIGN) != (R->flags & SIGN))
                    throw_syntax_warning("'" + node->lefthand->name + "' (" + L->name + ") and '" + node->righthand->name + "' (" + R->name + ") differ in signedness", 0, {}, "");

                if(L->size != R->size)
                    throw_syntax_warning("'" + node->lefthand->name + "' (" + L->name + ") and '" + node->righthand->name + "' (" + R->name + ") differ in size", 0, {}, "");

                if(L->kind == type_kind::POINTER) {
                    if(node->op != binop_type::ADDITION) {
                        throw_syntax_note("Lefthand of binary operation is a pointer to '" + L->pointed_type->name + "'", 0, {}, "");
                        throw_syntax_error("Operator is not supported for pointer arithmetrics", 0, {}, "");
                    }

                    return L;
                }

                return L;
            }

            case syntax_tree_node_type::PTR_ADDR_OF: {
                type* inner = (type*) this->get_type(node->lefthand);
                if(!inner)
                    throw std::runtime_error("unable to take address");

                return make_pointer(inner);
            }

            case syntax_tree_node_type::PTR_DEREF: {
                const type* pointer = this->get_type(node->lefthand);
                
                if(!pointer)
                    throw std::runtime_error("got null type attempting PTR_DEREF");
                if(pointer->kind != type_kind::POINTER)
                    throw std::runtime_error("type is not a pointer type");

                return pointer->pointed_type;
            }
        }

        return nullptr;
    }

    void CompilerInstance::build_syntax_tree() {
        this->ast_owned.clear();
        this->pos = 0;

        while(this->tokens[this->pos].type != token_type::EndOfFile) {
            syntax_tree_node* node_raw = nullptr;

            switch(this->tokens[this->pos].type) {
                case token_type::TypeKeyword: {
                    node_raw = this->parse_decl();
                    break;
                }
                
                // TODO: case Function
                case token_type::Function: {
                    std::cout << "token_type::Function !!!" << std::endl;
                    break;
                }

                case token_type::Identifier: {
                    node_raw = this->parse_expr_statement();
                    break;
                };

                default: {
                    node_raw = this->parse_expr_statement();
                    this->except_token(token_type::Semicolon);
                    break;
                }
            }

            if(!node_raw)
                throw std::runtime_error("parser returned null syntax tree node");

            this->ast_owned.push_back(syntax_tree_node_ptr(node_raw));
        }
    }

    instruction_operand CompilerInstance::compile_tree_node(syntax_tree_node* node, std::vector<instruction>& out) {
        if(!node) {
            std::cout << "warning: null node" << std::endl;
            return OPERAND_NULL;
        }

        instruction_operand result = OPERAND_NULL;
        instruction ins;

        std::cout << "compile node: " << (int) node->type << "(named '" << node->name << "')" << std::endl;

        try {
            switch(node->type) {
                case syntax_tree_node_type::NUMBER:
                    return {instruction_operand_type::IMMEDIATE, node->val};

                case syntax_tree_node_type::VARIABLE: {
                    // x
                    //      LDM gprX, [x]

                    register_id temp_register = alloc_temp();

                    instruction_operand operands[] = {
                        {.type = instruction_operand_type::REGISTER, temp_register},
                        {.type = instruction_operand_type::ADDRESS, node->symbol->stack_offset}
                    };
                    this->emit(LDM, operands, 2);

                    return {instruction_operand_type::REGISTER, temp_register};
                }

                case syntax_tree_node_type::ASSIGNMENT: {
                    if(!node->lefthand || !node->lefthand->symbol)
                        throw std::runtime_error("Assignment target missing");

                    instruction_operand R = this->compile_tree_node(node->righthand, out);
                    instruction_operand store_operands[] = {
                        {.type = instruction_operand_type::ADDRESS, .data = node->lefthand->symbol->stack_offset},
                        R
                    };

                    this->emit(STM, store_operands, 2);

                    if(check_temp_register(&R))
                        free_temp(R.data);

                    return OPERAND_NULL;
                }

                case syntax_tree_node_type::DECLARATION: {
                    if(!node->initial) {
                        this->emit(NOP, nullptr, 0);
                        return OPERAND_NULL;
                    }

                    instruction_operand R = this->compile_tree_node(node->initial, out);
                    instruction_operand store_operands[] = {
                        {.type = instruction_operand_type::ADDRESS, .data = node->symbol->stack_offset},
                        R
                    };

                    this->emit(STM, store_operands, 2);

                    if(check_temp_register(&R))
                        free_temp(R.data);

                    return OPERAND_NULL;
                }

                case syntax_tree_node_type::BINOP: {
                    //
                    // sum = x * 82
                    //       | | |
                    //       | | +--- R
                    //       | +----- op
                    //       +------- L
                    //

                    instruction_operand L = this->compile_tree_node(node->lefthand, out);
                    instruction_operand R = this->compile_tree_node(node->righthand, out);

                    // x + 4
                    //      ADD gprX, [x], #4

                    
                    instruction_opcode opcode;
                    switch(node->op) {
                        case binop_type::ADDITION:       opcode = ADD; break;
                        case binop_type::SUBTRACTION:    opcode = SUB; break;
                        case binop_type::MULTIPLICATION: opcode = MUL; break;
                        case binop_type::DIVISION:       opcode = DIV; break;

                        default:
                            throw std::runtime_error("invalid binary operator");
                    }

                    register_id temp_register = alloc_temp();
                    
                    std::cout << "BINOP:" << std::endl;
                    std::cout << "    L:" << instruction_operand_to_string(&L) << std::endl;
                    std::cout << "    R:" << instruction_operand_to_string(&R) << std::endl;

                    if(opcode == DIV && R.data == 0)
                        throw std::runtime_error("Division by zero");

                    instruction_operand operands[] = {
                        {instruction_operand_type::REGISTER, temp_register},
                        L, R
                    };
                    this->emit(opcode, operands, 3);

                    if(check_temp_register(&L))
                        free_temp(L.data);
                    if(check_temp_register(&R))
                        free_temp(R.data);

                    return {.type = instruction_operand_type::REGISTER, .data = temp_register};
                };

                case syntax_tree_node_type::EXPR_STATEMENT: {
                    instruction_operand result = compile_tree_node(node->lefthand, out);
                    if(check_temp_register(&result))
                        free_temp(result.data);

                    return OPERAND_NULL;
                }

                case syntax_tree_node_type::FN_DEF: {
                    node->symbol->entry_ip = ctx.instructions.size() * INSTRUCTION_SIZE;
                    bool does_return = false;

                    {
                        instruction_operand ops[] = {
                            {instruction_operand_type::REGISTER, REG_FP}
                        };
                        this->emit(PUSH, ops, 1);

                        instruction_operand mov[] = {
                            {instruction_operand_type::REGISTER, REG_SP},
                            {instruction_operand_type::REGISTER, REG_FP}
                        };
                        this->emit(MOV, mov, 2);
                    }

                    for(auto* statement: node->body) {
                        if(statement->type == syntax_tree_node_type::FN_RET)
                            does_return = true;

                        instruction_operand result = this->compile_tree_node(statement, out);

                        if(check_temp_register(&result))
                            free_temp(result.data);
                    }

                    if(!does_return) {
                        this->emit(RET, nullptr, 0);

//                        instruction_operand mov[] = {
//                            {instruction_operand_type::REGISTER, REG_FP},
//                            {instruction_operand_type::REGISTER, REG_SP}
//                        };
//                        this->emit(MOV, mov, 2);
//
//                        instruction_operand pop[] = {
//                            {instruction_operand_type::REGISTER, REG_FP}
//                        };
//                        this->emit(POP, pop, 1);
//
//                        this->emit(RET, nullptr, 0);
                    }

                    return OPERAND_NULL;
                }

                case syntax_tree_node_type::FN_RET: {
                    // instruction_operand result_operand = OPERAND_NULL;

                    if(node->lefthand) {

                        //
                        // called_fn:
                        //     ...
                        //     MOV r:FRV XXX
                        //     RET
                        //
                        // caller_fn:
                        //     ...
                        //     CALL [called_fn]
                        //     MOV gprY r:FRV
                        //     ...
                        //

                        instruction_operand op = this->compile_tree_node(node->lefthand, out);
                        instruction_operand mov[] = {
                            op,
                            {instruction_operand_type::REGISTER, REG_FRV}
                        };
                        this->emit(MOV, mov, 2);

                        if(check_temp_register(&op))
                            free_temp(op.data);
                    }

                    instruction_operand mov[] = {
                        {instruction_operand_type::REGISTER, REG_FP},
                        {instruction_operand_type::REGISTER, REG_SP}
                    };
                    emit(MOV, mov, 2);

                    instruction_operand pop[] = {
                        {instruction_operand_type::REGISTER, REG_FP}
                    };
                    emit(POP, pop, 1);

                    this->emit(RET, nullptr, 0);
                    
                    return {
                        instruction_operand_type::REGISTER,
                        REG_FRV
                    };
                }

                case syntax_tree_node_type::FN_CALL: {
                    if(!node->symbol) {
                        throw std::runtime_error("function call without symbol");
                    }

                    //
                    // called_fn:
                    //     ...
                    //     MOV r:FRV XXX
                    //     RET
                    //
                    // caller_fn:
                    //     ...
                    //     CALL [called_fn]
                    //     MOV gprY r:FRV
                    //     ...
                    //

                    register_id register_temp = alloc_temp();

                    instruction_operand operands_call[] = {
                        {instruction_operand_type::ADDRESS, node->symbol->entry_ip} // ADDR
                    };
                    this->emit(CALL, operands_call, 1);

                    instruction_operand operands_mov[] = {
                        {instruction_operand_type::REGISTER, register_temp},
                        {instruction_operand_type::REGISTER, REG_FRV}
                    };
                    this->emit(MOV, operands_mov, 2);

                    return {
                        instruction_operand_type::REGISTER,
                        register_temp
                    };
                }

                default:
                    std::cerr << "Unknown syntax tree node type: " << (int)node->type << std::endl;
                    throw std::runtime_error("Unknown AST node");
            }
        } catch(std::exception& e) {
            std::cerr << "compile_tree_node() failed for '" << node->name << "' type=" << int(node->type) << std::endl;
            std::cerr << "  what: " << e.what() << std::endl;

            throw e;
        }
    }
};