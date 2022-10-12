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

#include "InstructionJ.h"
#include "InstructionFactory.h"
#include "Symbol.h"

spect::InstructionJ::InstructionJ(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                                  uint16_t new_pc) :
    Instruction(mnemonic, InstructionType::J, opcode, func, op_mask),
    new_pc_(new_pc)
{}

spect::Symbol* spect::InstructionJ::Relocate()
{
    if (s_new_pc_) {
        if (s_new_pc_->resolved_) {
            new_pc_ = s_new_pc_->val_;
        } else {
            return s_new_pc_;
        }
    }
    return nullptr;
}

uint32_t spect::InstructionJ::Assemble()
{
    return (((new_pc_           & IENC_NEW_PC_MASK) << IENC_NEW_PC_OFFSET)  |
            ((func_             & IENC_FUNC_MASK)   << IENC_FUNC_OFFSET)    |
            ((opcode_           & IENC_OPCODE_MASK) << IENC_OPCODE_OFFSET)  |
            ((TO_INT(itype_)    & IENC_TYPE_MASK)   << IENC_TYPE_OFFSET)
           );
}

spect::Instruction* spect::InstructionJ::DisAssemble(uint32_t wrd)
{
    uint32_t new_pc = (wrd >> IENC_NEW_PC_OFFSET) & IENC_NEW_PC_MASK;
    uint32_t func   = (wrd >> IENC_FUNC_OFFSET)   & IENC_FUNC_MASK;
    uint32_t opcode = (wrd >> IENC_OPCODE_OFFSET) & IENC_OPCODE_MASK;
    uint32_t itype  = (wrd >> IENC_TYPE_OFFSET)   & IENC_TYPE_MASK;

    spect::InstructionJ *cln = (spect::InstructionJ*) spect::InstructionFactory::
            GetInstruction(INSTR_ENCODE(func, opcode, itype))->Clone();
    cln->new_pc_ = new_pc;
    return cln;
}

void spect::InstructionJ::Dump(std::ostream& os)
{
    os << new_pc_;
}