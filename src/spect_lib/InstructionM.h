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

#ifndef SPECT_LIB_INSTRUCTION_M_H_
#define SPECT_LIB_INSTRUCTION_M_H_

#include "spect.h"

#include "Instruction.h"


class spect::InstructionM : public Instruction
{
    public:
        InstructionM(std::string mnemonic, uint32_t opcode, uint32_t func, CpuGpr op1, uint16_t addr);
        void Dump(std::ostream& os);
        spect::Symbol* Relocate();
        uint32_t Assemble();
        static Instruction* DisAssemble(uint32_t wrd);

        CpuGpr op1_;
        uint16_t addr_ : IENC_ADDR_BITS;

        spect::Symbol *s_addr_;
};

#endif