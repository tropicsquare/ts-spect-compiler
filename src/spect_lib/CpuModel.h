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
extern "C" {
#include "KeccakSponge.h"
}

#include "ordt_pio_common.hpp"
#include "ordt_pio.hpp"

#include "spect_iss_dpi_types.h"

class spect::CpuModel
{
    public:

        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @section Control functions
        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief New CPU model constructor
        /// @param instr_mem_ahb_w Instruction memory writable via AHB
        /// @param instr_mem_ahb_r Instruction memory readable via AHB
        /// @returns New model object
        ///////////////////////////////////////////////////////////////////////////////////////////
        CpuModel(bool instr_mem_ahb_w, bool instr_mem_ahb_r);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief CPU model destructor
        ///////////////////////////////////////////////////////////////////////////////////////////
        ~CpuModel();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Start program execution
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Start();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Finish program execution
        /// @param status_err Value to be set in STATUS[ERR] bit.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Finish(int status_err);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Checks if program execution is finished (END instruction exectued)
        /// @return true - END has been executed
        ///         false - END has not been executed
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool IsFinished();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Execute multiple instructions of program
        /// @param n Number of instructions to execute
        /// @returns Number of actually executed instructions
        ///////////////////////////////////////////////////////////////////////////////////////////
        int  Step(int n);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Execute single instruction of program
        /// @param cycles Number of clock cycles it took to the actual HW to execute it.
        /// @returns 0 - Instruction executed, and 'cycles' is equal to 'cycles' during
        ///              previous execution of instruction with the same mnemonic.
        ///              Returned also if:
        ///                 1. Instruction with the same mnemonic is executed first time since
        ///                    model initialization.,
        ///                 2. The instruction was previously executed with 'cycles' = 0.
        ///                 3. Instruction has attribute 'c_time' set to false.
        ///          N - Number of instructions previous execution of instruction with the same
        ///              took.
        ///////////////////////////////////////////////////////////////////////////////////////////
        int  StepSingle(int cycles);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Reset the model
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Reset();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Update Interrupt outputs of model
        ///////////////////////////////////////////////////////////////////////////////////////////
        void UpdateInterrupts();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Update model based on access to memory registers
        ///////////////////////////////////////////////////////////////////////////////////////////
        void UpdateRegisterEffects();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Sets address of first instruction to be executed
        /// @param start_pc address of first instruction to be executed
        ///////////////////////////////////////////////////////////////////////////////////////////
        void SetStartPc(uint16_t start_pc);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Set data to model memory
        /// @param address Addresss to set
        /// @param data Data to set to the memory
        /// @note This function does not implement any memory access restrictions it "forces"
        ///       memory location to the 'data' argument.
        /// @note Model implements flat 16 bit of address space even in locations where real HW does
        ///       not have any memory. This function can use also this 'nonexistent' memory.
        /// @note This function does NOT report change to Report queue.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void SetMemory(uint16_t address, uint32_t data);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get data from model memory
        /// @param address Addresss to set
        /// @returns Data on the 'address' location
        /// @note This function does not implement any memory access restrictions it "gets" the
        ///       data regardless of memory being read/written.
        /// @note Model implements flat 16 bit of address space even in locations where real HW does
        //        not have any memory. This function can use also this 'nonexistent' memory.
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t GetMemory(uint16_t address);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get pointer to model memory
        /// @returns Pointer to first address of model memory
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t* GetMemoryPtr();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Read from the model memory space as-if via AHB bus
        /// @param address Addresss to read from
        /// @returns Data on the 'address' location
        /// @note This function implements AHB R/W restrictions as defined in SPECT specification
        ///         (e.g. Const ROM is not readable from AHB).
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t ReadMemoryAhb(uint16_t address);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Write from the model space as-if via AHB bus
        /// @param address Addresss to write
        /// @param data Data to be written
        /// @note This function implements AHB R/W restrictions as defined in SPECT specification
        ///         (e.g. Const ROM is not writable from AHB).
        /// @note This function reports change to Report queue.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void WriteMemoryAhb(uint16_t address, uint32_t data);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Read from model memory space as if done by Core data port (LD instruction)
        /// @param address Addresss to read from
        /// @returns Data on the 'address' location
        /// @note This function implements Read/Write restrictions as defined in SPECT specification
        ///         (e.g. Data RAM OUT is not readable by SPECT Core)
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t ReadMemoryCoreData(uint16_t address);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Write to model memory space as if done by Core data port (ST instruction)
        /// @param address Addresss to write
        /// @param data Data to write to the memory location
        /// @note This function implements Read/Write restrictions as defined in SPECT specification
        ///         (e.g. Const ROM is not writable by SPECT Core)
        /// @note This function reports change to Report queue.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void WriteMemoryCoreData(uint16_t address, uint32_t data);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Read from model memory space as if done by Core instruction port (fetch)
        /// @param address Addresss to fetch from
        /// @returns Value from 'address' memory location
        /// @note This function implements Read/Write restrictions as defined in SPECT specification
        ///         (e.g. attempt to fetch from CONST ROM will result in 0x0 being read)
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t ReadMemoryCoreFetch(uint16_t address);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Push value on RAR stack
        /// @param ret_addr Return address to be pushed onto a RAR stack
        /// @note Increments RAR_SP (RAR stack pointer) by 1
        ///////////////////////////////////////////////////////////////////////////////////////////
        void RarPush(uint16_t ret_addr);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Pop value from RAR stack
        /// @returns Value popped from RAR stack
        /// @note Decrements RAR_SP (RAR stack pointer) by 1;
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint16_t RarPop();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get RAR value at location
        /// @param index Pointer / Index from RAR stack to obtain
        /// @returns Value at 'ptr' index of 'rar_stack'
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint16_t GetRarAt(uint16_t index);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Push data to GRV Queue
        /// @param data Data to be pushed to TAIL of the queue.
        /// @note This function shall be used by outside world to push data to be returned by GRV
        //        instruction.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void GrvQueuePush(uint32_t data);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Pop data from GRV Queue
        /// @returns data from HEAD of the queue
        /// @note This function is used by instruction model when executing GRV instruction.
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t GrvQueuePop();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Push data to LDK Queue
        /// @param data Data to be pushed to TAIL of the queue.
        /// @note This function shall be used by outside world to push data to be returned by LDK
        //        instruction.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void LdkQueuePush(uint32_t data);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Pop data from LDK Queue
        /// @returns data from HEAD of the queue
        /// @note This function is used by instruction model when executing LDK instruction.
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t LdkQueuePop();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Push error flag to KBUS Error Queue
        /// @param error Error flag to be pushed to TAIL of the queue.
        /// @note This function shall be used by outside world to push error flag to be returned by
        //        KBUS transfers.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void KbusErrorQueuePush(bool error);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Pop data from KBUS Error Queue
        /// @returns error flag from HEAD of the queue
        /// @note This function is used by instruction model when executing LDK/STK/ERK instruction.
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool KbusErrorQueuePop();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Report Change on the CPU model
        /// @param change Change to be reported
        /// @note Change is reported only if change_reporting_ = true. Otherwise this call has no
        //        effect.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void ReportChange(dpi_state_change_t change);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Read (Consume) change from the model
        /// @returns Most recently reported change on the model which has not been reported yet.
        ///////////////////////////////////////////////////////////////////////////////////////////
        dpi_state_change_t ConsumeChange();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @returns True when there are changes to consume, False otherwise
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool HasChange();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Gets last executed instruction including sampled operands and filled results
        /// @param dpi_instr Instruction structure in which instruction will be filled
        /// @note This function shall be called after 'Step' or after 'StepSingle' to query last
        ///       executed instruction for functional coverage measurement. If this function is
        ///       queried without previous invocation of one of these functions, behavior of this
        ///       function is undefined.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void GetLastInstruction(dpi_instruction_t *dpi_instr);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Print debug message in the model
        /// @param verbosity_level Verbosity of the message
        ///         VERBOSITY_NONE      - Always shown
        ///         VERBOSITY_LOW       - Shown when verbosity is 1 or higher
        ///         VERBOSITY_MEDIUM    - Shown when verbosity is 2 or higher
        ///         VERBOSITY_HIGH      - Shown when verbosity is 3 or higher
        ///////////////////////////////////////////////////////////////////////////////////////////
        template<class... T>
        void DebugInfo(uint32_t verbosity_level, const T ...args);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Print current state of SHA512 unit context
        /// @param verbosity_level Verbosity of the message
        ///         VERBOSITY_NONE      - Always shown
        ///         VERBOSITY_LOW       - Shown when verbosity is 1 or higher
        ///         VERBOSITY_MEDIUM    - Shown when verbosity is 2 or higher
        ///         VERBOSITY_HIGH      - Shown when verbosity is 3 or higher
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrintHashContext(uint32_t verbosity_level);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Dump whole model (GPRs, Memory content, Hash unit context, RAR, Flags)
        /// @param path File where to dump Model context
        ///////////////////////////////////////////////////////////////////////////////////////////
        void DumpContext(const std::string &path);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Dump whole model (GPRs, Memory content, Hash unit content, RAR, Flags)
        /// @param path File where to dump Model context
        ///////////////////////////////////////////////////////////////////////////////////////////
        void LoadContext(const std::string &path);

        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @section Simple accessors
        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////

        // General purpose register (R0-R31) accessors
        const uint256_t& GetGpr(int index);
        void SetGpr(int index, const uint256_t &val);

        // Program counter (PC) accessors
        uint16_t GetPc();
        void SetPc(uint16_t val);

        // Flags (Z,C,E)
        void SetCpuFlag(CpuFlagType type, bool val);
        bool GetCpuFlag(CpuFlagType type);
        CpuFlags GetCpuFlags();

        // Secret Result Register (SRR)
        const uint256_t& GetSrr();
        void SetSrr(const uint256_t &val);

        // Return Address Register Stack Pointer
        uint16_t GetRarSp();
        void SetRarSp(uint16_t val);

        // Return address register
        void SetRarAt(uint16_t index, uint16_t val);

        // Get Interrupt output state
        bool GetInterrrupt(CpuIntType int_type);

        // Parity type
        void SetParityType(ParityType type);
        ParityType GetParityType();

        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @section Public attributes
        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////

        // Enable for reporting of processor state changes
        bool change_reporting_ = false;

        // Verbosity level of the model
        uint32_t verbosity_ = 0;

        // SHA512 calculation unit - Direct access
        Sha512 sha_512_;

        // Keccak Sponge instance
        KeccakWidth400_SpongeInstance keccak_inst_;

        // Program
        CpuProgram *program_ = NULL;

        // Simulator
        CpuSimulator *simulator_ = NULL;

        // Maximal number of instructions to execute
        uint64_t max_instr_cnt_ = std::numeric_limits<uint64_t>::max();

        // Number of already executed instructions since start
        uint64_t instr_cnt_ = 0;

        // Print function
        int (*print_fnc)(const char *format, ...);

    private:

        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// Core state
        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////

        // General Purpose registers (R0-R31)
        uint256_t gpr_[SPECT_GPR_CNT];

        // program Counter (PC)
        uint16_t pc_;

        // Flags (Z, C, E)
        CpuFlags flags_;

        // Return Address register (RAR) stack
        uint16_t rar_stack_[SPECT_RAR_DEPTH];

        // RAR stack pointer
        uint16_t rar_sp_;

        // Memory space (flat 16 bit space (64 KB))
        uint32_t* memory_;

        // Register model
        ordt_root *regs_;

        // Interrupt outputs
        bool int_done_ = false;
        bool int_err_ = false;

        // Random value queue, source of data provided by GRV instruction
        std::queue<uint32_t> grv_q_;

        // Private Key queue, source of data provided by LDK instruction
        std::queue<uint32_t> ldk_q_;

        // KBUS error queue, source of data provided by LDK instruction
        std::queue<bool> kbus_error_q_;

        // Queue for processor state changes
        std::queue<dpi_state_change_t> change_q_;

        // Instruction memory writable via AHB
        const bool instr_mem_ahb_w_;

        // Instruction memory readable via AHB
        const bool instr_mem_ahb_r_;

        // Parity check type
        ParityType parity_type_ = ParityType::NONE;

        ///////////////////////////////////////////////////////////////////////////////////////////
        // true  - Program has finished (END has been executed or Error occured)
        // false - Program was not started or it is running
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool end_executed_;

        // Address of first instruction to be executed
        uint16_t start_pc_ = SPECT_INSTR_MEM_BASE;

        // Last executed instruction
        dpi_instruction_t last_instr = {};

        uint32_t *MemToPtrs(CpuMemory mem, int *size);
        bool IsWithinMem(CpuMemory mem, uint16_t address);

        int ExecuteNextInstruction(int cycles);

        void PrintChange(dpi_state_change_t change);

        void PrintArgs();

        template<typename Arg>
        void PrintArgs(Arg arg);

        template<typename First, typename... Args>
        void PrintArgs(First first, Args... args);
};

#endif
