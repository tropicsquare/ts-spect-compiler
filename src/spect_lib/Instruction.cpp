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

#include "Instruction.h"
#include "InstructionR.h"
#include "InstructionM.h"
#include "InstructionJ.h"
#include "InstructionI.h"


spect::Instruction::Instruction(std::string mnemonic, InstructionType itype,
                                uint32_t opcode, uint32_t func, int op_mask) :
    itype_(itype),
    opcode_(opcode),
    func_(func),
    mnemonic_(mnemonic),
    op_mask_(op_mask)
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
