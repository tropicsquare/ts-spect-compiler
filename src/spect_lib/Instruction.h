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
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief New instruction constructor
        /// @param mnemonic Instruction mnemonic (as seen in '.s' file)
        /// @param itype Instruction Type (see SPECT Programmers manual)
        /// @param opcode Instruction op-code (see SPECT Design specification)
        /// @param func Intsruction function (see SPECT Design specification)
        /// @param op_mask Instruction operand mask (operands required in the '.s' file).
        ///////////////////////////////////////////////////////////////////////////////////////////
        Instruction(std::string mnemonic, InstructionType itype, uint32_t opcode,
                    uint32_t func, int op_mask);

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
        /// @returns Assembled encoding of the instruction
        ///////////////////////////////////////////////////////////////////////////////////////////
        virtual uint32_t Assemble() = 0;

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Clone the instruction
        /// @returns New instruction object living on heap, matches attributes of object which
        ///          called this function.
        ///////////////////////////////////////////////////////////////////////////////////////////
        virtual Instruction* Clone() = 0;

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Clone the instruction
        /// @returns New instruction object living on heap, matches attributes of object which
        ///          called this function.
        ///////////////////////////////////////////////////////////////////////////////////////////
        static spect::Instruction* DisAssemble(uint32_t wrd);

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

        // Symbol with instruction label from .s file
        spect::Symbol *s_label_ = nullptr;

    public:
        // SPECT Model attached to the instruction
        CpuModel *model_ = nullptr;
};

#endif