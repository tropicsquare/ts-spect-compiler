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

#include "CpuFault.h"

spect::CpuFault::CpuFault(){}

spect::CpuFault::CpuFault(
    uint32_t            inst_addr,
    uint32_t            inst_exec_cnt,
    uint32_t            fault_data,
    const std::string   description
) {
    m_inst_addr        = inst_addr;
    m_inst_exec_cnt    = inst_exec_cnt;
    m_fault_data       = fault_data;
    m_description      = description;
}

spect::CpuFault::CpuFault(const std::string fault_line) {
    std::stringstream is(fault_line);
    is >> std::hex >> m_inst_addr >> std::dec >> m_inst_exec_cnt >> std::hex >> m_fault_data;
    is >> m_description;
}

spect::CpuFault::~CpuFault() {}

bool spect::CpuFault::Check(const uint32_t addr, const uint32_t exec_cnt) {
    return (addr == m_inst_addr) && (exec_cnt == m_inst_exec_cnt);
}

void spect::CpuFault::Apply(uint32_t * wrd) {
    DebugInfo(VERBOSITY_MEDIUM, "Injecting Fault:", m_description);
    *wrd = m_fault_data;
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
