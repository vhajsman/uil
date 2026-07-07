#ifndef __SYNTAX_TREE_H
#define __SYNTAX_TREE_H

#include "symbols.hpp"
#include "types.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace uil {
    enum class syntax_tree_node_type {
        NUMBER,
        VARIABLE,
        BINOP,
        DECLARATION,
        ASSIGNMENT,
        FN_DEF,
        FN_CALL,
        FN_ARG,
        FN_RET,
    };

    enum class binop_type {
        ADDITION,
        SUBTRACTION,
        MULTIPLICATION,
        DIVISION
    };

    struct syntax_tree_node {
        syntax_tree_node_type type;
        uint32_t val;

        std::string name;

        binop_type op;
        syntax_tree_node* lefthand = nullptr;
        syntax_tree_node* righthand = nullptr;
        syntax_tree_node* initial = nullptr;

        struct symbol* symbol = nullptr;
        struct symbol* target_symbol = nullptr;

        std::vector<syntax_tree_node*> body;
        std::vector<syntax_tree_node*> args;
        const struct type* ret_type = nullptr;

        syntax_tree_node(int64_t val);
        syntax_tree_node(const std::string& varname);
        syntax_tree_node(syntax_tree_node_type t);
    };

    using syntax_tree_node_ptr = std::unique_ptr<syntax_tree_node>;
};

#endif
