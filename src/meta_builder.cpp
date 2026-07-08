#include "meta_builder.hpp"
#include "executable.hpp"
#include "symbols.hpp"
#include "uil.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>

namespace uil {
    executable_meta build_meta(SymbolTable& symbol_table, std::vector<const type*>& types_vect) {
        // TODO: handle stripped executables (an option to exclude symbol and types entries)

        executable_meta meta;

        std::cout << "-------- building meta header --------" << std::endl;

        // data types
        for(size_t i = 0; i < types_vect.size(); i++) {
            const type* t = types_vect[i];

            executable_meta_type mt;
            mt.name_offset = executable_string_pool_append(meta.string_pool, t->name);
            mt.size = t->size;
            mt.flags = t->flags;

            meta.types.push_back(mt);
            std::cout << "* Created type entry: '" << t->name << "'" << std::endl;
        }

        // symbols 
        // TODO: iterate throught all scopes
        symbol_scope* scope = symbol_table.get_global_scope();
        for(auto& [name, sym]: scope->symbols) {
            executable_meta_symbol ms;
            
            //auto it = std::find(types_vect.begin(), types_vect.end(), sym.type);
            auto it = std::find_if(types_vect.begin(), types_vect.end(), [&](const type* t) {
                return t == sym.type;
            });
            
            ms.type_id = (it != types_vect.end()) ? std::distance(types_vect.begin(), it) : 0;
            ms.name_offset = executable_string_pool_append(meta.string_pool, name);
            
            // TODO: handle meta symbol flags
            ms.flags = 0;

            //ms.stack_offset = sym.kind == symbol_kind::FUNCTION ? sym.entry_ip + sizeof(executable_header) : sym.stack_offset;
            if(sym.kind == symbol_kind::FUNCTION) {
                ms.stack_offset = sym.entry_ip + sizeof(executable_header);
            } else {
                ms.stack_offset = sym.stack_offset;
            }

            meta.symbols.push_back(ms);
            std::cout << "* Created symbol entry: '" << name << "'" << std::endl;
        }

        std::cout << "* Registered " << meta.symbols.size() << " symbols, " << meta.types.size() << " types" << std::endl;
        return meta;
    }
};
