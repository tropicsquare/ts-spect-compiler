
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

enum  optionIndex { UNKNOWN, HELP, FIRST_ADDR, SIM_HEX};

const option::Descriptor usage[] =
{
    {UNKNOWN,          0,  "" ,    ""               ,option::Arg::None,     "USAGE: spect_compiler [options]\n\n" "Options:" },
    {HELP,             0,  "h" ,    "help"          ,option::Arg::None,     "  --help                  Print usage and exit." },
    {FIRST_ADDR,       0,  ""  ,    "first-address" ,option::Arg::Optional, "  --first-address=<addr>  Address to place first instruction from first compiled file." },
    {SIM_HEX,          0,  ""  ,    "sim-hex"       ,option::Arg::Optional, "  --sim-hex=<file>        HEX file for simulator where code will be assembled."},

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
    comp = new spect::Compiler(first_addr);
    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
        std::string path = parse.nonOption(i);
        try {
            comp->Compile(path);
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    if (options[SIM_HEX]) {
        std::cout << "Assembling program to: " << options[SIM_HEX].arg << std::endl;
        try {
            comp->program_->Assemble(std::string(options[SIM_HEX].arg));
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    comp->CompileFinish();

    comp->program_->Dump(std::cout);

cleanup:
    delete comp;
    exit(ret_code);
}