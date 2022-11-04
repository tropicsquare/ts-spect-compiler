
/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <exception>

#include "CpuModel.h"

#include "spect.h"
#include "InstructionDefs.h"
#include "InstructionFactory.h"
#include "Compiler.h"

#include "OptionParser.h"

enum  optionIndex { UNKNOWN, HELP, FIRST_ADDR, HEX_FORMAT, SIM_HEX};

const option::Descriptor usage[] =
{
    {UNKNOWN,          0,  "" ,    ""               ,option::Arg::None,     "USAGE: spect_compiler [options]\n\n" "Options:" },
    {HELP,             0,  "h" ,    "help"          ,option::Arg::None,     "  --help                  Print usage and exit." },
    {FIRST_ADDR,       0,  ""  ,    "first-address" ,option::Arg::Optional, "  --first-address=<addr>  Address to place first instruction from first compiled file." },
    {HEX_FORMAT,       0,  ""  ,    "hex-format"    ,option::Arg::Optional, "  --hex-format=<type>     Format of hex file:\n"
                                                                            "                           0 - Hex file for Instruction simulator or SPECT DPI model (default).\n"
                                                                            "                           1 - Hex file for Verilog model (address not included)\n"
                                                                            "                           2 - Hex file for Verilog model (address included).\n"},
    {SIM_HEX,          0,  ""  ,    "sim-hex"       ,option::Arg::Optional, "  --hex-file=<file>       HEX file for simulator where code will be assembled."},

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

    uint32_t first_addr = SPECT_INSTR_MEM_BASE;
    if (options[FIRST_ADDR]) {
        std::stringstream ss;
        ss << std::hex << options[FIRST_ADDR].arg;
        ss >> first_addr;
    }

    // All remaining arguments are input source files
    try {
        comp = new spect::Compiler(first_addr);
        for (int i = 0; i < parse.nonOptionsCount(); ++i) {
            std::string path = parse.nonOption(i);
                comp->Compile(path);
        }
    } catch(std::runtime_error &err) {
        std::cout << err.what() << std::endl;
        ret_code = 1;
        goto cleanup;
    }

    if (options[SIM_HEX]) {
        std::cout << "Assembling program to: " << options[SIM_HEX].arg << std::endl;
        try {
            spect::HexFileType hex_type = spect::HexFileType::ISS_WORD;
            if (options[HEX_FORMAT]) {
                if (*options[HEX_FORMAT].arg == '1')
                    hex_type = spect::HexFileType::VERILOG_RAW_WORD;
                else if (*options[HEX_FORMAT].arg == '2')
                    hex_type = spect::HexFileType::VERILOG_ADDR_WORD;
            }

            comp->program_->Assemble(std::string(options[SIM_HEX].arg), hex_type);
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    comp->CompileFinish();

    comp->program_->Dump(comp->std_out_);

cleanup:
    delete comp;
    exit(ret_code);
}