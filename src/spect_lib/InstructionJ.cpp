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

#include "CpuModel.h"
#include "InstructionJ.h"
#include "InstructionFactory.h"
#include "Symbol.h"

spect::InstructionJ::InstructionJ(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                                  uint16_t new_pc, bool r31_dep, bool c_time) :
    Instruction(mnemonic, InstructionType::J, opcode, func, op_mask, r31_dep, c_time),
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

    spect::Instruction *instr = spect::InstructionFactory::
                                GetInstruction(INSTR_ENCODE(func, opcode, itype));
    spect::InstructionJ *cln = nullptr;
    if (instr) {
        cln = (spect::InstructionJ*) instr->Clone();
        cln->new_pc_ = new_pc;
    }

    return cln;
}


void spect::InstructionJ::Dump(std::ostream& os)
{
    for (int i = 2; i >= 0; i--)
        if (op_mask_ & (1 << i)) {
            if (i == 2)
                os << new_pc_;
        }
}

bool spect::InstructionJ::Execute()
{
    model_->DebugInfo(VERBOSITY_MEDIUM, "Inputs before execution:");

    if (op_mask_ & 0x4) {
        std::stringstream ss;
        ss << "    " << "NewPC:" << std::hex << "0x" << new_pc_;
        model_->DebugInfo(VERBOSITY_MEDIUM, ss.str().c_str());
    }

    return true;
}

void spect::InstructionJ::SampleInputs(dpi_instruction_t *dpi_instr, A_UNUSED CpuModel *model)
{
    if (op_mask_ & 0b100)
        dpi_instr->new_pc = new_pc_;
}

void spect::InstructionJ::SampleOutputs(A_UNUSED dpi_instruction_t *dpi_instr,
                                        A_UNUSED CpuModel *model)
{}