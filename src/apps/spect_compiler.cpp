
/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>

#include "CpuModel.h"

#include "spect.h"
#include "InstructionDefs.h"
#include "InstructionFactory.h"
#include "Compiler.h"

#include "OptionParser.h"

enum  optionIndex {
    UNKNOWN,
    HELP,
    FIRST_ADDR,
    HEX_FORMAT,
    HEX_FILE,
    PARITY,
    DUMP_PROGRAM,
    DUMP_SYMBOLS
};

const option::Descriptor usage[] =
{
    {UNKNOWN,          0,  "" ,    ""               ,option::Arg::None,     "USAGE: spect_compiler [options]\n\n" "Options:" },
    {HELP,             0,  "h" ,    "help"          ,option::Arg::None,     "  --help                  Print usage and exit." },
    {FIRST_ADDR,       0,  ""  ,    "first-address" ,option::Arg::Optional, "  --first-address=<addr>  Address to place first instruction from first compiled file." },
    {HEX_FORMAT,       0,  ""  ,    "hex-format"    ,option::Arg::Optional, "  --hex-format=<type>     Format of hex file:\n"
                                                                            "                           0 - Hex file for Instruction simulator or SPECT DPI model (default).\n"
                                                                            "                           1 - Hex file for Verilog model (address not included)\n"
                                                                            "                           2 - Hex file for Verilog model (address included).\n"},
    {HEX_FILE,         0,  ""  ,    "hex-file"      ,option::Arg::Optional, "  --hex-file=<file>       HEX file for simulator where code will be assembled."},
    {PARITY,           0,  ""  ,    "parity"        ,option::Arg::Optional, "  --parity=<type>         Parity type:\n"
                                                                            "                           1 - Odd parity.\n"
                                                                            "                           2 - Even parity.\n"
                                                                            "                           else - No parity (default).\n"},
    {DUMP_PROGRAM,     0,  ""  ,    "dump-program"  ,option::Arg::Optional, "  --dump-program=<file>   File where program to dump compiled program (.s file with addresses)\n"},
    {DUMP_SYMBOLS,     0,  ""  ,    "dump-symbols"  ,option::Arg::Optional, "  --dump-symbols=<file>   File where dump all symbols found during compilation.\n"},

    {0,0,0,0,0,0}
};

spect::Compiler *comp;


int main(int argc, char** argv)
{
    argc-=(argc>0); argv+=(argc>0);
    option::Stats  stats(usage, argc, argv);
    option::Option options[stats.options_max], buffer[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    int ret_code = 0;

    if (parse.error()) {
        option::printUsage(std::cout, usage);
        return 1;
    }

    bool has_unknown = false;
    for (option::Option* opt = options[UNKNOWN]; opt; opt = opt->next()) {
        std::cout << "Unknown option: " << opt->name << "\n";
        has_unknown = true;
    }

    if (has_unknown) {
        option::printUsage(std::cout, usage);
        return 1;
    }

    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        return 0;
    }

    comp = new spect::Compiler(SPECT_INSTR_MEM_BASE);
    if (options[FIRST_ADDR]) {
        std::stringstream ss;
        ss << std::hex << options[FIRST_ADDR].arg;
        ss >> comp->first_addr_;
    } else {
        comp->Warning("'--first-address' undefined. First instruction will be placed at start of"
                      " instruction memory.");
    }

    // All remaining arguments are input source files
    try {
        for (int i = 0; i < parse.nonOptionsCount(); ++i) {
            std::string path = parse.nonOption(i);
                comp->Compile(path);
        }
    } catch(std::runtime_error &err) {
        std::cout << err.what() << std::endl;
        ret_code = 1;
        goto cleanup;
    }

    if (options[HEX_FILE]) {
        std::cout << "Assembling program to: " << options[HEX_FILE].arg << std::endl;
        try {
            spect::HexFileType hex_type = spect::HexFileType::ISS_WORD;
            if (options[HEX_FORMAT]) {
                if (*options[HEX_FORMAT].arg == '1')
                    hex_type = spect::HexFileType::VERILOG_RAW_WORD;
                else if (*options[HEX_FORMAT].arg == '2')
                    hex_type = spect::HexFileType::VERILOG_ADDR_WORD;
            }

            spect::ParityType parity_type = spect::ParityType::NONE;
            if (options[PARITY]) {
                if (*options[PARITY].arg == '1')
                    parity_type = spect::ParityType::ODD;
                else if (*options[PARITY].arg == '2')
                    parity_type = spect::ParityType::EVEN;
            }

            comp->program_->Assemble(std::string(options[HEX_FILE].arg), hex_type, parity_type);
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    comp->CompileFinish();

    if (options[DUMP_PROGRAM]) {
        std::cout << "Dumping program to: " << options[DUMP_PROGRAM].arg << "\n";
        std::ofstream ofs(options[DUMP_PROGRAM].arg, std::fstream::out);
        comp->program_->Dump(ofs);
        ofs.close();
    }

    if (options[DUMP_SYMBOLS]) {
        std::cout << "Dumping symbols to: " << options[DUMP_SYMBOLS].arg << "\n";
        std::ofstream ofs(options[DUMP_SYMBOLS].arg, std::fstream::out);
        comp->symbols_->Dump(ofs);
        ofs.close();
    }

cleanup:
    delete comp;
    exit(ret_code);
}
