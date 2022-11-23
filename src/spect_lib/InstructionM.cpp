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
#include "InstructionM.h"
#include "InstructionFactory.h"
#include "Symbol.h"

spect::InstructionM::InstructionM(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                                  CpuGpr op1, uint16_t addr, bool r31_dep, bool c_time) :
    Instruction(mnemonic, InstructionType::M, opcode, func, op_mask, r31_dep, c_time),
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
    for (int i = 2; i >= 0; i--)
        if (op_mask_ & (1 << i)) {
            if (i == 2)
                os << op1_;
            else if (i == 1)
                os << "," << addr_;
        }
}

bool spect::InstructionM::Execute()
{
    model_->DebugInfo(VERBOSITY_MEDIUM, "Inputs before execution:");

    if (op_mask_ & 0x4) {
        std::stringstream ss;
        ss << "    " << op1_ << ": " << std::hex << "0x" << model_->GetGpr(TO_INT(op1_));
        model_->DebugInfo(VERBOSITY_MEDIUM, ss.str().c_str());
    }

    if (op_mask_ & 0x2) {
        std::stringstream ss;
        ss << "    " << "Addr:" << std::hex << "0x" << addr_;
        model_->DebugInfo(VERBOSITY_MEDIUM, ss.str().c_str());
    }

    return true;
}

void spect::InstructionM::SampleInputs(dpi_instruction_t *dpi_instr, A_UNUSED CpuModel *model)
{
    if (op_mask_ & 0b100)
        dpi_instr->op1 = TO_INT(op1_);
    if (op_mask_ & 0b110)
        dpi_instr->addr = addr_;
}

void spect::InstructionM::SampleOutputs(dpi_instruction_t *dpi_instr, CpuModel *model)
{
    for (int i = 0; i < 8; i++)
        if (op_mask_ & 0b100)
            dpi_instr->op1_v[i] = (uint32_t)(model->GetGpr(TO_INT(op1_)) >> (32 * i));
}