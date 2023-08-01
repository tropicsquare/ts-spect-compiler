/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef SPECT_LIB_CPU_SIMULATOR_H_
#define SPECT_LIB_CPU_SIMULATOR_H_

#include <vector>
#include <iostream>

#include "Instruction.h"

#include "cli.h"
#include "clilocalsession.h"
#include "loopscheduler.h"

class spect::CpuSimulator
{
    public:

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief New CPU simulator constructor
        /// @returns New model object
        ///////////////////////////////////////////////////////////////////////////////////////////
        CpuSimulator();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief CPU Simulator destructor
        ///////////////////////////////////////////////////////////////////////////////////////////
        ~CpuSimulator();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Add breakpoint
        /// @param address Address in program where to add breakpoint
        /// @returns True - Breakpoint added, False if breakpoint already exists
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool AddBreakpoint(uint16_t address);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Add breakpoint
        /// @param label Symbol label at which to add breakpoint
        /// @returns True - Breakpoint added
        //           False - Breakpoint already exists, or symbol 'label' not found
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool AddBreakpoint(std::string label);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Remove breakpoint
        /// @param address Address in program from which to remove the breakpoint
        /// @returns True - When breakpoint was erased, False when no breakpoint exist at address.
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool RemoveBreakPoint(uint32_t address);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Remove breakpoint
        /// @param label Symbol label at which to add breakpoint
        /// @returns True - Breakpoint removed
        //           False - No such breakpoint or symbol with 'label' exist
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool RemoveBreakPoint(std::string label);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @returns True - When there exists brekapoint at address
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool IsBreakpointAt(uint32_t address);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Prints all breakpoints
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrintBreakpoints();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Prints single breakpoint
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrintBreakpoint(uint32_t breakpoint);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Prints GPR registers
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrintGprRegisters();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Prints program counter
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrintPc();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Prints CPU Flags
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrintFlags();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Prints Contents of Return Address register stack
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrintRar();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Prints Symbol Table
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrintSymbols();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Starts the CPU simulator
        /// @param batch_mode True  - Start in batch mode
        ///                   False - Start in interactive mode
        /// @param context CPU context file to start the simulation with
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Start(bool batch_mode);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Starts the CPU simulator
        /// @param path Path to command file
        ///////////////////////////////////////////////////////////////////////////////////////////
        void ExecCmdFile(std::string path);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @returns True if program has already finished
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool CheckFinished();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @returns True if program is running
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool CheckRunning();

        // Reference to CPU model executing the code
        CpuModel *model_;

        // Reference to compiler which compiled this program
        Compiler *compiler_;

        // Reference to key memory
        KeyMemory *key_memory_;

        // Objects handling CLI and interactive simulation
        cli::Cli *cli_;

        // Path to CPU model context file to be preloaded before model execution
        std::string model_context_;

        // Path to command file for simulator
        std::string cmd_file_;

    private:

        // Functions to execute individual commands
        void CmdInfo(std::ostream &out, std::string arg1);
        void CmdBreak(std::ostream &out, std::string arg1);
        void CmdRun(std::ostream &out);
        void CmdDelete(std::ostream &out, std::string arg1, bool all);
        void CmdJump(std::ostream &out, std::string arg1);
        void CmdStep(std::ostream &out, int n);
        void CmdStart(std::ostream &out);
        void CmdSet(std::ostream &out, std::string arg1, std::string arg2);
        void CmdGet(std::ostream &out, std::string arg1);
        void CmdLoad(std::ostream &out, std::string arg1, uint32_t offset);
        void CmdDump(A_UNUSED std::ostream &out, std::string arg1, uint32_t address, uint32_t size);

        // Array of breakpoints break-points
        std::vector<uint32_t> breakpoints_;

        // Indication model execution is in progress
        bool program_running_ = false;

        // Create commands fo interactive CLI
        void BuildCliCommands(std::unique_ptr<cli::Menu> &menu);

        // Execute command file
        void ExecCmdFile(cli::CliLocalTerminalSession &session);

};

#endif
