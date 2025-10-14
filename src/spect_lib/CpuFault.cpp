/******************************************************************************
*
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
*
* @todo: License
*
* @author Vit Masek, <vit.masek@tropicsquare.com>
* @date 10.10.2025
*
*****************************************************************************/

#include <fstream>
#include <iostream>

#include "CpuFault.h"

spect::CpuFault::CpuFault(uint32_t inst_addr, uint32_t inst_exec_cnt, uint32_t fault_data, const std::string description) {
    inst_addr_       = inst_addr;
    inst_exec_cnt_   = inst_exec_cnt;
    fault_data_      = fault_data;
    description_     = description;
}

spect::CpuFault::CpuFault(const std::string &path) {
    std::ifstream ifs(path);
    if (ifs.is_open()) {
        ifs >> std::hex >> inst_addr_ >> std::dec >> inst_exec_cnt_ >> std::hex >> fault_data_;
        //std::cout << inst_addr_ << " " << inst_exec_cnt_ << " " << fault_data_ << std::endl;
        ifs >> description_;
        DebugInfo(VERBOSITY_MEDIUM, "Loaded fault:", description_);
    }
    else
        throw std::runtime_error("Unable to open a file: " + path);
}

bool spect::CpuFault::Check(const uint32_t addr, const uint32_t exec_cnt) {
    return (addr == inst_addr_) && (exec_cnt == inst_exec_cnt_);
}

void spect::CpuFault::Apply(uint32_t * wrd) {
    DebugInfo(VERBOSITY_MEDIUM, "Injecting Fault:", description_);
    *wrd = fault_data_;
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
