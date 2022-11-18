/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <regex>

#include "spect.h"
#include "CpuModel.h"
#include "Compiler.h"
#include "CpuSimulator.h"

spect::CpuSimulator::CpuSimulator()
{
    model_ = new spect::CpuModel(false);
    model_->verbosity_ = VERBOSITY_HIGH;
    compiler_ = new spect::Compiler(SPECT_INSTR_MEM_BASE);

    auto menu = std::make_unique<cli::Menu>("spect_iss");
    BuildCliCommands(menu);

    cli_ = new cli::Cli(std::move(menu));
    cli_->ExitAction( [](auto& out){
        out << std::string(80, '*') << "\n";
        out << "Exiting SPECT simulator... \n";
        out << std::string(80, '*') << "\n";
        }
    );
}

spect::CpuSimulator::~CpuSimulator()
{
    delete model_;
    delete compiler_;
    delete cli_;
}

bool spect::CpuSimulator::CheckFinished()
{
    if (model_->IsFinished()) {
        std::cout << "Program execution has previously finished!\n";
        std::cout << "Execute 'start' to restart the program execution\n";
        return true;
    }
    return false;
}

void spect::CpuSimulator::CmdRun()
{
    if (CheckFinished())
        return;

    if (!program_running_) {
        model_->Reset();
        model_->Start();
        program_running_ = true;
    }
    do {
        model_->StepSingle(0);
        auto pc = model_->GetPc();
        if (IsBreakpointAt(pc)) {
            std::cout << "Hit Breakpoint:\n";
            PrintBreakpoint(pc);
            break;
        }
    } while (!model_->IsFinished());

    if (model_->IsFinished())
        std::cout << "Program execution finished!\n";
}

bool spect::CpuSimulator::AddBreakpoint(uint16_t address)
{
    if (IsBreakpointAt(address)) {
        std::cout << "Breakpoint already exists at: 0x" << std::hex << address << std::endl;
        return false;
    }
    std::cout << "Adding breakpoint at:\n";
    std::cout << "    address: 0x" << std::hex << address << std::endl;
    breakpoints_.push_back(address);
    return true;
}

bool spect::CpuSimulator::AddBreakpoint(std::string label)
{
    Symbol *s = compiler_->symbols_->GetSymbol(label);
    if (s == nullptr) {
        std::cout << "Label '" << label << "' does not exist. Can't add breakpoint!\n";
        return false;
    }
    if (s->type_ != SymbolType::LABEL) {
        std::cout << "Symbol '" << label << "' exists, but it is not function label."
                     " Can't add breakpoint to a symbol which is not label!\n";
        return false;
    }

    uint16_t address = s->val_;
    if (IsBreakpointAt(address)) {
        std::cout << "Breakpoint already exists at: 0x" << std::hex << address << std::endl;
        return false;
    }

    std::cout << "Adding breakpoint at:\n";
    std::cout << "    " << label << ", address: 0x" << address << "\n";
    breakpoints_.push_back(address);

    return true;
}

bool spect::CpuSimulator::RemoveBreakPoint(uint32_t address)
{
    for (auto it = breakpoints_.begin(); it != breakpoints_.end(); it++)
        if (*it == address) {
            std::cout << "Removing breakpoint at:\n";
            std::cout << "    0x" << address << "\n";
            breakpoints_.erase(it);
            return true;
        }
    std::cout << "No breakpoint exists at:\n";
    std::cout << "0x" << address << "\n";
    return false;
}

bool spect::CpuSimulator::RemoveBreakPoint(std::string label)
{
    Symbol *s = compiler_->symbols_->GetSymbol(label);
    if (s == nullptr) {
        std::cout << "Label '" << label << "' does not exist. Can't remove breakpoint!\n";
        return false;
    }

    for (auto it = breakpoints_.begin(); it != breakpoints_.end(); it++)
        if (*it == s->val_) {
            std::cout << "Removing breakpoint at:\n";
            std::cout << "    " << label << ", address: 0x" << s->val_ << "\n";
            breakpoints_.erase(it);
            return true;
        }
    std::cout << "No breakpoint exists at:\n";
    std::cout << "    " << label << ", address: 0x" << s->val_ << "\n";

    return false;
}


bool spect::CpuSimulator::IsBreakpointAt(uint32_t address)
{
    for (const uint32_t bp : breakpoints_)
        if (bp == address)
            return true;
    return false;
}

void spect::CpuSimulator::PrintBreakpoint(uint32_t breakpoint)
{
    std::cout << "  " << std::hex << "0x" << breakpoint;
    Symbol *s = compiler_->symbols_->GetSymbol(breakpoint, SymbolType::LABEL);
    if (s)
        std::cout << "  " << s->identifier_;
    std::cout << "\n";
}

void spect::CpuSimulator::PrintBreakpoints()
{
    if (breakpoints_.size() > 0) {
        std::cout << "Breakpoints:\n";
        for (const uint32_t bp : breakpoints_)
            PrintBreakpoint(bp);
    } else {
        std::cout << "No breakpoints defined.\n";
    }
}

void spect::CpuSimulator::PrintGprRegisters()
{
    std::cout << "GPR Registers:\n";
    for (int i = 0; i < 32; i++) {
        std::cout << std::dec << "  " << ((CpuGpr)i) << ":";
        std::cout << std::hex << " 0x" << model_->GetGpr(i) << std::endl;
    }
}

void spect::CpuSimulator::PrintFlags()
{
    std::cout << "CPU Flags:\n";
    std::cout << std::dec << "  " << "Z: " << model_->GetCpuFlag(CpuFlagType::ZERO) << "\n";
    std::cout << std::dec << "  " << "C: " << model_->GetCpuFlag(CpuFlagType::CARRY) << "\n";
}

void spect::CpuSimulator::PrintPc()
{
    std::cout << "Program counter:\n";
    std::cout << std::hex << "  0x" << model_->GetPc() << "\n";
}

void spect::CpuSimulator::PrintRar()
{
    std::cout << "Return address register stack:\n";
    uint16_t sp = model_->GetRarSp();
    for (uint16_t i = 0; i < SPECT_RAR_DEPTH; i++) {
        std::cout << "  " << std::dec << i << ": ";
        std::cout << std::hex << "0x" << std::setw(4) << model_->GetRarAt(i);
        if (sp == i)
            std::cout << " <--- Stack pointer ---";
        std::cout << "\n";
    }
}

void spect::CpuSimulator::PrintSymbols()
{
    std::cout << "Symbol Table:\n";
    compiler_->symbols_->Print(std::cout);
}

void spect::CpuSimulator::SetObject(std::string object, std::string value)
{
    int index;
    std::stringstream ss;
    if (std::regex_match(object, std::regex("^" OP_REGEX))) {
        std::string i_str = object.substr(1, object.size() - 1);
        ss << i_str;
        ss >> index;
        model_->SetGpr(index, uint256_t(value.c_str()));

    } else if (std::regex_match(object, std::regex("^mem\\[" NUM_REGEX "\\]"))) {
        int from = object.find("[");
        int to = object.find("]");
        std::string i_str = object.substr(from + 1, to - from - 1);
        ss << std::hex << i_str;
        ss >> index;
        printf("I_STR is: %s, Index is: %d\n", i_str.c_str(), index);
        std::stringstream ss_v;
        ss_v << std::hex << value;
        uint32_t i_val;
        ss_v >> i_val;
        model_->SetMemory(index, i_val);
    }
}

void spect::CpuSimulator::BuildCliCommands(std::unique_ptr<cli::Menu> &menu)
{
    menu->Insert("info", [&](std::ostream &out, std::string type){
                    if (type == "breakpoints")
                        PrintBreakpoints();
                    else if (type == "registers")
                        PrintGprRegisters();
                    else if (type == "flags")
                        PrintFlags();
                    else if (type == "rar")
                        PrintRar();
                    else if (type == "pc")
                        PrintPc();
                    else if (type == "symbols")
                        PrintSymbols();
                    else
                        std::cout << "Unknown object: " << type << "\n";
                 },
                "Print information about:\n"
                "           info breakpoints     - Breakpoints\n"
                "           info registers       - CPU Registers\n"
                "           info flags           - CPU Flags\n"
                "           info pc              - Program counter\n"
                "           info rar             - Return address register stack\n"
                "           info symbols         - Symbol table\n");


    menu->Insert("break", [&](std::ostream &out, std::string breakpoint){
                    std::stringstream ss;

                    // offset relative to current PC
                    if (breakpoint.size() > 0 && (breakpoint[0] == '-' || breakpoint[0] == '+')) {
                        uint16_t offset;
                        ss << breakpoint.substr(1, breakpoint.size() - 1);
                        ss >> offset;

                        if (offset % 4 != 0) {
                            std::cout << "Can't place breakpoint to 0x4 non-aligned address!\n";
                            return;
                        }

                        uint16_t bp_address = model_->GetPc() + offset;
                        if (breakpoint[0] == '-')
                            bp_address = model_->GetPc() + offset;
                        AddBreakpoint(bp_address);

                    // Absolute address
                    } else if (std::regex_match(breakpoint, std::regex("^" VAL_REGEX) )) {
                        ss << breakpoint;
                        uint16_t bp_address;
                        ss >> bp_address;
                        AddBreakpoint(bp_address);

                    // Symbol
                    } else {
                        AddBreakpoint(breakpoint);
                    }
                 },
                "Add breakpoint:\n"
                "            break <label>       - Put breakpoint at position of <label>\n"
                "            break address       - Put breakpoint at absolute address\n"
                "            break -+number      - Put breakpoint +- n instructions from current PC.\n");

     menu->Insert("delete", [&](std::ostream &out){
                        breakpoints_.clear();
                 },
                "Delete all breakpoints.\n");

    menu->Insert("delete", [&](std::ostream &out, std::string breakpoint){
                    std::stringstream ss;

                    if (std::regex_match(breakpoint, std::regex("^" VAL_REGEX) )) {
                        ss << breakpoint;
                        uint16_t bp_address;
                        ss >> bp_address;
                        RemoveBreakPoint(bp_address);
                    } else {
                        RemoveBreakPoint(breakpoint);
                    }
                 },
                "Delete breakpoints:\n"
                "            delete <label>      - Delete breakpoint at <label>.\n"
                "            delete address      - Delete breakpoint at address.\n");

    menu->Insert("jump", [&](std::ostream &out, std::string breakpoint){
                    std::stringstream ss;

                    if (std::regex_match(breakpoint, std::regex("^" VAL_REGEX) )) {
                        ss << breakpoint;
                        uint16_t bp_address;
                        ss >> bp_address;
                        model_->SetPc(bp_address);
                    } else {
                        Symbol *s = compiler_->symbols_->GetSymbol(breakpoint);
                        if (s)
                            model_->SetPc(s->val_);
                        else
                            std::cout << "Symbol '" << breakpoint << "' undefined!\n";
                    }
                 },
                "Delete breakpoints:\n"
                "            jump <label>        - Set PC to position of <label>.\n"
                "            jump address        - Set PC to address.\n");

    menu->Insert("get", [&](std::ostream &out, std::string object, std::string value){
                    // TODO: Add support for GET
                },
                "Get object value:\n"
                "            get RX <value>             - Get GPR register value\n"
                "            get mem[address] <value>   - Get value at address.\n");

    menu->Insert("set", [&](std::ostream &out, std::string object, std::string value){
                    SetObject(object, value);
                },
                "Set object to a value:\n"
                "            set RX <value>             - Set GPR register to a value.\n"
                "            set mem[address] <value>   - Set memory address to value\n");

    menu->Insert("run", [&](std::ostream &out){
                    CmdRun();
                 },
                "Run program.");
    menu->Insert("r", [&](std::ostream &out){
                    CmdRun();
                },
                 "Run program.");

    menu->Insert("s", [&](std::ostream &out){
                    if (CheckFinished())
                        return;
                    model_->Step(1);
                },
                 "Step single instruction.");
    menu->Insert("step", [&](std::ostream &out, int n){
                    if (CheckFinished())
                        return;
                    model_->Step(n);
                },
                 "Step N instructions.");

    menu->Insert("start", [&](std::ostream &out){
                    model_->Reset();
                    model_->Start();
                    program_running_ = true;
                },
                 "Start execution of program (Reset SPECT and load Start PC)");

}

void spect::CpuSimulator::Start(bool batch_mode, const char* cmd_file)
{
    if (batch_mode) {
        model_->Reset();
        model_->Start();
        model_->Step(0);
    } else {
        std::cout << std::string(80, '*') << "\n";
        std::cout << "Launching SPECT Instruction Set Simulator" << "\n";
        std::cout << std::string(80, '*') << "\n";

        cli::LoopScheduler scheduler;
        cli::CliLocalTerminalSession session(*cli_, scheduler, std::cout);

        session.ExitAction(
            [&scheduler](auto& out)
            {
                scheduler.Stop();
            }
        );
        scheduler.Run();
    }
}
