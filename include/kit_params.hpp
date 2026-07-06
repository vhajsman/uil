#ifndef __KIT_PARAMS_H
#define __KIT_PARAMS_H

namespace uil {
    enum kit_params_subcommand {
        SUBCOMMAND_HELP,
        SUBCOMMAND_VERSION,
        SUBCOMMAND_COMPILER,
        SUBCOMMAND_ASSEMBLER,
        SUBCOMMAND_VM
    };

    struct kit_params {
        enum kit_params_subcommand subcommand;
    };
};

#endif