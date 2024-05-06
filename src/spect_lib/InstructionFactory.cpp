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

#define REGISTER_R_INSTRUCTION(version, name)                                               \
    InstructionFactory::Register(version, new name(CpuGpr::R0, CpuGpr::R0, CpuGpr::R0));

#define REGISTER_I_INSTRUCTION(version, name)                                               \
    InstructionFactory::Register(version, new name(CpuGpr::R0, CpuGpr::R0, 0x0));

#define REGISTER_M_INSTRUCTION(version, name)                                               \
    InstructionFactory::Register(version, new name(CpuGpr::R0, 0x0));

#define REGISTER_J_INSTRUCTION(version, name)                                               \
    InstructionFactory::Register(version, new name(0x0));



void spect::InstructionFactory::Register(int isa_version, spect::Instruction *instr)
{
    assert(isa_version > 0 && isa_version <= NUM_ISA_VERSIONS);

    std::shared_ptr<Instruction> sinstr(instr);

    mnemonic_maps_[isa_version - 1][instr->mnemonic_] = sinstr;
    uint32_t enc = INSTR_ENCODE(instr->func_, instr->opcode_, instr->itype_);
    encoding_maps_[isa_version - 1][enc] = sinstr;
}


bool spect::InstructionFactory::Initialize()
{
    /////////////////////////////////////////////////////////////////////////////////////
    // List of macros defined by CMake from InstructionDefs.txt
    // Create call to register each instruction within factory like so:
    //  REGISTER_?_INSTRUCTION(version, name)
    //  where:
    //      ?       - Instruction type
    //   version    - SPECT ISA version
    //   name       - Name of instruction class (V<version>Instruction<MNEMONIC>)
    /////////////////////////////////////////////////////////////////////////////////////
    SPECT_REGISTER_INSTRUCTIONS_V1
    SPECT_REGISTER_INSTRUCTIONS_V2

    // By default start from latest ISA version
    SetActiveISAVersion(NUM_ISA_VERSIONS);

    return true;
}

spect::InstructionFactory::~InstructionFactory()
{

}

spect::Instruction* spect::InstructionFactory::GetInstruction(uint32_t enc)
{
    auto it = encoding_maps_[active_isa_map_index].find(enc);
    if (it == encoding_maps_[active_isa_map_index].end())
        return nullptr;

    return encoding_maps_[active_isa_map_index][enc].get();
}

spect::Instruction* spect::InstructionFactory::GetInstruction(std::string mnemonic)
{
    auto it = mnemonic_maps_[active_isa_map_index].find(mnemonic);
    if (it == mnemonic_maps_[active_isa_map_index].end())
        return nullptr;
    return mnemonic_maps_[active_isa_map_index][mnemonic].get();
}

std::map<std::string, std::shared_ptr<spect::Instruction>>::iterator spect::InstructionFactory::GetInstructionIterator()
{
    return mnemonic_maps_[active_isa_map_index].begin();
}

bool spect::InstructionFactory::IteratorIsLast(
    std::map<std::string, std::shared_ptr<spect::Instruction>>::iterator &it)
{
    return (it == mnemonic_maps_[active_isa_map_index].end());
}

void spect::InstructionFactory::SetActiveISAVersion(int isa_version)
{
    assert(isa_version > 0 && isa_version <= NUM_ISA_VERSIONS);

    // ISA versions starts from 1. Map arrays are indexed from 0
    active_isa_map_index = isa_version - 1;
}

int spect::InstructionFactory::GetActiveISAVersion(void)
{
    return active_isa_map_index + 1;
}

thread_local std::map<std::string, std::shared_ptr<spect::Instruction>> spect::InstructionFactory::mnemonic_maps_[NUM_ISA_VERSIONS];;

thread_local std::map<uint32_t, std::shared_ptr<spect::Instruction>> spect::InstructionFactory::encoding_maps_[NUM_ISA_VERSIONS];;

bool spect::InstructionFactory::initialized_ = spect::InstructionFactory::Initialize();

int spect::InstructionFactory::active_isa_map_index = 0;
