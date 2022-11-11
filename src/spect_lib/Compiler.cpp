/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <fstream>
#include <regex>
#include <iostream>
#include <cassert>
#include <exception>

#include "Compiler.h"
#include "InstructionFactory.h"

#include "Instruction.h"
#include "InstructionI.h"
#include "InstructionJ.h"
#include "InstructionM.h"
#include "InstructionR.h"


spect::Compiler::Compiler(uint32_t first_addr) :
    first_addr_(first_addr),
    curr_addr_(first_addr)
{
    if (first_addr < SPECT_INSTR_MEM_BASE ||
       (first_addr_ > (SPECT_INSTR_MEM_BASE + SPECT_INSTR_MEM_SIZE)) ) {
        char buf[256];
        std::sprintf(buf, "First instruction placed at address 0x%04x which is not in Instruction "
                          "Memory of SPECT. Place first instruction between: 0x%04x - 0x%04x",
                          first_addr, SPECT_INSTR_MEM_BASE,
                          SPECT_INSTR_MEM_BASE + SPECT_INSTR_MEM_SIZE - 4);
        Error(buf);
    }

    symbols_ = new spect::SymbolTable();
    program_ = new spect::CpuProgram(2048);
    program_->first_addr_ = first_addr;
    program_->compiler_ = this;
    print_fnc = &(printf);
}

spect::Compiler::~Compiler()
{
    for (const auto &f : files_)
        delete f.second;
    delete symbols_;
    delete program_;
}

uint32_t spect::Compiler::ParseValue(spect::SourceFile *sf, int line_nr, const std::string &val,
                                     Symbol* &s, uint32_t limit)
{
    uint32_t rv = 0;

    // Check that it is not matching operand
    if (std::regex_match(val, std::regex("^" OP_REGEX))) {
        char buf[128];
        std::sprintf(buf, "Found operand: '%s' when expecting value or symbol.", val.c_str());
        ErrorAt(std::string(buf), sf, line_nr);
    }

    // Number
    if (std::regex_match(val, std::regex("^" NUM_REGEX))) {
        if (val.size() < 2) {
            rv = std::stoi (val, nullptr);
        } else if (val[0] == '0') {
            if (val[1] == 'x' || val[1] == 'X') {
                rv = std::stoi (val, nullptr, 16);
            } else if (val[1] == 'b' || val[1] == 'B') {
                rv = std::stoi (val, nullptr, 2);
            }
        } else {
            rv = std::stoi (val, nullptr);
        }

        if (limit > 0 && rv > limit) {
            char buf[128];
            std::sprintf(buf, "Overflow when parsing value: %d , maximal size: %d", rv, limit);
            WarningAt(std::string(buf), sf, line_nr);
        }
        s = nullptr;

    // Symbol
    } else {
        if (!symbols_->IsDefined(val))
            symbols_->AddSymbol(val, SymbolType::UNKNOWN, line_nr);
        s = symbols_->GetSymbol(val);
        rv = 0;
    }

    //std:: cout << "Parsed value: " << rv << std::endl;
    return rv;
}

spect::CpuGpr spect::Compiler::ParseOp(spect::SourceFile *sf, int line_nr, std::string &arg)
{
    if (std::regex_match(arg, std::regex("^" OP_REGEX))) {
        arg.erase(0, 1);
        return static_cast<spect::CpuGpr>(stoi(arg));
    } else {
        char buf[128];
        std::sprintf(buf, "Invalid operand: '%s'. Valid operands: R0, R1 ... R31", arg.c_str());
        ErrorAt(std::string(buf), sf, line_nr);
    }
    return CpuGpr::R0;
}

void spect::Compiler::ParseArgument(spect::SourceFile *sf, int line_nr, spect::Instruction* instr,
                                    std::string &arg, int arg_index)
{
    assert(!(arg_index == 2 && instr->itype_ == InstructionType::J));
    assert(!(arg_index == 3 && ((instr->itype_ == InstructionType::J) ||
                                (instr->itype_ == InstructionType::M))));
    assert(arg_index <= 3);

    spect::Symbol *s;

    spect::InstructionI *instr_i = static_cast<spect::InstructionI*>(instr);
    spect::InstructionJ *instr_j = static_cast<spect::InstructionJ*>(instr);
    spect::InstructionM *instr_m = static_cast<spect::InstructionM*>(instr);
    spect::InstructionR *instr_r = static_cast<spect::InstructionR*>(instr);

    switch (instr->itype_) {
    case InstructionType::R:
        if (arg_index == 1) {
            instr_r->op1_ = ParseOp(sf, line_nr, arg);
        } else if (arg_index == 2) {
            instr_r->op2_ = ParseOp(sf, line_nr, arg);
        } else {
            instr_r->op3_ = ParseOp(sf, line_nr, arg);
        }
        break;

    case InstructionType::I:
        if (arg_index == 1) {
            instr_i->op1_ = ParseOp(sf, line_nr, arg);
        } else if (arg_index == 2) {
            instr_i->op2_ = ParseOp(sf, line_nr, arg);
        } else {
            instr_i->immediate_ = ParseValue(sf, line_nr, arg, s, IENC_IMMEDIATE_MASK);
            instr_i->s_immediate_ = s;
            if (s && s->type_ == SymbolType::LABEL) {
                char buf[128];
                std::sprintf(buf, "Using label: '%s' as immediate oprand. Is this correct?",
                                    s->identifier_.c_str());
                WarningAt(buf, sf, line_nr);
            }
        }
        break;

    case InstructionType::M:
        if (arg_index == 1) {
            instr_m->op1_ = ParseOp(sf, line_nr, arg);
        } else {
            instr_m->addr_ = ParseValue(sf, line_nr, arg, s, IENC_ADDR_MASK);
            instr_m->s_addr_ = s;
        }
        break;

    case InstructionType::J:
        instr_j->new_pc_ = ParseValue(sf, line_nr, arg, s, IENC_NEW_PC_MASK);
        instr_j->s_new_pc_ = s;
        break;
    }
}

spect::Symbol* spect::Compiler::ParseLabel(spect::SourceFile *sf, std::string &line_buf,
                                           int line_nr)
{
    if (std::regex_match(line_buf, std::regex("^" IDENT_REGEX ":.*"))) {
        std::string ident = line_buf.substr(0, line_buf.find(':'));
        line_buf = std::regex_replace(line_buf, std::regex("^" IDENT_REGEX ":"), "");

        if (symbols_->IsDefined(ident)) {
            Symbol *s = symbols_->GetSymbol(ident);

            if (s->resolved_) {
                char buf[128];
                std::sprintf(buf, "Symbol: '%s' previously defined at: %s:%d",
                                ident.c_str(), s->f_->path_.c_str(), s->line_nr_);
                ErrorAt(std::string(buf), sf, line_nr);
            } else {
                symbols_->ResolveSymbol(s, SymbolType::LABEL, curr_addr_);
                return s;
            }
        }

        return symbols_->AddSymbol(ident, SymbolType::LABEL, curr_addr_, line_nr);
    }
    return nullptr;
}

bool spect::Compiler::ParseConstant(spect::SourceFile *sf, std::string &line_buf, int line_nr)
{
    if (std::regex_match(line_buf, std::regex("^" IDENT_REGEX "[ ]+" EQ_KEYWORD "[ ]+" VAL_REGEX))) {
        std::string ident = line_buf.substr(0, line_buf.find(' '));
        std::string val = line_buf.substr(line_buf.find_last_of(' ') + 1, line_buf.size() - 1);
        Symbol *s;
        Symbol *s_dummy;

        if (symbols_->IsDefined(ident)) {
            s = symbols_->GetSymbol(ident);
            if (s->resolved_) {
                char buf[128];
                std::sprintf(buf, "Symbol: '%s' previously defined at: %s:%d",
                                ident.c_str(), s->f_->path_.c_str(), s->line_nr_);
                ErrorAt(std::string(buf), sf, line_nr);
            } else {
                symbols_->ResolveSymbol(s, SymbolType::CONSTANT,
                                        ParseValue(sf, line_nr, val, s_dummy));
            }
        } else {
            symbols_->AddSymbol(ident, SymbolType::CONSTANT,
                                ParseValue(sf, line_nr, val, s_dummy), line_nr);
        }

        // TODO: Check s_dummy ??

        return true;
    }
    return false;
}

bool spect::Compiler::ParseIncludeFile(spect::SourceFile *sf, std::string &line_buf)
{
    if (std::regex_match(line_buf, std::regex("^" INCLUDE_KEYWORD "[ ]+" FILE_REGEX))) {

        // TODO: Make this universal across OS type!
        std::string new_file = sf->path_.substr(0, sf->path_.find_last_of("/")) + "/" +
                                                   line_buf.substr(line_buf.find_last_of(' ') + 1,
                                                line_buf.size() - 1);
        print_fnc("Loading included file: %s\n", new_file.c_str());
        Compile(new_file);
        return true;
    }
    return false;
}

spect::Instruction* spect::Compiler::ParseInstruction(spect::SourceFile *sf, std::string &line_buf,
                                                      int line_nr, spect::Symbol *label)
{
    // Parse mnemonic and find instruction
    int space_pos = line_buf.find(' ');
    std::string mnemonic = line_buf.substr(0, space_pos);
    line_buf.erase(0, space_pos);
    TrimSpaces(line_buf);
    spect::Instruction *gold_instr = InstructionFactory::GetInstruction(mnemonic);

    if (gold_instr == nullptr) {
        char buf[128];
        std::sprintf(buf, "Unknown instruction: %s", mnemonic.c_str());
        ErrorAt(std::string(buf), sf, line_nr);
    }

    spect::Instruction *new_instr = gold_instr->Clone();
    //std::cout << label << "\n";
    new_instr->s_label_ = label;

    // Calculate number of expected arguments (number of bitsin 1 in op_mask)
    int exp_argc = 0;
    for (int i = 2; i >= 0; i--)
        if ((new_instr->op_mask_ >> i) & 0x1)
            exp_argc++;

    // Parse arguments
    int arg_index = 1;
    int skipped_args = 0;

    while (!line_buf.empty()) {

        if (arg_index > exp_argc + skipped_args) {
            char buf[128];
            std::sprintf(buf, "Too many arguments to instruction: %s. Expected only %d arguments.",
                            mnemonic.c_str(), exp_argc);
            ErrorAt(std::string(buf), sf, line_nr);
        }

        // Skip un-implemented arguments for given unstruction
        while (((new_instr->op_mask_ >> (3 - arg_index)) & 0x1) == 0) {
            arg_index++;
            skipped_args++;
        }

        size_t pos = line_buf.find(',');
        std::string arg;
        if (pos != std::string::npos) {
            arg = line_buf.substr(0, pos);
            line_buf.erase(0, pos + 1);
        } else {
            arg = line_buf;
            line_buf = std::string("");
        }

        ParseArgument(sf, line_nr, new_instr, arg, arg_index);

        TrimSpaces(line_buf);
        arg_index++;
    }

    if (arg_index <= exp_argc + skipped_args) {
        char buf[128];
        std::sprintf(buf, "Missing arguments to instruction: %s. "
                          "Expected %d arguments found only %d!",
                          mnemonic.c_str(), exp_argc, arg_index - 1);
        ErrorAt(std::string(buf), sf, line_nr);
    }

    num_instr_++;

    return new_instr;
}


void spect::Compiler::Compile(std::string path)
{
    SourceFile *sf = new spect::SourceFile(path, curr_addr_);
    Symbol *label;
    Symbol *last_label;
    symbols_->curr_file_ = sf;
    files_[path] = sf;

    print_fnc("Compiling: %s\n", path.c_str());

    for (unsigned int line_nr = 1; line_nr <= sf->lines_.size(); line_nr++) {
        std::string line_buf = sf->lines_[line_nr - 1];

        // Remove comments
        line_buf = std::regex_replace(line_buf, std::regex(";.*"), "");
        TrimSpaces(line_buf);

        // Parse Label
        label = ParseLabel(sf, line_buf, line_nr);
        if (label)
            last_label = label;

        if (line_buf.empty())
            continue;

        TrimSpaces(line_buf);

        // Check for definitions of constants
        if (ParseConstant(sf, line_buf, line_nr))
            continue;

        // Check for include of another file
        if (ParseIncludeFile(sf, line_buf))
            continue;

        spect::Instruction *new_instr = ParseInstruction(sf, line_buf, line_nr, last_label);
        last_label = nullptr;

        if (curr_addr_ >= SPECT_INSTR_MEM_BASE + SPECT_INSTR_MEM_SIZE) {
            char buf[256];
            // TODO: Check this calculation!!!
            std::sprintf(buf, "Program does not fit into Instruction memory. "
                              "Address of first instruction: 0x%08x, "
                              "Maximal program size till end of Instruction Memory: "
                              "%d instructions",
                            first_addr_,
                            (SPECT_INSTR_MEM_BASE + SPECT_INSTR_MEM_SIZE - first_addr_) / 4);
            ErrorAt(std::string(buf), sf, line_nr);
        }

        program_->AppendInstruction(new_instr);
        //print_fnc("%s", new_instr->Dump().c_str());
        curr_addr_ += 4;
    }
}

int spect::Compiler::CompileFinish()
{
    int rv = 0;

    print_fnc("%s\n", std::string(80, '*').c_str());
    print_fnc("\033[1;32m");
    print_fnc("Compilation finished OK!\n");
    print_fnc("\033[0m");
    print_fnc("%s\n", std::string(80, '*').c_str());
    print_fnc("First instruction address: 0x%4x\n", first_addr_);
    print_fnc("Last instruction address:  0x%4x\n", curr_addr_ - 4);
    print_fnc("Number of instructions:    %d\n", num_instr_);

    if (num_instr_ == 0) {
        Warning("Program is empty, no instructions found!");
        rv = 1;
    }
    if (!symbols_->IsDefined(START_SYMBOL)) {
        Warning("'" START_SYMBOL "' symbol not found in the program!");
        rv = 1;
    } else
        print_fnc("Program start address:     0x%4x\n", symbols_->GetSymbol(START_SYMBOL)->val_);

    print_fnc("%s\n", std::string(80, '*').c_str());

    return rv;
}

void spect::Compiler::TrimSpaces(std::string &input)
{
    input = std::regex_replace(input, std::regex("^ +"), "");
    input = std::regex_replace(input, std::regex(" +$"), "");
}

void spect::Compiler::ErrorAt(std::string err, const SourceFile *sf, int line_nr)
{
    print_fnc("\033[1m");
    print_fnc("%s:%d:", sf->path_.c_str(), line_nr);
    print_fnc("\033[1m \033[31m Error:\033[0m");
    print_fnc("%s\n", err.c_str());
    if (line_nr - 2 >= 0) {
        print_fnc("%d:%s\n", line_nr - 1, sf->lines_[line_nr - 2].c_str());
    }
    print_fnc("\033[1m");
    print_fnc("%d:", line_nr);
    print_fnc("%s\n", sf->lines_[line_nr - 1].c_str());
    // TODO: Right Adjust the line numbers!
    print_fnc("\033[0m");
    if ((size_t)line_nr < sf->lines_.size())
        print_fnc("%d:%s\n", (line_nr + 1), sf->lines_[line_nr]);

    std::string msg = std::string(80, '*') +
                      std::string("\nCompilation failed\n") +
                      std::string(80, '*');
    throw std::runtime_error(msg.c_str());
}

void spect::Compiler::Error(std::string err)
{
    print_fnc("\033[1m \033[31m Error: \033[0m");
    print_fnc("%s\n", err);

    throw std::runtime_error("Compilation failed");
}

void spect::Compiler::WarningAt(std::string warn, const SourceFile *sf, int line_nr)
{
    print_fnc("\033[1m");
    print_fnc("%s:%d:", sf->path_.c_str(), line_nr);
    print_fnc("\033[33m Warning: \033[0m");
    print_fnc("%s\n", warn.c_str());
    print_fnc("%s\n", sf->lines_[line_nr - 1].c_str());
}
void spect::Compiler::Warning(std::string warn)
{
    print_fnc("\033[1m \033[33m Warning: \033[0m");
    print_fnc("%s\n", warn.c_str());
}
