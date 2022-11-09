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

    private:

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Register instruction within Instruction factory
        /// @param instr Pointer to instruction to register
        ///////////////////////////////////////////////////////////////////////////////////////////
        static void Register(spect::Instruction *instr);

        // True - Factory is initialized (All instructions registered), False otherwise
        static bool initialized_;

        // Hash map with instructions: mnemonic -> *Instruction
        static std::map<std::string, spect::Instruction*> mnemonic_map_;

        // Hash map with instructions: encoding -> *Instruction
        static std::map<uint32_t, spect::Instruction*> encoding_map_;
};

#endif