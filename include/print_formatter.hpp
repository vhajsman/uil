#ifndef __PRINT_FORMATTER_H
#define __PRINT_FORMATTER_H

#include <cstdint>
#include <string>

#define COLOR_IDX_BLACK     0
#define COLOR_IDX_RED       1
#define COLOR_IDX_GREEN     2
#define COLOR_IDX_YELLOW    3
#define COLOR_IDX_BLUE      4
#define COLOR_IDX_PURPLE    5
#define COLOR_IDX_CYAN      6
#define COLOR_IDX_WHITE     7

#define COLOR_IDX_RESET     255

namespace uil {
    std::string make_ansi_code(
        uint8_t color_idx = COLOR_IDX_RESET,
        bool bold = false,
        bool underline = false
    );

    static inline std::string ansi_reset() {
        return make_ansi_code(COLOR_IDX_RESET, false, false);
    }
};

#endif
