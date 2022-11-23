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

#ifndef SPECT_LIB_INSTRUCTION_I_H_
#define SPECT_LIB_INSTRUCTION_I_H_

#include "spect.h"
#include "Instruction.h"
#include "CpuModel.h"
#include "spect_iss_dpi_types.h"

class spect::InstructionI : public Instruction
{
    public:
        InstructionI(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                     CpuGpr op1, CpuGpr op2, uint16_t immediate, bool r31_dep, bool c_time);
        void Dump(std::ostream& os);
        spect::Symbol* Relocate();
        uint32_t Assemble();
        static Instruction* DisAssemble(uint32_t wrd);
        bool Execute();
        void SampleInputs(dpi_instruction_t *dpi_instr, CpuModel *model);
        void SampleOutputs(dpi_instruction_t *dpi_instr, CpuModel *model);

        CpuGpr op1_;
        CpuGpr op2_;
        uint16_t immediate_ : IENC_IMMEDIATE_BITS;

        spect::Symbol *s_immediate_ = nullptr;
};

#endif
