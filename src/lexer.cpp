#include "lexer.hpp"
#include "error_handling.hpp"
#include "token.hpp"
#include "types.hpp"
#include <cctype>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace uil {
    Lexer::Lexer(std::string& input) : input(input), pos(0) {

    }

    char Lexer::peek() {
        if(pos >= input.size())
            return '\0';

        return input[pos];
    }

    char Lexer::get() {
        if(pos >= input.size())
            return '\0';

        return input[pos++];
    }

    std::vector<token> Lexer::tokenize() {
        std::vector<token> tokens;

        this->pos = 0;
        while(this->pos < this->input.size()) {
            char c = this->peek();

            if(std::isspace(c)) {
                (void) this->get();
                continue;
            }

            if(std::isdigit(c)) {
                std::string number_str;
                size_t i = 0;
                
                int base = 10;
                while(true) {
                    char c = this->peek();
                    if(c == '_' && i > 0) {
                        i++;
                        continue;
                    }

                    if(c == '0' && i == 0) {
                        number_str += this->get();
                        c = this->peek();
                        if(c == 'x') {
                            base = 16;
                            number_str += this->get();
                        } else if(c == 'b') {
                            base = 2;
                            number_str += this->get();
                        } else if(c == 'o') {
                            base = 8;
                            number_str += this->get();
                        }
                    }

                    if( (base == 10 && std::isdigit(c))         ||
                        (base == 16 && std::isxdigit(c))        ||
                        (base == 8  && (c >= '0' && c <= '7'))  ||
                        (base == 2  && (c == '0' || c == '1'))) {
                        number_str += this->get();
                    } else {
                        break;
                    }

                    i++;
                }

                tokens.push_back({token_type::Number, number_str});
                continue;
            }

            if(c == '\'') {
                (void) this->get();
                char ch = this->get();

                if(this->peek() != '\'') {
                    // throw std::runtime_error("Excepted closing quote for character literar");
                    throw_syntax_error("Excepted closing quote for character literar", 0, {}, "");
                }

                this->get();

                std::string str = std::to_string(static_cast<uint32_t>(ch));
                tokens.push_back({token_type::Number, str});
                continue;
            }

            // TODO: Strings

            if(std::isalpha(c)) {
                std::string str;
                while(std::isalnum(this->peek()) || this->peek() == '_')
                    str += this->get();

                token_type t = token_type::Identifier;

                // TODO: handle individual keywords here
                
                const type* type_ptr = get_type_by_name(str);
                if(type_ptr != nullptr) {
                    tokens.push_back({token_type::TypeKeyword, type_ptr->name});
                    continue;
                }

                tokens.push_back({t, str});
                continue;
            }

            switch(c) {
                case '+': tokens.push_back({token_type::Plus,       std::string(1, this->get())}); break;
                case '-': tokens.push_back({token_type::Minus,      std::string(1, this->get())}); break;
                case '*': tokens.push_back({token_type::Mul,        std::string(1, this->get())}); break;
                case '/': tokens.push_back({token_type::Div,        std::string(1, this->get())}); break;
                case '=': tokens.push_back({token_type::Assign,     std::string(1, this->get())}); break;
                case ';': tokens.push_back({token_type::Semicolon,  std::string(1, this->get())}); break;
                case ',': tokens.push_back({token_type::Comma,      std::string(1, this->get())}); break;
                case '(': tokens.push_back({token_type::LParen,     std::string(1, this->get())}); break;
                case ')': tokens.push_back({token_type::RParen,     std::string(1, this->get())}); break;
                case '{': tokens.push_back({token_type::LCurly,     std::string(1, this->get())}); break;
                case '}': tokens.push_back({token_type::RCurly,     std::string(1, this->get())}); break;

                default: 
                    throw std::runtime_error(std::string("Unknown token: ") + c);
            }
        }

        tokens.push_back({token_type::EndOfFile});

        return tokens;
    }
};