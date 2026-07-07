#include "print_formatter.hpp"
#include <cstdint>

namespace uil {
    std::string make_ansi_code(uint8_t color_idx, bool bold, bool underline) {
        std::string code = "\033[";
        bool first = true;

        if(bold) {
            code += 1;
            first = false;
        }

        if(underline) {
            if(!first)
                code += ";";

            code += "4";
            first = false;
        }

        if(color_idx == COLOR_IDX_RESET) {
            if(!first)
                code += ";";

            code += "0";
        } else {
            if(!first)
                code += ";";

            code += std::to_string(30 + color_idx);
        }

        code += "m";
        return code;
    }
};
