/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef SPECT_LIB_CPU_PROGRAM_H_
#define SPECT_LIB_CPU_PROGRAM_H_

#include <vector>
#include <iostream>

#include "Instruction.h"

class spect::CpuProgram
{
    public:
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief New CPU Program constructor
        /// @param expected_size Expected size of the program. Pre-allocates instruction array.
        /// @returns New CPU Program object
        ///////////////////////////////////////////////////////////////////////////////////////////
        CpuProgram(size_t expected_size);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Append insruction to the end of CPU Program
        /// @param instr Pointer to instruction to append.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void AppendInstruction(spect::Instruction *instr);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Assemble the program
        /// @param mem Pointer to memory where the program shall be assembled
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Assemble(uint32_t *mem);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Assemble the program
        /// @param path Path to HEX file where the program shall be assembled.
        /// @param hex_type Type (format) of HEX file to generate
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Assemble(std::string path, spect::HexFileType hex_type);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Dump the program
        /// @param os Stream to dump the program into
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Dump(std::ostream &os);

        // Address of first instruction of program (not first executed instruction, but instruction
        //  with lowest address)
        uint32_t first_addr_;

        // Reference to compiler which compiled this program
        Compiler *compiler_;

    private:
        std::vector<spect::Instruction*> code_;

};

#endif