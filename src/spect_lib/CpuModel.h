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

#ifndef SPECT_LIB_CPU_MODEL_H_
#define SPECT_LIB_CPU_MODEL_H_

#include <queue>

#include "spect.h"
#include "CpuProgram.h"
#include "Sha512.h"

#include "ordt_pio_common.hpp"
#include "ordt_pio.hpp"

#include "spect_iss_dpi_types.h"

class spect::CpuModel
{
    public:
        CpuModel(bool instr_mem_rw);
        ~CpuModel();

        void Start();
        int  Step(int n);
        int  StepSingle(int cycles);
        void Reset();

        void UpdateInterrupts();
        void UpdateRegisterEffects();

        // Accessors for direct access to CPU memory
        void SetMemory(uint16_t address, uint32_t data);
        uint32_t GetMemory(uint16_t address);

        // Accessors to memory space from outside of CPU
        uint32_t ReadMemoryAhb(uint16_t address);
        void WriteMemoryAhb(uint16_t address, uint32_t data);

        // Accessors to memory space from inside of CPU
        uint32_t ReadMemoryCoreData(uint16_t address);
        uint32_t WriteMemoryCoreData(uint16_t address, uint32_t data);
        uint32_t ReadMemoryCoreFetch(uint16_t address);

        void FillMemory(CpuMemory mem, uint32_t val);

        // Core state
        uint256_t   gpr_[SPECT_GPR_CNT];
        uint16_t    pc_;
        CpuFlags    flags_;
        Sha512      sha_512_;
        uint256_t   srr_;
        uint16_t    rar_stack_[SPECT_RAR_DEPTH];
        uint16_t    rar_sp_;
        bool        end_executed_;

        // Address of first instruction to be executed
        uint16_t    start_pc_ = SPECT_INSTR_MEM_BASE;

        // Memory subsystem (flat 16 bit space (64 KB))
        uint32_t    *memory_;

        // Register model
        ordt_root   *regs_;

        // Interrupt outputs
        bool        int_done_ = false;
        bool        int_err_ = false;

        // Program
        CpuProgram  *program_ = NULL;

        // Random value queue to be provided by GRV
        std::queue<uint256_t> grv_q_;

        // Private Key queues to be provided by GPK
        // queue is selected by immediate[2:0]
        std::queue<uint256_t> gpk_q_[8];

        // Enable for reporting of processor state changes
        bool        change_reporting_ = false;

        // Queue for processor state changes
        std::queue<dpi_state_change_t> change_q_;

    private:
        // true  - Instruction RAM (R/W from AHB), R from Core
        // false - Instruction ROM(no access from AHB), R from Core
        const bool instr_mem_rw_;

        uint32_t *MemToPtrs(CpuMemory mem, int *size);
        bool IsWithinMem(CpuMemory mem, uint16_t address);

        int ExecuteNextInstruction(int cycles);
};

#endif