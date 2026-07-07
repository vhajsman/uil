#include "types.hpp"

namespace uil {
    const type* get_type_by_name(const std::string& name) {
        if(name == "int8")  return &TYPE_INT8;
        if(name == "int16") return &TYPE_INT16;
        if(name == "int32") return &TYPE_INT32;
        if(name == "int64") return &TYPE_INT64;

        if(name == "uint8")  return &TYPE_UINT8;
        if(name == "uint16") return &TYPE_UINT16;
        if(name == "uint32") return &TYPE_UINT32;
        if(name == "uint64") return &TYPE_UINT64;

        if(name == "char") return &TYPE_CHAR;

        if(name == "void") return &TYPE_VOID;

        return nullptr;
    }
};