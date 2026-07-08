#include "error_handling.hpp"
#include "symbols.hpp"
#include "types.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

#define FIRST_SYMBOL_OFFSET 0x1F

namespace uil {
    symbol* symbol_scope::declare(const std::string& name, const struct type* type, enum symbol_kind kind, size_t align_head, size_t align_tail) {
        if(this->symbols.count(name) > 0) {
            // throw std::runtime_error("Symbol already declared: " + name);
            throw_syntax_error("'" + name + "' is already declared in this scope", 0, {}, "");
        }

        symbol new_symbol {  
            .name = name,
            .symbol_id = 0,
            .kind = kind,
            .type = type,
            .stack_offset = static_cast<uint32_t>(this->next_offset + align_head),
            .entry_ip = static_cast<uint32_t>(-1)
        };

        auto it = this->symbols.emplace(name, new_symbol);
        this->next_offset = type->size + this->next_offset + align_head + align_tail;
        
        return &(it.first->second);
    }

    const symbol* symbol_scope::lookup(const std::string& name) {
        auto it = this->symbols.find(name);
        if(it != this->symbols.end())
            return &(it->second);

        if(this->parent != nullptr)
            return this->parent->lookup(name);

        return nullptr;
    }

    const symbol* symbol_scope::lookup(uint32_t symbol_id) {
        for(const auto& [name, sym] : this->symbols) {
            if(sym.symbol_id == symbol_id)
                return &sym;
        }

        if(this->parent != nullptr)
            return this->parent->lookup(symbol_id);

        return nullptr;
    }

    SymbolTable::SymbolTable() {
        this->scope_global = new symbol_scope {
            .name = "<global>",
            .parent = nullptr,
            .next_offset = FIRST_SYMBOL_OFFSET
        };

        this->scope_current = this->scope_global;
        this->next_symbol_id = 1;
    }

    SymbolTable::~SymbolTable() {
        delete this->scope_global;
    }

    symbol_scope* SymbolTable::enter_scope(const std::string& name) {
        symbol_scope* s = new symbol_scope;
        s->name = name;
        s->parent = this->scope_current;
        s->next_offset = this->scope_current->next_offset;

        this->scope_current = s;
        return s;
    }

    symbol_scope* SymbolTable::leave_scope() {
        if(this->scope_current->parent == nullptr)
            throw std::runtime_error("Cannot exit global scope");

        symbol_scope* old = this->scope_current;
        this->scope_current = this->scope_current->parent;
        
        delete old;

        return this->scope_current;
    }

    symbol* SymbolTable::declare(const std::string& name, const struct type* type, enum symbol_kind kind, size_t align_head, size_t align_tail) {
        symbol* sym = this->scope_current->declare(name, type, kind, align_head, align_tail);
        sym->symbol_id = this->next_symbol_id++;

        std::cout << "Declared symbol: " << name << " (id: " << sym->symbol_id << ", type: " << type->name << ", kind: " << (kind == symbol_kind::VARIABLE ? "variable" : "function") << ", offset: " << sym->stack_offset << ")" << std::endl;
        
        return sym;
    }

    const symbol* SymbolTable::lookup(const std::string& name) {
        return const_cast<symbol*>(this->scope_current->lookup(name));
    }

    const symbol* SymbolTable::lookup(uint32_t symbol_id) {
        return const_cast<symbol*>(this->scope_current->lookup(symbol_id));
    }

    symbol_scope* SymbolTable::get_current_scope() const {
        return this->scope_current;
    }

    symbol_scope* SymbolTable::get_global_scope() const {
        return this->scope_global;
    }
};