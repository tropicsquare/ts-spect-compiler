/******************************************************************************
*
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
*
* @todo: License
*
* @author Ondrej Ille, <ondrej.ille@tropicsquare.com>
* @date 19.9.2022
*
*****************************************************************************/

#include <ostream>
#include <cassert>

#include "Instruction.h"
#include "InstructionR.h"
#include "InstructionM.h"
#include "InstructionJ.h"
#include "InstructionI.h"


spect::Instruction::Instruction(std::string mnemonic, InstructionType itype, uint32_t opcode,
                                uint32_t func, int op_mask, bool r31_dep, bool c_time) :
    itype_(itype),
    opcode_(opcode),
    func_(func),
    mnemonic_(mnemonic),
    op_mask_(op_mask),
    r31_dep_(r31_dep),
    c_time_(c_time)
{}

spect::Instruction::~Instruction()
{}

spect::Instruction* spect::Instruction::DisAssemble(uint32_t wrd)
{
    using namespace spect;
    uint32_t itype = (wrd >> IENC_TYPE_OFFSET) & IENC_TYPE_MASK;

    switch (itype) {
    case TO_INT(InstructionType::R):
        return InstructionR::DisAssemble(wrd);
    case TO_INT(InstructionType::I):
        return InstructionI::DisAssemble(wrd);
    case TO_INT(InstructionType::M):
        return InstructionM::DisAssemble(wrd);
    default:
        return InstructionJ::DisAssemble(wrd);
    }
}

void spect::Instruction::SampleInputs(dpi_instruction_t *dpi_instr, CpuModel *model)
{
    assert(dpi_instr != nullptr);
    assert(model != nullptr);

    // First clear all previous stuff so that we don't get overlapping instructions
    memset(dpi_instr, 0xFF, sizeof(dpi_instruction_t));

    dpi_instr->i_type = TO_INT(itype_);
    dpi_instr->opcode = opcode_;
    dpi_instr->func = func_;

    switch (itype_) {
    case InstructionType::R:
        return static_cast<spect::InstructionR*>(this)->SampleInputs(dpi_instr, model);
    case InstructionType::I:
        return static_cast<spect::InstructionI*>(this)->SampleInputs(dpi_instr, model);
    case InstructionType::M:
        return static_cast<spect::InstructionM*>(this)->SampleInputs(dpi_instr, model);
    default:
        return static_cast<spect::InstructionJ*>(this)->SampleInputs(dpi_instr, model);
    }
}

void spect::Instruction::SampleOutputs(dpi_instruction_t *dpi_instr, CpuModel *model)
{
    switch (itype_) {
    case InstructionType::R:
        return static_cast<spect::InstructionR*>(this)->SampleOutputs(dpi_instr, model);
    case InstructionType::I:
        return static_cast<spect::InstructionI*>(this)->SampleOutputs(dpi_instr, model);
    case InstructionType::M:
        return static_cast<spect::InstructionM*>(this)->SampleOutputs(dpi_instr, model);
    default:
        return static_cast<spect::InstructionJ*>(this)->SampleOutputs(dpi_instr, model);
    }
}

void spect::Instruction::Dump(std::ostream& os)
{
    os.width(10);
    os << std::left << mnemonic_;

    switch (itype_) {
    case spect::InstructionType::R:
        static_cast<spect::InstructionR*>(this)->Dump(os);
        break;

    case spect::InstructionType::I:
        static_cast<spect::InstructionI*>(this)->Dump(os);
        break;

    case spect::InstructionType::M:
        static_cast<spect::InstructionM*>(this)->Dump(os);
        break;

    default:
        static_cast<spect::InstructionJ*>(this)->Dump(os);
        break;
    }
}

std::string spect::Instruction::Dump()
{
    std::stringstream ss;
    Dump(ss);
    return ss.str();
}

