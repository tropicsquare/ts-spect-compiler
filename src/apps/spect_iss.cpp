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

enum  optionIndex {
    UNKNOWN,
    HELP,
    PROGRAM,
    FIRST_ADDR,
    START_PC,
    CMD_FILE,
    INSTRUCTION_MEM_HEX,
    CONST_ROM_HEX,
    DATA_RAM_IN_HEX,
    DATA_RAM_OUT_HEX
};

const option::Descriptor usage[] =
{
    {UNKNOWN,               0,  ""  ,    ""                     ,option::Arg::None,         "USAGE: spect_compiler [options]\n\n" "Options:" },
    {HELP,                  0,  "h" ,    "help"                 ,option::Arg::None,         "  --help                       Print usage and exit." },
    {PROGRAM,               0,  ""  ,    "program"              ,option::Arg::Optional,     "  --program=<s-file>           Program (unassembled) to be compiled and loaded to Instruction memory.\n"},
    {FIRST_ADDR,            0,  ""  ,    "first-address"        ,option::Arg::Optional,     "  --first-address=<addr>       Address to place first instruction from first compiled file. Use this "
                                                                                                                           "option only when loading program via '--program' switch. Option is ignored"
                                                                                                                           "when loading program from HEX file.\n" },
    {START_PC,              0,  ""  ,    "start-pc"             ,option::Arg::Optional,     "  --start-pc=<addr>            Address of first instruction to be executed by model.\n"},
    {CMD_FILE,              0,  ""  ,    "cmd-file"             ,option::Arg::Optional,     "  --cmd-file=<file>            Command file to be executed by Instruction simulator.\n"},
    {INSTRUCTION_MEM_HEX,   0,  ""  ,    "instruction-mem"      ,option::Arg::Optional,     "  --instruction-mem=<hex-file> Program (assembled) to be loaded to instruction memory.\n"},
    {CONST_ROM_HEX,         0,  ""  ,    "const-rom"            ,option::Arg::Optional,     "  --const-rom=<hex-file>       Content of Constant ROM to be loaded.\n"},
    {DATA_RAM_IN_HEX,       0,  ""  ,    "data-ram-in"          ,option::Arg::Optional,     "  --data-ram-in=<hex-file>     Content of Data RAM IN to be loaded.\n"},
    {DATA_RAM_OUT_HEX,      0,  ""  ,    "data-ram-out"         ,option::Arg::Optional,     "  --data-ram-out=<hex-file>    Path where content of Data RAM out will be dumped.\n"},

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

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Initialize objects
    ///////////////////////////////////////////////////////////////////////////////////////////////
    model = new spect::CpuModel(false);
    comp = new spect::Compiler(SPECT_INSTR_MEM_BASE);
    uint32_t start_pc = SPECT_INSTR_MEM_BASE;
    std::cout << "1" << std::endl;

    if (options[PROGRAM] && options[FIRST_ADDR]) {
        std::stringstream ss;
        ss << std::hex << options[FIRST_ADDR].arg;
        ss >> comp->first_addr_ ;
    } else {
        comp->Warning("'--first-address' undefined, and program loaded via '--program'."
                      "First instruction will be placed at start of Instruction memory.");
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Compile and/or preload memories
    ///////////////////////////////////////////////////////////////////////////////////////////////
    uint32_t *m_mem = model->GetMemoryPtr();

    if (options[INSTRUCTION_MEM_HEX]) {
        if (options[PROGRAM]) {
            comp->Error("Instruction model invoked with both '--program' and '--instruction-mem'."
                        "Use only one source of program (either HEX or .s file)");
        }
        try {
            std::string path = std::string(options[INSTRUCTION_MEM_HEX].arg);
            spect::HexHandler::LoadHexFile(path, m_mem, SPECT_INSTR_MEM_BASE);
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    if (options[PROGRAM]) {
        std::stringstream ss;
        ss << options[PROGRAM].arg;
        try {
            comp->Compile(ss.str());
            uint32_t *p_start = m_mem + (comp->program_->first_addr_ >> 2);
            comp->program_->Assemble(p_start);
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    if (options[CONST_ROM_HEX]) {
        try {
            std::string path = std::string(options[CONST_ROM_HEX].arg);
            spect::HexHandler::LoadHexFile(path, m_mem, SPECT_CONST_ROM_BASE);
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    if (options[DATA_RAM_IN_HEX]) {
        try {
            std::string path = std::string(options[DATA_RAM_IN_HEX].arg);
            spect::HexHandler::LoadHexFile(path, m_mem, SPECT_DATA_RAM_IN_BASE);
        } catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            ret_code = 1;
            goto cleanup;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Configure start address
    ///////////////////////////////////////////////////////////////////////////////////////////////
    if (options[PROGRAM]) {
        if (comp->symbols_->IsDefined(START_SYMBOL))
            start_pc = comp->symbols_->GetSymbol(START_SYMBOL)->val_;
        if (options[START_PC])
            comp->Warning("'_start' symbol defined in .s file and '--start-pc' switch is used."
                          " Using '--start-pc' as address of first executed instruction.");
    }
    if (options[START_PC]) {
        std::stringstream ss;
        ss << std::hex << options[START_PC].arg;
        ss >> start_pc;
    }
    model->SetStartPc(start_pc);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Reset the model
    ///////////////////////////////////////////////////////////////////////////////////////////////
    model->verbosity_ = VERBOSITY_HIGH;
    model->Reset();
    model->Start();

    model->Step(0);

cleanup:
    delete model;
    delete comp;
    return ret_code;
}