/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef SPECT_LIB_CPU_PROGRAM_H_
#define SPECT_LIB_CPU_PROGRAM_H_

#include <vector>
#include <iostream>

#include "Instruction.h"

class spect::CpuProgram
{
    public:
        CpuProgram(size_t expected_size);
        void AppendInstruction(spect::Instruction *instr);
        void Assemble(uint32_t *mem);
        void Assemble(std::string path);
        void Dump(std::ostream &os);
        //void DisAssemble(uint32_t *mem, size_t len);

        uint32_t first_addr_;
        Compiler *compiler_;

    private:
        std::vector<spect::Instruction*> code_;

};

#endif