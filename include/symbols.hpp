#ifndef __SYMBOL_H
#define __SYMBOL_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <unordered_map>
namespace uil {
    enum class symbol_kind {
        VARIABLE,
        FUNCTION
    };

    struct symbol {
        std::string name;
        uint32_t symbol_id;
        
        enum symbol_kind kind = symbol_kind::VARIABLE;
        const struct type* type;

        uint32_t stack_offset;
        uint32_t entry_ip;
    };
    
    struct symbol_scope {
        std::string name;
        symbol_scope* parent;
        
        std::unordered_map<std::string, symbol> symbols;

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
};

#endif
