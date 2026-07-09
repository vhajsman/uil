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

namespace uil {
    unsigned int expr_precedence(token_type* type) {
        switch(*type) {
            case token_type::Mul:
            case token_type::Div:
                return 20;

            case token_type::Plus:
            case token_type::Minus:
                return 10;

            default:
                return 0;
        }
    }

    bool is_binop(token_type tt) {
        return tt == token_type::Plus || tt == token_type::Minus || tt == token_type::Div || tt == token_type::Mul;
    }

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
                    //std::cout << "Warning: Operands '" + node->lefthand->name + "' (" + L->name + ") and '" + node->righthand->name + "' (" + R->name + ") differ in signedness" << std::endl;

                if(L->size != R->size)
                    throw_syntax_warning("'" + node->lefthand->name + "' (" + L->name + ") and '" + node->righthand->name + "' (" + R->name + ") differ in size", 0, {}, "");
                    // std::cout << "Warning: Operands '" + node->lefthand->name + "' (" + L->name + ") and '" + node->righthand->name + "' (" + R->name + ") differ in size" << std::endl;

                return L;
            }
        }

        return nullptr;
    }

    syntax_tree_node* CompilerInstance::parse_decl() {
        token tok_type = this->except_token(token_type::TypeKeyword);
        const type* decl_type = get_type_by_name(tok_type.text);

        token tok_identifier = this->except_token(token_type::Identifier);

        if(this->tokens[this->pos].type == token_type::LParen)
            return this->parse_function_decl(decl_type, tok_identifier);

        return this->parse_var_decl(decl_type, tok_identifier);
    }

    syntax_tree_node* CompilerInstance::parse_var_decl(const type* var_type, const token& tok_identifier) {
        symbol* sym = this->symbol_table.declare_variable(tok_identifier.text, var_type);
        std::cout << "Create syntax tree node '" << tok_identifier.text << "'" << std::endl;

        syntax_tree_node* node = new syntax_tree_node(syntax_tree_node_type::DECLARATION);
        node->name = tok_identifier.text;
        node->symbol = sym;

        if(this->tokens[this->pos].type == token_type::Assign) {
            this->pos++;
            node->initial = this->parse_expr();

            (void) this->get_type(node->initial);

            // if(node->initial) {
            //     const type* initial_type = this->get_type(node->initial);
            // }
        } else {
            node->initial = nullptr;
        }

        this->except_token(token_type::Semicolon);
        return node;
    }

    syntax_tree_node* CompilerInstance::parse_function_decl(const type* return_type, const token& tok_identifier) {
        //auto* fn_type = new function_type("fn()->" + return_type->name, return_type);

        auto fn_type = std::make_unique<function_type>("fn()->" + return_type->name, return_type);
        const type* fn_type_ptr = fn_type.get();

        this->types_owned.push_back(std::move(fn_type));
        
        symbol* sym = this->symbol_table.declare_function(tok_identifier.text, fn_type_ptr);

        sym->function_info_ptr = std::make_unique<function_info>();
        sym->function_info_ptr->return_type = return_type;
        sym->function_info_ptr->symbol_self = sym;

        syntax_tree_node* node = new syntax_tree_node(syntax_tree_node_type::FN_DEF);

        node->name = tok_identifier.text;
        node->symbol = sym;
        node->ret_type = return_type;

        this->except_token(token_type::LParen);
        this->except_token(token_type::RParen);

        this->except_token(token_type::LCurly);

        while(this->tokens[this->pos].type != token_type::RCurly) {
            node->body.push_back(this->parse_decl());
        }

        this->except_token(token_type::RCurly);

        return node;
    }

    syntax_tree_node* CompilerInstance::parse_expr(int min_precedence) {
        syntax_tree_node* L = this->parse_postfix();
        if(!L)
            throw std::runtime_error("parse_postfix() returned nullptr");

        while(this->pos < this->tokens.size()) {
            token_type op = this->tokens[this->pos].type;
            if(op == token_type::Semicolon || op == token_type::EndOfFile || !is_binop(op))
                break;

            unsigned int precedence = expr_precedence(&op);
            if(precedence < min_precedence)
                break;

            this->pos++;

            syntax_tree_node* R = this->parse_expr(precedence + 1);
            if(!L || !R)
                throw_syntax_error("Invalid expression operands", 0, {}, "");
                // throw std::runtime_error("Invalid expression operands");

            binop_type op_type;
            switch(op) {
                case token_type::Plus:  op_type = binop_type::ADDITION; break;
                case token_type::Minus: op_type = binop_type::SUBTRACTION; break;
                case token_type::Div:   op_type = binop_type::DIVISION; break;
                case token_type::Mul:   op_type = binop_type::MULTIPLICATION; break;

                default:
                    break;
            }

            const type* left_type = this->get_type(L);
            const type* right_type = this->get_type(R);
            if(!left_type || !right_type)
                throw_syntax_error("Invalid expression operands", 0, {}, "");
                // throw std::runtime_error("Could not determine operand types");

            if((left_type->flags & SIGN) != (right_type->flags & SIGN))
                throw_syntax_warning("'" + L->name + "' (" + left_type->name + ") and '" + R->name + "' (" + right_type->name + ") differ in signedness", 0, {}, "");
                // std::cout << "Warning: Operands '" + L->name + "' (" + left_type->name + ") and '" + R->name + "' (" + right_type->name + ") differ in signedness" << std::endl;
            
            if(left_type->size != right_type->size)
                throw_syntax_warning("'" + L->name + "' (" + left_type->name + ") and '" + R->name + "' (" + right_type->name + ") differ in size", 0, {}, "");
                // std::cout << "Warning: Operands '" + L->name + "' (" + left_type->name + ") and '" + R->name + "' (" + right_type->name + ") differ in size" << std::endl;

            const type* result_type = left_type;
            if(right_type->size > left_type->size) {
                result_type = right_type;
            } else if((left_type->flags & SIGN) != (right_type->flags & SIGN)) {
                result_type = (left_type->flags & SIGN) ? left_type : right_type;
            }

            syntax_tree_node* node = new syntax_tree_node(syntax_tree_node_type::BINOP);
            node->lefthand = L;
            node->righthand = R;
            node->op = op_type;

            node->symbol = new symbol {
                .name = "<binop>",
                .symbol_id = 0,
                .kind = symbol_kind::VARIABLE,
                .type = result_type,
                .stack_offset = 0,
                .entry_ip = UINT32_MAX
            };

            L = node;
        }

        return L;
    }

    syntax_tree_node* CompilerInstance::parse_primary() {
        token& tok = this->tokens[this->pos];

        if(tok.type == token_type::LParen) {
            this->pos++;
            syntax_tree_node* expr = this->parse_expr();

            this->except_token(token_type::RParen);
            return expr;
        }

        if(tok.type == token_type::Number) {
            this->pos++;
            return new syntax_tree_node(std::stoll(tok.text));
        }

        if(tok.type == token_type::Identifier) {
            this->pos++;
            
            const symbol* sym = this->symbol_table.lookup(tok.text);
            if(!sym)
                throw_syntax_error("'" + tok.text + "' is not declared in current scope", 0, {}, "");
                // throw std::runtime_error("'" + tok.text + "' is not declared in current scope");

            syntax_tree_node* node = new syntax_tree_node(syntax_tree_node_type::VARIABLE);
            node->name = tok.text;
            node->symbol = const_cast<symbol*>(sym);

            return node;
        }

        throw std::runtime_error("Excepted primary exception");
    }

    syntax_tree_node* CompilerInstance::parse_postfix() {
        syntax_tree_node* node = this->parse_primary();

        while(this->pos < this->tokens.size()) {
            if(this->tokens[this->pos].type != token_type::LParen)
                break;

            if(node->name.empty() || !node->symbol)
                break;

            // if(node->symbol->kind != symbol_kind::FUNCTION) {
            //     
            // }

            syntax_tree_node* call_node = new syntax_tree_node(syntax_tree_node_type::FN_CALL);
            call_node->lefthand = node;
            call_node->symbol = node->symbol;

            this->pos++;

            if(this->tokens[this->pos].type != token_type::RParen) {
                do {
                    syntax_tree_node* arg_node = this->parse_expr();
                    call_node->args.push_back(arg_node);

                    if(this->tokens[this->pos].type != token_type::Comma)
                        break;

                    this->pos++;
                } while(true);
            }

            this->except_token(token_type::RParen);
            node = call_node;
        }

        return node;
    }

    void CompilerInstance::build_syntax_tree() {
        this->ast_owned.clear();
        this->pos = 0;

        while(this->tokens[this->pos].type != token_type::EndOfFile) {
            syntax_tree_node* node_raw = nullptr;

            switch(this->tokens[this->pos].type) {
                case token_type::TypeKeyword:
                    node_raw = this->parse_decl();
                    break;
                
                // TODO: case Function
                case token_type::Function:
                    break;

                default: {
                    node_raw = this->parse_expr();
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

                case syntax_tree_node_type::FN_DEF: {
                    node->symbol->entry_ip = ctx.instructions.size() * INSTRUCTION_SIZE;
                    for(auto* statement: node->body) {
                        instruction_operand result = this->compile_tree_node(statement, out);
                        if(check_temp_register(&result))
                            free_temp(result.data);
                    }

                    this->emit(RET, nullptr, 0);
                    return OPERAND_NULL;
                }
            }
        } catch(std::exception& e) {
            std::cerr << "compile_tree_node() failed for '" << node->name << "' type=" << int(node->type) << std::endl;
            std::cerr << "  what: " << e.what() << std::endl;

            throw e;
        }
    }
};