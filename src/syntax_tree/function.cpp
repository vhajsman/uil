#include "error_handling.hpp"
#include "uil.hpp"

// ! function declaration can be found at src/syntax_tree/decl.cpp

namespace uil {
    syntax_tree_node* CompilerInstance::parse_function_call(syntax_tree_node* calle) {
        if(!calle->symbol)
            throw_syntax_error("call of an undeclared symbol", 0, {}, "");

        if(calle->symbol->kind != symbol_kind::FUNCTION) {
            throw_syntax_note("'" + calle->symbol->name + "' is a variable of type " + calle->symbol->type->name, 0, {}, "");
            throw_syntax_error("'" + calle->symbol->name + "' is not a function", 0, {}, "");
        }

        syntax_tree_node* node = new syntax_tree_node(syntax_tree_node_type::FN_CALL);
        node->lefthand = calle;
        node->symbol = calle->symbol;

        this->except_token(token_type::LParen);

        if(this->tokens[this->pos].type != token_type::RParen) {
            while(true) {
                node->args.push_back(this->parse_expr());
                
                if(this->tokens[this->pos].type != token_type::Comma)
                    break;

                this->pos++;
            }
        }

        this->except_token(token_type::RParen);
        return node;
    }

    syntax_tree_node* CompilerInstance::parse_function_return() {
        syntax_tree_node* node = new syntax_tree_node(syntax_tree_node_type::FN_RET);
        this->pos++;

        if(this->tokens[this->pos].type != token_type::Semicolon)
            node->lefthand = this->parse_expr();

        this->except_token(token_type::Semicolon);
        return node;
    }

    // syntax_tree_node* CompilerInstance::parse_ptr_decl() {
    //     syntax_tree_node* node = new syntax_tree_node()
    // }

    syntax_tree_node* CompilerInstance::parse_postfix() {
        syntax_tree_node* node = this->parse_primary();

        while(this->pos < this->tokens.size()) {
            if(this->tokens[this->pos].type == token_type::LParen) {
                node = this->parse_function_call(node);
                continue;
            }

            break;
        }

        return node;
    }

    //syntax_tree_node* CompilerInstance::parse_function_call() {
    //    return this->parse_postfix();
    //}
};
