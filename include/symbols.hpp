#ifndef __SYMBOL_H
#define __SYMBOL_H

#include "types.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace uil {
    struct symbol;
    struct symbol_scope;
    enum class symbol_kind {
        VARIABLE,
        FUNCTION
    };

    struct function_info {
        std::vector<symbol*> args;

        uint32_t local_stack_size = 0;
        const type* return_type = nullptr;

        symbol* symbol_self = nullptr;
        symbol_scope* function_scope = nullptr;

        // uint64_t entry_ip = 0;
    };

    struct symbol {
        std::string name;
        uint32_t symbol_id;
        
        enum symbol_kind kind = symbol_kind::VARIABLE;
        const struct type* type;

        uint32_t stack_offset;
        uint32_t entry_ip;

        std::unique_ptr<function_info> function_info_ptr;

        ~symbol();
    };
    
    struct symbol_scope {
        std::string name;
        symbol_scope* parent;
        
        std::unordered_map<std::string, std::unique_ptr<symbol>> symbols;

        size_t next_offset;

        public:
        symbol* declare(
            const std::string& name, 
            const struct type* type, 
            enum symbol_kind kind, 
            size_t align_head = 0, 
            size_t align_tail = 0
        );

        inline symbol* declare_variable(const std::string& name, const struct type* type, size_t align_head = 0, size_t align_tail = 0) {
            return this->declare(name, type, symbol_kind::VARIABLE, align_head, align_tail);
        }

        inline symbol* declare_function(const std::string& name, const struct type* type, size_t align_head = 0, size_t align_tail = 0) {
            return this->declare(name, type, symbol_kind::FUNCTION, align_head, align_tail);
        }

        const symbol* lookup(const std::string& name);
        const symbol* lookup(uint32_t symbol_id);
    };

    class SymbolTable {
        private:
        symbol_scope* scope_global;
        symbol_scope* scope_current;

        uint32_t next_symbol_id;

        public:
        SymbolTable();
        ~SymbolTable();

        symbol_scope* enter_scope(const std::string& name);
        symbol_scope* leave_scope();

        symbol* declare(
            const std::string& name, 
            const struct type* type, 
            enum symbol_kind kind, 
            size_t align_head = 0, 
            size_t align_tail = 0
        );

        inline symbol* declare_variable(const std::string& name, const struct type* type, size_t align_head = 0, size_t align_tail = 0) {
            return this->declare(name, type, symbol_kind::VARIABLE, align_head, align_tail);
        }

        inline symbol* declare_function(const std::string& name, const struct type* type, size_t align_head = 0, size_t align_tail = 0) {
            return this->declare(name, type, symbol_kind::FUNCTION, align_head, align_tail);
        }

        const symbol* lookup(const std::string& name);
        const symbol* lookup(uint32_t symbol_id);

        symbol_scope* get_current_scope() const;
        symbol_scope* get_global_scope() const;
    };

    constexpr uint32_t FUNCTION_MAX_ARGS = 32;
};

#endif
