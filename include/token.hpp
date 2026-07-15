#ifndef __TOKEN_H
#define __TOKEN_H

#include <string>

namespace uil {
    enum class token_type {
        LParen,
        RParen,
        LCurly,
        RCurly,
        Comma,
        TypeKeyword,
        Identifier,
        Number,
        Plus,
        Minus,
        Mul,
        Div,
        Assign,
        Semicolon,
        Function,
        Return,
        PtrNull,
        PtrKeyword,
        PtrAddrOf,
        PtrDeref,
        EndOfFile
    };

    struct token {
        enum token_type type;
        std::string text;
    };

    inline std::string token_type_to_string(enum token_type type) {
        switch(type) {
            case token_type::LParen: return "LParen";
            case token_type::RParen: return "RParen";
            case token_type::LCurly: return "LCurly";
            case token_type::RCurly: return "RCurly";
            case token_type::Comma: return "Comma";
            case token_type::TypeKeyword: return "TypeKeyword";
            case token_type::Identifier: return "Identifier";
            case token_type::Number: return "Number";
            case token_type::Plus: return "Plus";
            case token_type::Minus: return "Minus";
            case token_type::Mul: return "Mul";
            case token_type::Div: return "Div";
            case token_type::Assign: return "Assign";
            case token_type::Semicolon: return "Semicolon";
            case token_type::Function: return "Function";
            case token_type::Return: return "Return";
            case token_type::PtrNull: return "Null pointer";
            case token_type::PtrAddrOf: return "Reference";
            case token_type::PtrDeref: return "Dereference";
            case token_type::EndOfFile: return "EndOfFile";
        }

        return "?";
    }
};

#endif