#include "uil.hpp"
#include <iostream>

namespace uil {
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
            // node->body.push_back(this->parse_decl());
            node->body.push_back(this->parse_statement());
        }

        this->except_token(token_type::RCurly);

        return node;
    }
};
