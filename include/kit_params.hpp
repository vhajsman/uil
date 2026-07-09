#ifndef __KIT_PARAMS_H
#define __KIT_PARAMS_H

#include <string>
namespace uil {
    enum kit_params_subcommand {
        SUBCOMMAND_HELP,
        SUBCOMMAND_VERSION,
        SUBCOMMAND_COMPILER,
        SUBCOMMAND_DISASSEMBLER,
        SUBCOMMAND_MAP,
        SUBCOMMAND_VM
    };

    struct kit_params {
        enum kit_params_subcommand subcommand;

        std::string input_file;
        std::string output_file;
        bool assembler_only;

        // --- assembler ---
        bool color;
        bool enable_symbol_lookup;
    };
};

#endif