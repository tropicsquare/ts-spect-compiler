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

#include "InstructionR.h"
#include "InstructionFactory.h"

#include <iostream>

spect::InstructionR::InstructionR(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                                  CpuGpr op1, CpuGpr op2, CpuGpr op3) :
    Instruction(mnemonic, InstructionType::R, opcode, func, op_mask),
    op1_(op1),
    op2_(op2),
    op3_(op3)
{}

spect::Symbol* spect::InstructionR::Relocate()
{
    return nullptr;
}

uint32_t spect::InstructionR::Assemble()
{
    return (((TO_INT(op3_)   & IENC_OP_MASK)        << IENC_OP3_OFFSET)         |
            ((TO_INT(op2_)   & IENC_OP_MASK)        << IENC_OP2_OFFSET)         |
            ((TO_INT(op1_)   & IENC_OP_MASK)        << IENC_OP1_OFFSET)         |
            ((func_          & IENC_FUNC_MASK)      << IENC_FUNC_OFFSET)        |
            ((opcode_        & IENC_OPCODE_MASK)    << IENC_OPCODE_OFFSET)      |
            ((TO_INT(itype_) & IENC_TYPE_MASK)      << IENC_TYPE_OFFSET)
           );
}

spect::Instruction* spect::InstructionR::DisAssemble(uint32_t wrd)
{
    uint32_t op1    = (wrd >> IENC_OP1_OFFSET)    & IENC_OP_MASK;
    uint32_t op2    = (wrd >> IENC_OP2_OFFSET)    & IENC_OP_MASK;
    uint32_t op3    = (wrd >> IENC_OP3_OFFSET)    & IENC_OP_MASK;
    uint32_t func   = (wrd >> IENC_FUNC_OFFSET)   & IENC_FUNC_MASK;
    uint32_t opcode = (wrd >> IENC_OPCODE_OFFSET) & IENC_OPCODE_MASK;
    uint32_t itype  = (wrd >> IENC_TYPE_OFFSET)   & IENC_TYPE_MASK;

    spect::Instruction *instr = spect::InstructionFactory::
                                GetInstruction(INSTR_ENCODE(func, opcode, itype));
    spect::InstructionR *cln = nullptr;
    if (instr) {
        cln = (spect::InstructionR*) instr->Clone();
        cln->op1_ = TO_CPU_GPR(op1);
        cln->op2_ = TO_CPU_GPR(op2);
        cln->op3_ = TO_CPU_GPR(op3);
    }

    return cln;
}

void spect::InstructionR::Dump(std::ostream& os)
{
    os << op1_ << "," << op2_ << "," << op3_;
}