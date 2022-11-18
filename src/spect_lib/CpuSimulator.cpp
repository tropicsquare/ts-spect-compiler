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
    std::cout << "Adding breakpoint at address: 0x" << std::hex << address << std::endl;
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

    std::cout << "Adding breakpoint to label: " << label;
    std::cout  << ", address: 0x" << address << "\n";
    breakpoints_.push_back(address);

    return true;
}

bool spect::CpuSimulator::RemoveBreakPoint(uint32_t address)
{
    for (auto it = breakpoints_.begin(); it != breakpoints_.end(); it++)
        if (*it == address) {
            std::cout << "Removing breakpoint at: 0x" << address << std::endl;
            breakpoints_.erase(it);
            return true;
        }
    std::cout << "No breakpoint exists at: 0x" << address << std::endl;
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
                "           info breakpoints   - Breakpoints\n"
                "           info registers     - CPU Registers\n"
                "           info flags         - CPU Flags\n"
                "           info pc            - Program counter\n"
                "           info rar           - Return address register stack\n"
                "           info symbols       - Symbol table\n");


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
                "           break symbol-name  - Put breakpoint at position of label\n"
                "           break address      - Put breakpoint at absolute address\n"
                "           break -+number     - Put breakpoint +- n instructions from current PC.\n");


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
                 "Step");
    menu->Insert("step", [&](std::ostream &out, int n){
                    if (CheckFinished())
                        return;
                    model_->Step(n);
                },
                 "Step (number of instructions)");

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
