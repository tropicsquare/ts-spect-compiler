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
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Initialize instruction factory.
        ///        This method registers all instructions defined in InstructionDefs.txt
        ///////////////////////////////////////////////////////////////////////////////////////////
        static bool Initialize();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Instruction factory Destructor
        ///////////////////////////////////////////////////////////////////////////////////////////
        ~InstructionFactory();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @return Instruction object from factory
        /// @param enc Unique encoding of the instruction given by
        ///             INSTR_ENCODE(func, opcode, itype)
        /// @returns Pointer to instruction matching the encoding, nullptr if no instruction with
        ///          'enc' encoding has been registered.
        ///////////////////////////////////////////////////////////////////////////////////////////
        static spect::Instruction* GetInstruction(uint32_t enc);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @return Instruction object from factory
        /// @param enc Instruction mnemonic (as seen in .s file)
        /// @returns Pointer to instruction matching the mnemonic, nullptr if no instruction with
        ///          'mnemonic' has been registered.
        ///////////////////////////////////////////////////////////////////////////////////////////
        static spect::Instruction* GetInstruction(std::string mnemonic);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @return Instruction iterator over all registered instructions.
        ///////////////////////////////////////////////////////////////////////////////////////////
        static std::map<std::string, spect::Instruction*>::iterator GetInstructionIterator();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @returns True if iterator is at the last instruction
        ///////////////////////////////////////////////////////////////////////////////////////////
        static bool IteratorIsLast(std::map<std::string, spect::Instruction*>::iterator &it);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @returns True if iterator is at the last instruction
        /// @param isa_version Version of ISA under which to register an Instruction
        ///////////////////////////////////////////////////////////////////////////////////////////
        static void SetActiveISAVersion(int isa_version);

    private:

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Register instruction within Instruction factory
        /// @param instr Pointer to instruction to register
        /// @param isa_version Version of ISA under which to register an Instruction
        ///////////////////////////////////////////////////////////////////////////////////////////
        static void Register(int isa_version, spect::Instruction *instr);

        // True - Factory is initialized (All instructions registered), False otherwise
        static bool initialized_;

        // Current active version of ISA
        // Used to select a mnemonic map from array of mnemonic maps, each holding instructions
        // for a single ISA version.
        //  active_isa_map_index = isa_version - 1
        static int active_isa_map_index;

        // Hash maps with instructions: mnemonic -> *Instruction
        // Single map for each ISA version
        static std::map<std::string, spect::Instruction*> mnemonic_maps_[NUM_ISA_VERSIONS];

        // Hash maps with instructions: encoding -> *Instruction
        // Single map for each ISA version
        static std::map<uint32_t, spect::Instruction*> encoding_maps_[NUM_ISA_VERSIONS];
};

#endif