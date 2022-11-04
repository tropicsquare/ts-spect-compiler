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
        Compiler(uint32_t first_addr);
        ~Compiler();

        void Compile(std::string path);
        int CompileFinish();

        spect::SymbolTable *symbols_;
        spect::CpuProgram *program_;
        std::map<std::string, spect::SourceFile*> files_;

        uint32_t first_addr_;
        uint32_t curr_addr_;

        int num_instr_ = 0;

        // Print function
        int (*print_fnc)(const char *format, ...);

    private:
        void TrimSpaces(std::string &input);
        uint32_t ParseValue(spect::SourceFile *sf, int line_nr, const std::string &val, spect::Symbol* &s, uint32_t limit = 0);
        void ParseArgument(spect::SourceFile *sf, int line_nr, spect::Instruction* instr, std::string &arg, int arg_index);
        spect::Symbol* ParseLabel(spect::SourceFile *sf, std::string &line_buf, int line_nr);
        bool ParseConstant(spect::SourceFile *sf, std::string &line_buf, int line_nr);
        bool ParseIncludeFile(spect::SourceFile *sf, std::string &line_buf);
        spect::Instruction* ParseInstruction(spect::SourceFile *sf, std::string &line_buf, int line_nr,  spect::Symbol *label);
        spect::CpuGpr ParseOp(spect::SourceFile *sf, int line_nr, std::string &arg);

    public:
        void Error(std::string err);
        void ErrorAt(std::string err, const SourceFile *sf, int line_nr);
        void Warning(std::string warn);
        void WarningAt(std::string warn, const SourceFile *sf, int line_nr);
};

#endif