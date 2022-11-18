/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef SPECT_LIB_SYMBOL_TABLE_H_
#define SPECT_LIB_SYMBOL_TABLE_H_

#include <map>
#include <string>

#include "Instruction.h"

class spect::SymbolTable
{
    public:
        SymbolTable();
        ~SymbolTable();
        Symbol* AddSymbol(std::string identifier, spect::SymbolType type, int line_nr);
        Symbol* AddSymbol(std::string identifier, spect::SymbolType type, uint32_t val, int line_nr);
        void ResolveSymbol(spect::Symbol *s, spect::SymbolType type, uint32_t val);
        Symbol* GetSymbol(const std::string &identifier);
        Symbol* GetSymbol(const uint32_t val, spect::SymbolType type);
        bool IsDefined(const std::string &identifier);
        void Dump(std::ostream& os);
        void Print(std::ostream& os);
        spect::SourceFile *curr_file_;

    private:
        std::map<std::string, spect::Symbol*> symbol_map_;
};

#endif