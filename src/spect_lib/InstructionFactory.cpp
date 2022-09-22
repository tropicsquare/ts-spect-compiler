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

#include <iostream>

#include "InstructionDefs.h"
#include "InstructionFactory.h"

#define REGISTER_R_INSTRUCTION(name)                                        \
    InstructionFactory::Register(new name(CpuGpr::R0, CpuGpr::R0, CpuGpr::R0));
    
#define REGISTER_I_INSTRUCTION(name)                                        \
    InstructionFactory::Register(new name(CpuGpr::R0, CpuGpr::R0, 0x0));

#define REGISTER_M_INSTRUCTION(name)                                        \
    InstructionFactory::Register(new name(CpuGpr::R0, 0x0));

#define REGISTER_J_INSTRUCTION(name)                                        \
    InstructionFactory::Register(new name(0x0));



void spect::InstructionFactory::Register(spect::Instruction *instr)
{
    mnemonic_map_[instr->mnemonic_] = instr;
    uint32_t enc = INSTR_ENCODE(instr->func_, instr->opcode_, instr->itype_);
    encoding_map_[enc] = instr;
}


bool spect::InstructionFactory::Initialize()
{
    /////////////////////////////////////////////////////////////////////////////////////
    // List of macros defined by CMake from InstructionDefs.txt
    // Create call to register each instruction within factory like so:
    //  REGISTER_?_INSTRUCTION(name)
    //  where:
    //      ? - Instruction type
    //   name - Name of instruction class (Instruction<MNEMONIC>)
    /////////////////////////////////////////////////////////////////////////////////////
    SPECT_REGISTER_INSTRUCTIONS

    return true;
}

spect::InstructionFactory::~InstructionFactory()
{
    for (auto &instr : mnemonic_map_)
        delete instr.second;
}

spect::Instruction* spect::InstructionFactory::GetInstruction(uint32_t enc)
{
    auto it = encoding_map_.find(enc);
    if (it == encoding_map_.end())
        return nullptr;

    return encoding_map_[enc];
}

spect::Instruction* spect::InstructionFactory::GetInstruction(std::string mnemonic)
{
    auto it = mnemonic_map_.find(mnemonic);
    if (it == mnemonic_map_.end())
        return nullptr;
    return mnemonic_map_[mnemonic]; 
}

std::map<std::string, spect::Instruction*> spect::InstructionFactory::mnemonic_map_;

std::map<uint32_t, spect::Instruction*> spect::InstructionFactory::encoding_map_;

bool spect::InstructionFactory::initialized_ = spect::InstructionFactory::Initialize();
