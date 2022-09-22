/**************************************************************************************************
* 
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
* 
* @todo: License
*
* @author Ondrej Ille, <ondrej.ille@tropicsquare.com>
* @date 19.9.2022
* 
**************************************************************************************************/

#ifndef SPECT_LIB_INSTRUCTION_FACTORY_H_
#define SPECT_LIB_INSTRUCTION_FACTORY_H_

#include <map>

#include "spect.h"

#include "Instruction.h"

class spect::InstructionFactory
{
    public:
        static void Register(spect::Instruction *instr);
        static bool Initialize();
        ~InstructionFactory();
        static spect::Instruction* GetInstruction(uint32_t enc);
        static spect::Instruction* GetInstruction(std::string mnemonic);

    private:
        static bool initialized_;

        // By mnemonic accessible
        static std::map<std::string, spect::Instruction*> mnemonic_map_;

        // By encoding accessible
        static std::map<uint32_t, spect::Instruction*> encoding_map_;
};

#endif