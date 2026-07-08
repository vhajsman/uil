#ifndef __META_BUILDER_H
#define __META_BUILDER_H

#include "executable.hpp"
#include "symbols.hpp"
#include "types.hpp"
#include <vector>

namespace uil {
    executable_meta build_meta(SymbolTable& symbol_table, std::vector<const type*>& types_vect);
};

#endif
