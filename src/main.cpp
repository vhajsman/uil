#include "kit_params.hpp"
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
                    ("input", po::value<std::string>(), "Input file")
                    ("output,o", po::value<std::string>(), "Output file")
                    ("a,assembler-only", po::bool_switch(), "Only output assembler file instead of executable");
                break;

            case SUBCOMMAND_ASSEMBLER:
                desc.add_options()
                    ("help,h", "Show help")
                    ("input", po::value<std::string>(), "Input file")
                    ("output,o", po::value<std::string>(), "Output file");
                break;

            default:
                return params;
            }
            
        return params;
    }
};


int main(int argc, char** argv) {
    uil::kit_params params = uil::parse_params(argc, argv);
}