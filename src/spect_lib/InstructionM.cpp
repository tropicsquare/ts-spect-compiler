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

#include "InstructionM.h"
#include "InstructionFactory.h"
#include "Symbol.h"

spect::InstructionM::InstructionM(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                                  CpuGpr op1, uint16_t addr) :
    Instruction(mnemonic, InstructionType::M, opcode, func, op_mask),
    op1_(op1),
    addr_(addr)
{}

spect::Symbol* spect::InstructionM::Relocate()
{
    if (s_addr_) {
        if (s_addr_->resolved_) {
            addr_ = s_addr_->val_;
        } else {
            return s_addr_;
        }
    }
    return nullptr;
}

uint32_t spect::InstructionM::Assemble()
{
    return (((addr_             & IENC_ADDR_MASK)       << IENC_ADDR_OFFSET)    |
            ((TO_INT(op1_)      & IENC_OP_MASK)         << IENC_OP1_OFFSET)     |
            ((func_             & IENC_FUNC_MASK)       << IENC_FUNC_OFFSET)    |
            ((opcode_           & IENC_OPCODE_MASK)     << IENC_OPCODE_OFFSET)  |
            ((TO_INT(itype_)    & IENC_TYPE_MASK)       << IENC_TYPE_OFFSET)
           );
}

spect::Instruction* spect::InstructionM::DisAssemble(uint32_t wrd)
{
    uint32_t addr   = (wrd >> IENC_ADDR_OFFSET)   & IENC_ADDR_MASK;
    uint32_t op1    = (wrd >> IENC_OP1_OFFSET)    & IENC_OP_MASK;
    uint32_t func   = (wrd >> IENC_FUNC_OFFSET)   & IENC_FUNC_MASK;
    uint32_t opcode = (wrd >> IENC_OPCODE_OFFSET) & IENC_OPCODE_MASK;
    uint32_t itype  = (wrd >> IENC_TYPE_OFFSET)   & IENC_TYPE_MASK;

    spect::Instruction *instr = spect::InstructionFactory::
                                GetInstruction(INSTR_ENCODE(func, opcode, itype));
    spect::InstructionM *cln = nullptr;
    if (instr) {
        cln = (spect::InstructionM*) instr->Clone();
        cln->op1_ = TO_CPU_GPR(op1);
        cln->addr_ = addr;
    }
    return cln;
}

void spect::InstructionM::Dump(std::ostream& os)
{
    os << op1_ << "," << addr_;
}
