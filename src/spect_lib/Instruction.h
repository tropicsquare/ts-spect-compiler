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

#ifndef SPECT_LIB_INSTRUCTION_H_
#define SPECT_LIB_INSTRUCTION_H_

#include <list>

#include "spect.h"

class spect::Instruction
{
    public:
        Instruction(std::string mnemonic, InstructionType itype, uint32_t opcode,
                    uint32_t func, int op_mask);
        virtual ~Instruction();
        void Dump(std::ostream& os);
        virtual bool Execute() = 0;
        virtual uint32_t Assemble() = 0;
        virtual Instruction* Clone() = 0;
        static spect::Instruction* DisAssemble(uint32_t wrd);
        virtual Symbol* Relocate() = 0;

        const InstructionType itype_;
        const uint32_t opcode_ : IENC_OPCODE_BITS;
        const uint32_t func_ : IENC_FUNC_BITS;
        const std::string mnemonic_;
        int op_mask_;
        int cycles_ = 0;

        spect::Symbol *s_label_ = nullptr;

    public:
        CpuModel *model_ = nullptr;
};

#endif