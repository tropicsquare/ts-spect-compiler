/******************************************************************************
*
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
*
* @license For the license see file LICENSE.txt file in the root directory of this source tree.
*
*
*****************************************************************************/

#ifndef SPECT_LIB_SYMBOL_H_
#define SPECT_LIB_SYMBOL_H_

#include <iostream>

#include "spect.h"

class spect::Symbol
{
    public:
        Symbol(std::string identifier, spect::SymbolType type, spect::SourceFile *f, int line_nr);
        Symbol(std::string identifier, spect::SymbolType type, uint32_t val, spect::SourceFile *f, int line_nr);
        friend std::ostream& operator << ( std::ostream& os, const spect::Symbol& sym); 

        std::string identifier_;
        spect::SymbolType type_;
        uint32_t val_;
        bool resolved_;
        spect::SourceFile *f_;
        int line_nr_;

};

#endif