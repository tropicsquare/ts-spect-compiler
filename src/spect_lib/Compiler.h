/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef SPECT_LIB_COMPILER_H_
#define SPECT_LIB_COMPILER_H_

#include <iostream>

#include "Instruction.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "CpuProgram.h"
#include "SourceFile.h"

class spect::Compiler
{
    public:

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief New Compiler constructor
        /// @returns New model object
        ///////////////////////////////////////////////////////////////////////////////////////////
        Compiler(void);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Compiler destructor
        ///////////////////////////////////////////////////////////////////////////////////////////
        ~Compiler();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Compiler destructor
        /// @param first_addr Address where compiler shall place first instruction of first
        ///                   processed assembly file.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void CompileInit(uint32_t first_addr);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Compile program
        /// @param path Path to SPECT .s file
        /// @throw std::runtime_error upon compilation error
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Compile(std::string path);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Finalize compilation (to be called after 'Compile'). Only prints stas of
        ///        compiled program.
        /// @returns 0 - When program contained '_start' symbol, 1 otherwise.
        ///////////////////////////////////////////////////////////////////////////////////////////
        int CompileFinish();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Adds conditional define
        ///////////////////////////////////////////////////////////////////////////////////////////
        void CondDefAdd(std::string ident);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Checks if conditional define exists
        /// @returns true if "ioent" is existing conditional define
        ///////////////////////////////////////////////////////////////////////////////////////////
        bool CondDefExists(std::string ident);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Throw compiler error
        /// @param err Error message to print
        /// @param err_code Error code to throw
        /// @throw std::runtime_error
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Error(std::string err, ErrCode err_code);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Throw compiler error with reference to source file location
        /// @param err Error message to print
        /// @param sf Pointer to source file where error occured.
        /// @param line_nr Line number where error occured.
        /// @param err_code Error code to throw
        /// @throw std::runtime_error
        ///////////////////////////////////////////////////////////////////////////////////////////
        void ErrorAt(std::string err, const SourceFile *sf, int line_nr, ErrCode err_code);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Throw compiler warning
        /// @param warn Warning message to print
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Warning(std::string warn);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Throw compiler error with reference to source file location
        /// @param warn Warning message to print
        /// @param sf Pointer to source file where error occured.
        /// @param line_nr Line number where error occured.
        ///////////////////////////////////////////////////////////////////////////////////////////
        void WarningAt(std::string warn, const SourceFile *sf, int line_nr);

        // Pointer to symbol table
        spect::SymbolTable *symbols_;

        // Pointer to compiled program
        spect::CpuProgram *program_ = nullptr;

        // Pointer to source files (.s) used for compilation
        std::map<std::string, spect::SourceFile*> files_;

        // Address where first compiled instruction is placed (set by constructor)
        uint32_t first_addr_;

        // Address of last compiled instruction (increments during 'Compile')
        uint32_t curr_addr_;

        // Number of instructions compiler compiled.
        int num_instr_ = 0;

        // Print function - By default 'printf'
        int (*print_fnc)(const char *format, ...);

    private:
        void TrimSpaces(std::string &input);
        uint32_t ParseValue(spect::SourceFile *sf, int line_nr, const std::string &val,
                            spect::Symbol* &s, uint32_t limit = 0);
        void ParseArgument(spect::SourceFile *sf, int line_nr, spect::Instruction* instr,
                            std::string &arg, int arg_index);
        spect::Symbol* ParseLabel(spect::SourceFile *sf, std::string &line_buf, int line_nr);
        bool ParseConstant(spect::SourceFile *sf, std::string &line_buf, int line_nr);
        bool ParseIncludeFile(spect::SourceFile *sf, std::string &line_buf);
        spect::Instruction* ParseInstruction(spect::SourceFile *sf, std::string &line_buf,
                                             int line_nr,  spect::Symbol *label);
        spect::CpuGpr ParseOp(spect::SourceFile *sf, int line_nr, std::string &arg);
        bool ParseCondCompile(spect::SourceFile *sf, std::string &line_buf, int line_nr);

        std::list<bool> cond_stack_;
        std::vector<std::string> cond_defs_;
        bool ShouldParse(void);
};

#endif