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

#ifndef SPECT_LIB_INSTRUCTION_DEFS_H_
#define SPECT_LIB_INSTRUCTION_DEFS_H_

#include "spect.h"

#include "CpuModel.h"
#include "Instruction.h"

#include "InstructionR.h"
#include "InstructionI.h"
#include "InstructionM.h"
#include "InstructionJ.h"

#include "spect_iss_dpi_types.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Macros for definitions of instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFINE_R_INSTRUCTION(name,mnemonic,opcode,func)                                 \
    class name : public spect::InstructionR {                                           \
        public:                                                                         \
            name(CpuGpr op1, CpuGpr op2, CpuGpr op3) :                                  \
                InstructionR(std::string(mnemonic), opcode, func, op1, op2, op3)        \
                {};                                                                     \
            spect::Instruction* Clone() {                                               \
                return new name(op1_, op2_, op3_);                                      \
            }                                                                           \
            bool Execute();                                                             \
    };

#define DEFINE_I_INSTRUCTION(name,mnemonic,opcode,func)                                 \
    class name : public spect::InstructionI {                                           \
        public:                                                                         \
            name(CpuGpr op1, CpuGpr op2, uint16_t immediate) :                          \
                InstructionI(std::string(mnemonic), opcode, func, op1, op2, immediate)  \
                {};                                                                     \
            spect::Instruction* Clone() {                                               \
                return new name(op1_, op2_, immediate_);                                \
            }                                                                           \
            bool Execute();                                                             \
    };                                                                                  \

#define DEFINE_M_INSTRUCTION(name,mnemonic,opcode,func)                                 \
    class name : public spect::InstructionM {                                           \
        public:                                                                         \
            name(CpuGpr op1, uint16_t addr) :                                           \
                InstructionM(std::string(mnemonic), opcode, func, op1, addr)            \
                {};                                                                     \
            spect::Instruction* Clone() {                                               \
                return new name(op1_, addr_);                                           \
            }                                                                           \
            bool Execute();                                                             \
    };                                                                                  \

#define DEFINE_J_INSTRUCTION(name,mnemonic,opcode,func)                                 \
    class name : public spect::InstructionJ {                                           \
        public:                                                                         \
            name(uint16_t new_pc) :                                                     \
                InstructionJ(std::string(mnemonic), opcode, func, new_pc)               \
                {};                                                                     \
            spect::Instruction* Clone() {                                               \
                return new name(new_pc_);                                               \
            }                                                                           \
            bool Execute();                                                             \
    };                                                                                  \


///////////////////////////////////////////////////////////////////////////////////////////////////
// Macros to create/report changes in the model
///////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFINE_CHANGE(name, change_kind, obj_id)                                        \
    dpi_state_change_t name;                                                            \
    name.kind = change_kind;                                                            \
    name.obj = obj_id;                                                                  \

// TODO: Check typecast without explciit masking is OK!
#define PUT_GPR_TO_CHANGE(chn, old_or_new, gpr)                                         \
    for (int i = 0; i < 8; i++)                                                         \
        chn.old_or_new[i] = uint32_t(gpr >> (32 * i));                                  \

#define PUT_FLAG_TO_CHANGE(chn, old_or_new, flag)                                       \
    chn.old_or_new[0] = flag;                                                           \



///////////////////////////////////////////////////////////////////////////////////////////////////
// List of macros defined by CMake from InstructionDefs.txt
// Create class declaration for each instruction in format:
//  DEFINE_?_INSTRUCTION(InstructionXYZ,...)
//  where:
//      ? - Instruction type
//      XYZ - Instruction Mnemonic
//      ... - Arguments as needed to construct Instruction? type object
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace spect {
    
    SPECT_DEFINE_INSTRUCTIONS

}

#endif