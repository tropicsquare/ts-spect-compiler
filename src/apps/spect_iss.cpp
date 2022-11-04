/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include "OptionParser.h"

#include "Compiler.h"
#include "CpuModel.h"
#include "CpuProgram.h"
#include "HexHandler.h"

enum  optionIndex { UNKNOWN, HELP, PROGRAM, INSTRUCTIION_MEM_HEX, CONST_ROM_HEX, DATA_RAM_IN_HEX, DATA_RAM_OUT_HEX};

const option::Descriptor usage[] =
{
    {UNKNOWN,               0,  ""  ,    ""                     ,option::Arg::None,         "USAGE: spect_compiler [options]\n\n" "Options:" },
    {HELP,                  0,  "h" ,    "help"                 ,option::Arg::None,         "  --help                       Print usage and exit." },
    {PROGRAM,               0,  ""  ,    "program"              ,option::Arg::None,         "  --program=<s-file>           Program (unassembled) to be compiled and loade to instruction memory."},
    {INSTRUCTIION_MEM_HEX,  0,  ""  ,    "instruction-mem"      ,option::Arg::Optional,     "  --instruction-mem=<hex-file> Program (assembled) to be loaded to instruction memory."},
    {CONST_ROM_HEX,         0,  ""  ,    "const-rom"            ,option::Arg::Optional,     "  --const-rom=<hex-file>       Content of Constant ROM to be loaded."},
    {DATA_RAM_IN_HEX,       0,  ""  ,    "data-ram-in"          ,option::Arg::Optional,     "  --data-ram-in=<hex-file>     Content of Data RAM IN to be loaded."},
    {DATA_RAM_OUT_HEX,      0,  ""  ,    "data-ram-out"         ,option::Arg::Optional,     "  --data-ram-out=<hex-file>    Path where content of Data RAM out will be dumped."},

    {0,0,0,0,0,0}
};

spect::Compiler *comp = nullptr;
spect::CpuModel *model = nullptr;

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

    model = new spect::CpuModel(false);

    if (options[INSTRUCTIION_MEM_HEX]) {
        try {
            std::string path = std::string(options[INSTRUCTIION_MEM_HEX].arg);
            spect::HexHandler::LoadHexFile(path, model->GetMemoryPtr());
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    // TODO: Load Data RAM IN and const ROM!
    model->SetStartPc(0x8000);
    model->Start();
    model->Step(0);

cleanup:
    delete model;
    return ret_code;
}