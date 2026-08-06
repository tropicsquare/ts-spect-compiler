/******************************************************************************
*
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
*
* @license For the license see file LICENSE.txt file in the root directory of this source tree.
*
*
*****************************************************************************/

#include <sstream>
#include <iostream>
#include <memory>

#include "CpuFault.h"

spect::CpuFault::CpuFault()     {}

bool spect::CpuFault::Check (const uint32_t addr, const uint32_t exec_cnt)
{
    return (addr == inst_addr_) && (exec_cnt == inst_exec_cnt_);
}

void spect::CpuFault::PrintArgs()
{
    print_fnc("\n");
}

template<typename Arg>
void spect::CpuFault::PrintArgs(Arg arg)
{
    std::stringstream ss;
    ss << std::hex << arg << std::endl;
    print_fnc(ss.str().c_str());
}

template<typename First, typename... Args>
void spect::CpuFault::PrintArgs(First first, Args... args)
{
    std::stringstream ss;
    ss << std::hex << first << " ";
    print_fnc(ss.str().c_str());
    PrintArgs(args...);
}

template<typename... Args>
void spect::CpuFault::DebugInfo(uint32_t verbosity_level, const Args ...args)
{
    if (verbosity_ >= verbosity_level)
        PrintArgs(FAULT_LABEL, args...);
}

spect::CpuFaultInstruction::CpuFaultInstruction(std::stringstream & is)
{
    is >> std::hex >> inst_addr_ >> std::dec >> inst_exec_cnt_ >> std::hex >> new_instruction_;
    is >> description_;
}

spect::CpuFaultPC::CpuFaultPC(std::stringstream & is)
{
    is >> std::hex >> inst_addr_ >> std::dec >> inst_exec_cnt_ >> skip_cnt_;
    is >> description_;
}

spect::CpuFaultGPR::CpuFaultGPR(std::stringstream & is)
{
    is >> std::hex >> inst_addr_ >> std::dec >> inst_exec_cnt_ >> gpr_index_ >> bitflip_pos_ >> std::hex >> bitflip_mask_;
    is >> is_transient_;
    is >> description_;
}

spect::CpuFaultMemory::CpuFaultMemory(std::stringstream & is)
{
    is >> std::hex >> inst_addr_ >> std::dec >> inst_exec_cnt_ >> std::hex >> mem_address_ >> bitflip_mask_;
    is >> is_transient_;
    is >> description_;
}

FaultType   spect::CpuFaultInstruction::GetType () const
{
    return FaultType::INSTRUCTION;
}
FaultType   spect::CpuFaultPC::GetType () const
{
    return FaultType::PC;
}
FaultType   spect::CpuFaultGPR::GetType () const
{
    return FaultType::GPR;
}
FaultType   spect::CpuFaultMemory::GetType () const
{
    return FaultType::MEMORY;
}

void        spect::CpuFaultInstruction::Apply  (uint32_t *data)
{
    DebugInfo(VERBOSITY_MEDIUM, "Apply Instruction Fault:", description_);
    DebugInfo(VERBOSITY_MEDIUM, "Original : ", *data);
    DebugInfo(VERBOSITY_MEDIUM, "Faulted  : ", new_instruction_);
    *data = new_instruction_;
}
void        spect::CpuFaultPC::Apply  (uint32_t *data)
{
    DebugInfo(VERBOSITY_MEDIUM, "Apply Program Counter Fault:", description_);
    uint32_t new_pc = *data + (skip_cnt_ << 2);
    DebugInfo(VERBOSITY_MEDIUM, "Original : ", *data);
    DebugInfo(VERBOSITY_MEDIUM, "Faulted  : ", new_pc);
    *data = new_pc;
}
void        spect::CpuFaultGPR::Apply  (uint256_t *data)
{
    DebugInfo(VERBOSITY_MEDIUM, "Apply GPR Fault:", description_);
    uint256_t new_gpr = *data ^ ((uint256_t)(bitflip_mask_) << bitflip_pos_);
    DebugInfo(VERBOSITY_MEDIUM, "Original : ", *data);
    DebugInfo(VERBOSITY_MEDIUM, "Faulted  : ", new_gpr);
    *data = new_gpr;
}
void        spect::CpuFaultMemory::Apply  (uint32_t *data)
{
    DebugInfo(VERBOSITY_MEDIUM, "Apply Memory Fault:", description_);
    uint32_t new_data = *data ^ bitflip_mask_;
    DebugInfo(VERBOSITY_MEDIUM, "Original : ", *data);
    DebugInfo(VERBOSITY_MEDIUM, "Faulted  : ", new_data);
    *data = new_data;
}

int         spect::CpuFaultGPR::GetGPRIndex ()
{
    return gpr_index_;
}

uint16_t    spect::CpuFaultMemory::GetMemAddress ()
{
    return mem_address_;
}

bool        spect::CpuFaultGPR::IsTransient () {
    return is_transient_;
}

bool        spect::CpuFaultMemory::IsTransient () {
    return is_transient_;
}

std::unique_ptr<spect::CpuFault> GetFault (const std::string fault_line)
{
    std::stringstream is(fault_line);
    int fault_type;
    is >> fault_type;

    switch (fault_type) {
        case FaultType::INSTRUCTION :
            return std::make_unique<spect::CpuFaultInstruction>(is);

        case FaultType::PC :
            return std::make_unique<spect::CpuFaultPC>(is);

        case FaultType::GPR :
            return std::make_unique<spect::CpuFaultGPR>(is);

        case FaultType::MEMORY :
            return std::make_unique<spect::CpuFaultMemory>(is);
    }

    return nullptr;
}
