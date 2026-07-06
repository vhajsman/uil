#include "kit_params.hpp"
#include "uil.hpp"
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <cstring>
#include <iostream>

namespace po = boost::program_options;

namespace uil {
    static struct kit_params parse_params(int argc, char** argv) {
        kit_params params;

        if(argc < 2) {
            std::cout << "No subcommand specified provided." << std::endl;
            std::cout << "Available subcommands are: help, version, compile, assembler, run" << std::endl;
            exit(1);
        }

        if(strcmp(argv[1], "help") == 0)            params.subcommand = SUBCOMMAND_HELP;
        else if(strcmp(argv[1], "version") == 0)    params.subcommand = SUBCOMMAND_VERSION;
        else if(strcmp(argv[1], "compile") == 0)    params.subcommand = SUBCOMMAND_COMPILER;
        else if(strcmp(argv[1], "assembler") == 0)  params.subcommand = SUBCOMMAND_ASSEMBLER;
        else if(strcmp(argv[1], "run") == 0)        params.subcommand = SUBCOMMAND_VM;
        else {
            std::cout << "Unknown subcommand: " << argv[1] << std::endl;
            exit(1);
        }

        // argv++;
        // argc--;

        po::options_description desc("Program options");
        switch (params.subcommand) {
            case SUBCOMMAND_COMPILER:
                desc.add_options()
                    ("help,h", "Show help")
                    ("input", po::value<std::string>(&params.input_file), "Input file")
                    ("output,o", po::value<std::string>(&params.output_file), "Output file")
                    ("assembler-only,a", po::bool_switch(&params.assembler_only), "Only output assembler file instead of executable");
                break;

            case SUBCOMMAND_ASSEMBLER:
                desc.add_options()
                    ("help,h", "Show help")
                    ("input", po::value<std::string>(&params.input_file), "Input file")
                    ("output,o", po::value<std::string>(&params.output_file), "Output file");
                break;

            default:
                break;
        }

        po::variables_map vm;
        auto parsed = po::command_line_parser(argc - 1, argv + 1)
            .options(desc)
            .allow_unregistered()
            .run();

        po::store(parsed, vm);
        po::notify(vm);

        return params;
    }
};


int main(int argc, char** argv) {
    uil::kit_params params = uil::parse_params(argc, argv);

    switch(params.subcommand) {
        case uil::SUBCOMMAND_COMPILER: {
            uil::CompilerInstance compiler(&params);
            compiler.compile();

            break;
        }
    }

    return 0;
}