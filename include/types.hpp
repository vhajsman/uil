#ifndef __TYPES_H
#define __TYPES_H

#include <cstdint>
#include <string>

namespace uil {
    enum class type_kind {
        VOID,
        INTEGER,
        CHAR
    };

    enum type_flags {
        SIGN     = 1 << 0,
        USER     = 1 << 1,
        INTEGRAL = 1 << 2,
        NUMERIC  = 1 << 3
    };

    struct type {
        std::string name;
        size_t size;
        uint8_t flags;

        type_kind kind;
    };

    const type* get_type_by_name(const std::string& name);

    inline const type TYPE_INT8  {"int8",  1, (INTEGRAL | NUMERIC), type_kind::INTEGER};
    inline const type TYPE_INT16 {"int16", 2, (INTEGRAL | NUMERIC), type_kind::INTEGER};
    inline const type TYPE_INT32 {"int32", 4, (INTEGRAL | NUMERIC), type_kind::INTEGER};
    inline const type TYPE_INT64 {"int64", 8, (INTEGRAL | NUMERIC), type_kind::INTEGER};

    inline const type TYPE_UINT8  {"uint8",  1, (INTEGRAL | NUMERIC | SIGN), type_kind::INTEGER};
    inline const type TYPE_UINT16 {"uint16", 2, (INTEGRAL | NUMERIC | SIGN), type_kind::INTEGER};
    inline const type TYPE_UINT32 {"uint32", 4, (INTEGRAL | NUMERIC | SIGN), type_kind::INTEGER};
    inline const type TYPE_UINT64 {"uint64", 8, (INTEGRAL | NUMERIC | SIGN), type_kind::INTEGER};

    inline const type TYPE_CHAR {"char", 1, (INTEGRAL), type_kind::CHAR};

    inline const type TYPE_VOID {"void", 0, 0, type_kind::VOID};
};

#endif
