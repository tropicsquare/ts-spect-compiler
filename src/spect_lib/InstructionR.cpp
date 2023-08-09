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
#include "InstructionR.h"
#include "InstructionFactory.h"

#include <iostream>

spect::InstructionR::InstructionR(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                                  CpuGpr op1, CpuGpr op2, CpuGpr op3, bool r31_dep, bool c_time,
                                  int cycles) :
    Instruction(mnemonic, InstructionType::R, opcode, func, op_mask, r31_dep, c_time, cycles),
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
    for (int i = 2; i >= 0; i--)
        if (op_mask_ & (1 << i)) {
            if (i == 2)
                os << op1_;
            else if (i == 1)
                os << "," << op2_;
            else
                os << "," << op3_;
        }
}

bool spect::InstructionR::Execute()
{
    model_->DebugInfo(VERBOSITY_MEDIUM, "Inputs before execution:");

    if (op_mask_ & 0x2) {
        std::stringstream ss;
        ss << "    " << op2_ << ": " << std::hex << "0x" << model_->GetGpr(TO_INT(op2_));
        model_->DebugInfo(VERBOSITY_MEDIUM, ss.str().c_str());
    }

    if (op_mask_ & 0x1) {
        std::stringstream ss;
        ss << "    " << op3_ << ": " << std::hex << "0x" << model_->GetGpr(TO_INT(op3_));
        model_->DebugInfo(VERBOSITY_MEDIUM, ss.str().c_str());
    }

    if (r31_dep_) {
        std::stringstream ss;
        ss << "    " << "R31" << ": " << std::hex << "0x" << model_->GetGpr(31);
        model_->DebugInfo(VERBOSITY_MEDIUM, ss.str().c_str());
    }

    return true;
}

void spect::InstructionR::SampleInputs(dpi_instruction_t *dpi_instr, CpuModel *model)
{
    if (op_mask_ & 0b100)
        dpi_instr->op1 = TO_INT(op1_);
    if (op_mask_ & 0b010)
        dpi_instr->op2 = TO_INT(op2_);
    if (op_mask_ & 0b001)
        dpi_instr->op3 = TO_INT(op3_);

    for (int i = 0; i < 8; i++) {
        if (op_mask_ & 0b010)
            dpi_instr->op2_v[i] = (uint32_t)(model->GetGpr(TO_INT(op2_)) >> (32 * i));
        if (op_mask_ & 0b001)
            dpi_instr->op3_v[i] = (uint32_t)(model->GetGpr(TO_INT(op3_)) >> (32 * i));
        if (r31_dep_)
            dpi_instr->r31_v[i] = (uint32_t)(model->GetGpr(31          ) >> (32 * i));
    }
}


void spect::InstructionR::SampleOutputs(dpi_instruction_t *dpi_instr, CpuModel *model)
{
    for (int i = 0; i < 8; i++)
        if (op_mask_ & 0b100)
            dpi_instr->op1_v[i] = (uint32_t)(model->GetGpr(TO_INT(op1_)) >> (32 * i));
}
