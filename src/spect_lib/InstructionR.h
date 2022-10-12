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

#ifndef SPECT_LIB_INSTRUCTION_R_H_
#define SPECT_LIB_INSTRUCTION_R_H_

#include "spect.h"
#include "Instruction.h"


class spect::InstructionR : public Instruction
{
    public:
        InstructionR(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                     CpuGpr op1, CpuGpr op2, CpuGpr op3);
        void Dump(std::ostream& os);
        spect::Symbol* Relocate();
        uint32_t Assemble();
        static Instruction* DisAssemble(uint32_t wrd);

        CpuGpr op1_;
        CpuGpr op2_;
        CpuGpr op3_;
};

#endif