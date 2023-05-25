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
#include "spect_iss_dpi_types.h"

class spect::Instruction
{
    public:
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief New instruction constructor
        /// @param mnemonic Instruction mnemonic (as seen in '.s' file)
        /// @param itype Instruction Type (see SPECT Programmers manual)
        /// @param opcode Instruction op-code (see SPECT Design specification)
        /// @param func Intsruction function (see SPECT Design specification)
        /// @param op_mask Instruction operand mask (operands required in the '.s' file).
        /// @param r31_dep True - Instruction depends on content of register 31, False otherwise.
        /// @param c_time True - Instruction shall execute in constant time, False otherwise.
        ///////////////////////////////////////////////////////////////////////////////////////////
        Instruction(std::string mnemonic, InstructionType itype, uint32_t opcode,
                    uint32_t func, int op_mask, bool r31_dep, bool c_time);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Instruction destructor
        ///////////////////////////////////////////////////////////////////////////////////////////
        virtual ~Instruction();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Dumps the instruction
        /// @param os Stream to dump the instruction to
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Dump(std::ostream& os);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Dumps the instruction
        /// @returns String with dumped instruction
        ///////////////////////////////////////////////////////////////////////////////////////////
        std::string Dump();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Execute the instruction
        /// @returns True  - PC shall be increased by +0x4 after the call (e.g ADD)/
        ///          False - PC shall not be increased, instruction modfied the PC by itself
        ///                  (e.g. JUMP)
        ///////////////////////////////////////////////////////////////////////////////////////////
        virtual bool Execute() = 0;

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Assemble the instruction
        /// @param parity_type Type of parity to generate
        /// @returns Assembled encoding of the instruction
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t Assemble(spect::ParityType parity_type);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Clone the instruction
        /// @returns New instruction object living on heap, matches attributes of object which
        ///          called this function.
        ///////////////////////////////////////////////////////////////////////////////////////////
        virtual Instruction* Clone() = 0;

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Clone the instruction
        /// @param parity_type Type of parity to check
        /// @returns New instruction object living on heap, matches attributes of object which
        ///          called this function.
        ///////////////////////////////////////////////////////////////////////////////////////////
        static spect::Instruction* DisAssemble(spect::ParityType parity_type, uint32_t wrd);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Relocate the instruction.
        ///         If instruction has symbols attached to it, then values of these symbols are
        ///         filed to a attribute which can correspond to symbol (immediate, newPC or Addr)
        /// @returns nullptr - If symbol attached to instruction was relocated or if there is no
        ///                    Symbol attached to the instruction (in such case instruction does
        ///                    not change).
        ///          Pointer to Symbol - If attached symbol is unresolved.
        ///////////////////////////////////////////////////////////////////////////////////////////
        virtual Symbol* Relocate() = 0;

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Samples instruction input parameter values from a CPU model to DPI structure
        /// @param dpi_instr DPI instruction structure where to sample
        /// @param model Model from which to sample the instructions
        /// @note This function shall be called BEFORE the instruction was executed to sample
        ///       valid results.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void SampleInputs(dpi_instruction_t *dpi_instr, CpuModel *model);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Samples instruction input parameters from a CPU model to DPI structure
        /// @param dpi_instr DPI instruction structure where to sample
        /// @param model Model from which to sample the instructions
        /// @note This function shall be called AFTER the instruction was executed to sample
        ///       valid results.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void SampleOutputs(dpi_instruction_t *dpi_instr, CpuModel *model);

    private:
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Check Parity
        /// @param parity_type Type of parity to check
        /// @param wrd Data word
        /// @returns True if the parity check passes, false otherwise.
        ///////////////////////////////////////////////////////////////////////////////////////////
        static bool CheckParity(spect::ParityType parity_type, uint32_t wrd);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Compute Parity
        /// @param parity_type Type of parity to generate
        /// @param wrd Data word
        /// @returns Parity bit
        ///////////////////////////////////////////////////////////////////////////////////////////
        static uint32_t ComputeParity(spect::ParityType parity_type, uint32_t wrd);


    public:
        // Instruction type
        const InstructionType itype_;

        // Instruction opcode
        const uint32_t opcode_ : IENC_OPCODE_BITS;

        // Instruction function
        const uint32_t func_ : IENC_FUNC_BITS;

        // Instruction mnemonic (as placed in assembly file)
        const std::string mnemonic_;

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// Instruction operand mask
        ///  Sequence of 3 bits, if a bit is in 1, then the operand shall be present with the
        ///  instruction in the program (e.g.)
        ///         ADD r0,r1,r2        - op_mask 111
        ///         MOVI r0, Immediate  - op_mask 101
        ///////////////////////////////////////////////////////////////////////////////////////////
        int op_mask_;

        // Number of clock cycles instruction took to execute on RTL
        int cycles_ = 0;

        // Number of times instruction was executed
        uint64_t exec_cnt_ = 0;

        // Symbol with instruction label from .s file
        spect::Symbol *s_label_ = nullptr;

        // True when instruction depends on register 31 content
        bool r31_dep_;

        // Instruction executes in constant time
        bool c_time_;

    public:
        // SPECT Model attached to the instruction
        CpuModel *model_ = nullptr;
};

#endif
