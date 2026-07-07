#ifndef __LEXER_H
#define __LEXER_H

#include "token.hpp"
#include <string>
#include <vector>

namespace uil {
    class Lexer {
        private:
        std::string& input;
        size_t pos;

        public:
        Lexer(std::string& input);
        char peek();
        char get();

        std::vector<token> tokenize();
    };

};

#endif
