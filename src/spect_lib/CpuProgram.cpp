/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <iostream>
#include <fstream>

#include "Instruction.h"
#include "CpuProgram.h"
#include "Symbol.h"
#include "Compiler.h"

spect::CpuProgram::CpuProgram(size_t expected_size)
{
    code_.reserve(expected_size);
}

void spect::CpuProgram::AppendInstruction(spect::Instruction *instr)
{
    code_.push_back(instr);
}

void spect::CpuProgram::Assemble(uint32_t *mem)
{
    for (auto const &instr : code_) {

        // If relocation entry is unresolved, then it is returned
        Symbol *s_unknown = instr->Relocate();
        if (s_unknown != nullptr) {
            char buf[128];
            std::sprintf(buf, "Unable to find definition of symbol: '%s'.",
                              s_unknown->identifier_.c_str());
            compiler_->ErrorAt(buf, s_unknown->f_, s_unknown->line_nr_);
        }
        *mem = instr->Assemble();
        mem++;
    }
}

void spect::CpuProgram::Assemble(std::string path, spect::HexFileType hex_type)
{
    uint32_t *mem = new uint32_t[code_.size()];
    Assemble(mem);

    std::ofstream ofs;
    ofs.open(path);
    ofs << std::hex;
    ofs << std::setfill('0');
    for (size_t i = 0; i < code_.size(); i++) {
        if (hex_type == HexFileType::VERILOG_ADDR_WORD ||
            hex_type == HexFileType::ISS_WORD) {
            ofs << "@" << std::setw(4);
            int addr;

            if (hex_type == HexFileType::VERILOG_ADDR_WORD)
                addr = first_addr_ - SPECT_INSTR_MEM_BASE + i;
            else
                addr = first_addr_ + (4 * i);

            ofs << addr << " ";
        }
        ofs << std::setw(8) << mem[i] << std::endl;
    }
    ofs.close();

    delete mem;
}


void spect::CpuProgram::Dump(std::ostream &os)
{
    using namespace spect;

    for (auto const &instr : code_) {
        instr->Dump(os);
        os <<  std::endl;
    }
}