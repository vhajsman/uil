#include "error_handling.hpp"
#include "uil.hpp"

namespace uil {
    static unsigned int expr_precedence(token_type* type) {
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

    static inline bool is_binop(token_type tt) {
        return tt == token_type::Plus || tt == token_type::Minus || tt == token_type::Div || tt == token_type::Mul;
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

    syntax_tree_node* CompilerInstance::parse_expr_statement() {
        syntax_tree_node* expr = this->parse_expr();
        
        this->except_token(token_type::Semicolon);

        syntax_tree_node* node = new syntax_tree_node(syntax_tree_node_type::EXPR_STATEMENT);
        node->lefthand = expr;

        return node;
    }

    syntax_tree_node* CompilerInstance::parse_statement() {
        switch(this->tokens[this->pos].type) {
            case token_type::TypeKeyword:
                return this->parse_decl();
            
            case token_type::Return:
                return this->parse_function_return();
            
            default:
                return this->parse_expr_statement();
        }
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

        //throw std::runtime_error("Excepted primary exception");
        throw std::runtime_error("Excepted primary expression, got token: " + tok.text);
    }
};
