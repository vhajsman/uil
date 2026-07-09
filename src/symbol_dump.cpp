#include "executable.hpp"
#include "types.hpp"
#include "uil.hpp"
#include <cstdint>
#include <iostream>
#include <ostream>

namespace uil {
    inline static std::string format_offset(uint32_t value) {
        std::stringstream ss;
        ss << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << value;
        return ss.str();
    }

    void symbol_dump(struct uil::kit_params* params) {
        if(!params)
            return;

        if(params->input_file.empty())
            throw std::runtime_error("No input file specified");

        executable_image image = load_executable(params->input_file);
        std::cout << "Symbol dump for: " << params->input_file << std::endl << std::endl;

        std::cout << "* section " << format_offset(image.header.code_offset) << " - " << format_offset(image.header.code_size + image.header.code_offset) << " " << image.header.code_size << " bytes \t " << ".code" << std::endl;
        std::cout << "* section " << format_offset(image.header.data_offset) << " - " << format_offset(image.header.data_size + image.header.data_offset) << " " << image.header.data_size << " bytes \t " << ".data" << std::endl;
        std::cout << "* section " << format_offset(image.header.meta_offset) << " - " << format_offset(image.header.meta_size + image.header.meta_offset) << " " << image.header.meta_size << " bytes \t " << ".meta" << std::endl;

        std::cout << std::endl;

        for(uint32_t i = 0; i < image.meta.types.size(); i++) {
            executable_meta_type sym = image.meta.types[i];
            std::cout << "type " << executable_meta_get_string(image.meta, sym.name_offset) << "\t ";

            std::cout << "  " << sym.size << " bytes\t ";

            if(sym.flags & SIGN)     std::cout << "signed ";
            if(sym.flags & USER)     std::cout << "user ";
            if(sym.flags & INTEGRAL) std::cout << "int ";
            if(sym.flags & NUMERIC)  std::cout << "num ";
            
            std::cout << std::endl;
        }

        std::cout << std::endl;

        for(uint32_t i = 0; i < image.meta.symbols.size(); i++) {
            executable_meta_symbol sym = image.meta.symbols[i];
            std::cout << format_offset(sym.stack_offset) << "   ";
            std::cout << " " << executable_meta_get_string(image.meta, image.meta.types[sym.type_id].name_offset) << "\t  ";
            std::cout << ((sym.flags & SYM_FLAG_FUNCT) ? "F" : "V") << " ";
            std::cout << executable_meta_get_string(image.meta, sym.name_offset) << "\t  ";

            if(sym.flags & SYM_FLAG_GLOBAL) std::cout << "global ";
            if(sym.flags & SYM_FLAG_CONST) std::cout << "ro ";
            if(sym.flags & SYM_FLAG_PARAM) std::cout << "arg ";

            std::cout << std::endl;
        }
    }
};
