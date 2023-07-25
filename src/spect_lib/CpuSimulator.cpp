/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <regex>
#include <fstream>
#include <iostream>

#include "spect.h"
#include "CpuModel.h"
#include "Compiler.h"
#include "CpuSimulator.h"
#include "HexHandler.h"
#include "KeyMemory.h"

spect::CpuSimulator::CpuSimulator()
{
    model_ = new spect::CpuModel(SPECT_INSTR_MEM_AHB_W, SPECT_INSTR_MEM_AHB_R);
    model_->verbosity_ = VERBOSITY_HIGH;
    model_->simulator_ = this;
    compiler_ = new spect::Compiler(SPECT_INSTR_MEM_BASE);
    key_memory_ = new spect::KeyMemory();

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

bool spect::CpuSimulator::CheckRunning()
{
    if (!program_running_) {
        std::cout << "Program is not running. Start the program with 'start',\n";
        std::cout << "or run it till first breakpoint by 'run' command.\n";
        return false;
    }
    return true;
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
    std::cout << std::dec << "  " << "E: " << model_->GetCpuFlag(CpuFlagType::ERROR) << "\n";
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


///////////////////////////////////////////////////////////////////////////////////////////////////
// Command functions
///////////////////////////////////////////////////////////////////////////////////////////////////

void spect::CpuSimulator::CmdInfo(A_UNUSED std::ostream &out, std::string arg1)
{
    if (arg1 == "breakpoints")
        PrintBreakpoints();
    else if (arg1 == "registers")
        PrintGprRegisters();
    else if (arg1 == "flags")
        PrintFlags();
    else if (arg1 == "rar")
        PrintRar();
    else if (arg1 == "pc")
        PrintPc();
    else if (arg1 == "symbols")
        PrintSymbols();
    else
        std::cout << "Unknown object: " << arg1 << "\n";
}

void spect::CpuSimulator::CmdBreak(A_UNUSED std::ostream &out, std::string arg1)
{
    std::stringstream ss;

    // offset relative to current PC
    if (arg1.size() > 0 && (arg1[0] == '-' || arg1[0] == '+')) {
        uint16_t offset;
        ss << arg1.substr(1, arg1.size() - 1);
        ss >> offset;

        if (offset % 4 != 0) {
            std::cout << "Can't place breakpoint to 0x4 non-aligned address!\n";
            return;
        }

        uint16_t bp_address = model_->GetPc() + offset;
        if (arg1[0] == '-')
            bp_address = model_->GetPc() + offset;
        AddBreakpoint(bp_address);

    // Absolute address
    } else if (std::regex_match(arg1, std::regex("^" VAL_REGEX) )) {
        ss << arg1;
        uint16_t bp_address;
        ss >> bp_address;
        AddBreakpoint(bp_address);

    // Symbol
    } else {
        AddBreakpoint(arg1);
    }
}

void spect::CpuSimulator::CmdRun(A_UNUSED std::ostream &out)
{
    if (CheckFinished())
        return;

    if (!program_running_) {
        model_->Reset();
        if (model_context_ != "")
            model_->LoadContext(model_context_);
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

void spect::CpuSimulator::CmdDelete(A_UNUSED std::ostream &out, std::string arg1, bool all)
{
    std::stringstream ss;

    if (all) {
        if (arg1.size() == 0)
            std::cout << "No breakpoint defined!:\n";
        else
            for (const uint32_t &bp : arg1)
                RemoveBreakPoint(bp);
    } else {
        if (std::regex_match(arg1, std::regex("^" VAL_REGEX) )) {
            ss << arg1;
            uint16_t bp_address;
            ss >> bp_address;
            RemoveBreakPoint(bp_address);
        } else {
            RemoveBreakPoint(arg1);
        }
    }
}

void spect::CpuSimulator::CmdJump(A_UNUSED std::ostream &out, std::string arg1)
{
    std::stringstream ss;

    if (std::regex_match(arg1, std::regex("^" VAL_REGEX) )) {
        ss << arg1;
        uint16_t bp_address;
        ss >> bp_address;
        model_->SetPc(bp_address);
    } else {
        Symbol *s = compiler_->symbols_->GetSymbol(arg1);
        if (s)
            model_->SetPc(s->val_);
        else
            std::cout << "Symbol '" << arg1 << "' undefined!\n";
    }
}

void spect::CpuSimulator::CmdGet(A_UNUSED std::ostream &out, std::string arg1)
{
    if (std::regex_match(arg1, std::regex("^" OP_REGEX))) {
        std::string i_str = arg1.substr(1, arg1.size() - 1);
        out << tohexs(model_->GetGpr(stoint(i_str))) << "\n";

    } else if (std::regex_match(arg1, std::regex("^mem\\[" NUM_REGEX "\\](\\+" NUM_REGEX ")?"))) {
        // Check if multiple addresses should be shown
        int n_pos = 1;
        if (arg1.find('+') != std::string::npos)
            n_pos = stoint(arg1.substr(arg1.find('+') + 1, arg1.size() - 1));

        // Parse out base address
        int b_low = arg1.find("[");
        int b_high = arg1.find("]");
        std::string i_str = arg1.substr(b_low + 1, b_high - b_low - 1);

        // temporarily disable verbosity, to print data in HEX-like format
        int tmp_verbosity = model_->verbosity_;
        model_->verbosity_ = 0;
        uint16_t base_addr = stoint(i_str);
        for (uint16_t addr = base_addr; addr < (base_addr + (4 * n_pos)); addr += 4) {
            out << "@" << tohexs(addr, 4) << ": ";
            out << tohexs(model_->GetMemory(addr), 8) << "\n";
        }
        model_->verbosity_ = tmp_verbosity;
    } else if (std::regex_match(arg1, std::regex("^keymem\\[" NUM_REGEX "\\]\\[" NUM_REGEX "\\]\\[" NUM_REGEX "\\](\\+" NUM_REGEX ")?"))) {
        // Check if multiple offsets should be shown
        int n_pos = 0;
        if (arg1.find('+') != std::string::npos)
            n_pos = stoint(arg1.substr(arg1.find('+') + 1, arg1.size() - 1));

        // Parse out type, slot and offset
        int type_b_low = arg1.find("[");
        int type_b_high = arg1.find("]");
        int slot_b_low = arg1.find("[", type_b_high+1);
        int slot_b_high = arg1.find("]", type_b_high+1);
        int offset_b_low = arg1.find("[", slot_b_high+1);
        int offset_b_high = arg1.find("]", slot_b_high+1);
        std::string type_str = arg1.substr(type_b_low + 1, type_b_high - type_b_low - 1);
        std::string slot_str = arg1.substr(slot_b_low + 1, slot_b_high - slot_b_low - 1);
        std::string offset_str = arg1.substr(offset_b_low + 1, offset_b_high - offset_b_low - 1);

        // print data in HEX-like format
        uint8_t type   = stoint(type_str);
        uint8_t slot   = stoint(slot_str);
        uint8_t offset = stoint(offset_str);
        for (uint8_t i = 0; i <= n_pos; i++) {
            out << "[" << type_str << "][" << slot_str << "][" << std::to_string(offset+i) << "] = ";
            out << tohexs(key_memory_->Get(type, slot, offset+i), 8) << "\n";
        }
    } else {
        std::cout << "Invalid object: " << arg1 << "\n";
    }
}

void spect::CpuSimulator::CmdSet(A_UNUSED std::ostream &out, std::string arg1, std::string arg2)
{
    std::stringstream ss;

    if (std::regex_match(arg1, std::regex("^" OP_REGEX))) {
        std::string i_str = arg1.substr(1, arg1.size() - 1);
        model_->SetGpr(stoint(i_str), uint256_t(arg2.c_str()));

    } else if (std::regex_match(arg1, std::regex("^mem\\[" NUM_REGEX "\\]"))) {
        int from = arg1.find("[");
        int to = arg1.find("]");
        std::string i_str = arg1.substr(from + 1, to - from - 1);
        model_->SetMemory(stoint(i_str), stoint(arg2));
    } else if (std::regex_match(arg1, std::regex("^keymem\\[" NUM_REGEX "\\]\\[" NUM_REGEX "\\]\\[" NUM_REGEX "\\]"))) {
        // Parse out type, slot and offset
        int type_b_low = arg1.find("[");
        int type_b_high = arg1.find("]");
        int slot_b_low = arg1.find("[", type_b_high+1);
        int slot_b_high = arg1.find("]", type_b_high+1);
        int offset_b_low = arg1.find("[", slot_b_high+1);
        int offset_b_high = arg1.find("]", slot_b_high+1);
        std::string type_str = arg1.substr(type_b_low + 1, type_b_high - type_b_low - 1);
        std::string slot_str = arg1.substr(slot_b_low + 1, slot_b_high - slot_b_low - 1);
        std::string offset_str = arg1.substr(offset_b_low + 1, offset_b_high - offset_b_low - 1);

        key_memory_->Set(stoint(type_str), stoint(slot_str), stoint(offset_str), stoint(arg2));
    }
}

void spect::CpuSimulator::CmdLoad(A_UNUSED std::ostream &out, std::string arg1, uint32_t offset)
{
    uint32_t *mem = model_->GetMemoryPtr();
    std::cout << "Loading " << arg1 << " to SPECT memory!\n";
    HexHandler::LoadHexFile(arg1, mem, offset);
}

void spect::CpuSimulator::CmdDump(A_UNUSED std::ostream &out, std::string arg1, uint32_t address,
                                  uint32_t size)
{
    uint32_t *mem = model_->GetMemoryPtr();
    std::cout << "Dumping memory:\n";
    std::cout << std::hex;
    std::cout << "   From address:    0x" << address << "\n";
    std::cout << "   To address:      0x" << (address + size) << "\n";
    std::cout << "   Number of bytes:   " << std::dec << size << "\n";
    std::cout << "   Number of words:   " << std::dec << (size >> 2) << "\n";

    HexHandler::DumpHexFile(arg1, HexFileType::ISS_WORD, mem, address, size);
}

void spect::CpuSimulator::CmdStep(A_UNUSED std::ostream &out, int n)
{
    if (CheckFinished())
        return;
    if (!CheckRunning())
        return;
    model_->Step(n);
}

void spect::CpuSimulator::CmdStart(A_UNUSED std::ostream &out)
{
    model_->Reset();
    if (model_context_ != "")
        model_->LoadContext(model_context_);
    model_->Start();
    program_running_ = true;
}

void spect::CpuSimulator::BuildCliCommands(std::unique_ptr<cli::Menu> &menu)
{
    menu->Insert("info", [&](std::ostream &out, std::string arg1){
                    CmdInfo(out, arg1);
                 },
                "Print information about:\n"
                "           info breakpoints     - Breakpoints\n"
                "           info registers       - CPU Registers\n"
                "           info flags           - CPU Flags\n"
                "           info pc              - Program counter\n"
                "           info rar             - Return address register stack\n"
                "           info symbols         - Symbol table\n");

    menu->Insert("break", [&](std::ostream &out, std::string arg1){
                    CmdBreak(out, arg1);
                 },
                "Add breakpoint:\n"
                "            break <label>       - Put breakpoint at position of <label>\n"
                "            break address       - Put breakpoint at absolute address\n"
                "            break -+number      - Put breakpoint +- n instructions from current PC.\n");

     menu->Insert("delete", [&](std::ostream &out){
                    CmdDelete(out, std::string(""), true);
                 },
                "Delete all breakpoints.\n");

    menu->Insert("delete", [&](std::ostream &out, std::string arg1){
                    CmdDelete(out, arg1, false);
                 },
                "Delete breakpoints:\n"
                "            delete <label>      - Delete breakpoint at <label>.\n"
                "            delete address      - Delete breakpoint at address.\n");

    menu->Insert("jump", [&](std::ostream &out, std::string arg1){
                    CmdJump(out, arg1);
                 },
                "Delete breakpoints:\n"
                "            jump <label>        - Set PC to position of <label>.\n"
                "            jump address        - Set PC to address.\n");

    menu->Insert("get", [&](std::ostream &out, std::string arg1){
                    CmdGet(out, arg1);
                },
                "Get object value:\n"
                "            get RX                           - Get GPR register X value.\n"
                "            get mem[address]                 - Get value at memory address.\n"
                "            get mem[address]+X               - Get value at memory address + X next addresses\n"
                "            get keymem[type][slot][offset]   - Get value of key memory for given type, slot and offset.\n"
                "            get keymem[type][slot][offset]+X - Get value of key memory for given type, slot and offset + X next offsets\n");

    menu->Insert("set", [&](std::ostream &out, std::string arg1, std::string arg2){
                    CmdSet(out, arg1, arg2);
                },
                "Set object to a value:\n"
                "            set RX <value>                         - Set GPR register X to <value>.\n"
                "            set mem[address] <value>               - Set memory address to <value>\n"
                "            set keymem[type][slot][offset] <value> - Set key memory for given type, slot and offset to to <value>\n");

    menu->Insert("load", [&](std::ostream &out, std::string arg1){
                    CmdLoad(out, arg1, 0);
                 },
                "Load file to SPECT memory. Place at address from HEX file.\n"
                "           load <hex-file>               - Load HEX file.");

    menu->Insert("load", [&](std::ostream &out, std::string arg1, std::string arg2){
                    CmdLoad(out, arg1, stoint(arg2));
                 },
                "Load file to SPECT memory. Place at address from HEX file + offset:\n"
                "           load <hex-file> <offset>      - Load HEX file.");

    menu->Insert("dump", [&](std::ostream &out, std::string arg1, std::string arg2, std::string arg3){
                    CmdDump(out, arg1, stoint(arg2), stoint(arg3));
                 },
                "Dump SPECT memory to a file:\n"
                "           dump <hex-file> <start_addr> <size>      - Dump memory to HEX file.");

    menu->Insert("run", [&](std::ostream &out){
                    CmdRun(out);
                 },
                "Run program.");

    menu->Insert("r", [&](std::ostream &out){
                    CmdRun(out);
                },
                 "Run program.");

    menu->Insert("s", [&](std::ostream &out){
                    CmdStep(out, 1);
                },
                 "Step single instruction.");

    menu->Insert("step", [&](std::ostream &out, int n){
                    CmdStep(out, n);
                },
                 "Step N instructions.");

    menu->Insert("start", [&](std::ostream &out){
                    CmdStart(out);
                },
                 "Start execution of program (Reset SPECT, Load context and Load Start PC)");

}

void spect::CpuSimulator::Start(bool batch_mode)
{
    if (batch_mode) {
        model_->Reset();
        if (model_context_ != "")
            model_->LoadContext(model_context_);
        model_->Start();
        model_->Step(0);
    } else {
        std::cout << std::string(80, '*') << "\n";
        std::cout << "Launching SPECT Instruction Set Simulator" << "\n";
        std::cout << std::string(80, '*') << "\n";

        cli::LoopScheduler scheduler;
        cli::CliLocalTerminalSession session(*cli_, scheduler, std::cout);

        session.ExitAction(
            [&scheduler](A_UNUSED auto& out)
            {
                scheduler.Stop();
            }
        );

        if (cmd_file_ != "")
            ExecCmdFile(session);

        scheduler.Run();
    }
}

void spect::CpuSimulator::ExecCmdFile(cli::CliLocalTerminalSession &session)
{
    std::ifstream ifs;
    ifs.open(cmd_file_);
    std::string line;
    if (ifs.is_open()) {
        std::cout << "Loading command file: " << cmd_file_ << "\n";
        while (std::getline(ifs, line)) {
            std::cout << "Executing command: " << line << "\n";
            session.Feed(line);
        }
        session.Feed("\n");
    } else {
        std::cout << "Failed to open command file: " << cmd_file_ << "\n";
    }
}

