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

#include "spect.h"

namespace spect {

std::ostream& operator << ( std::ostream& os, const spect::CpuGpr& gpr)
{
    // TODO: Handle when stream is formatted as non-decimal base!
    return os << "R" << TO_INT(gpr);
}

std::ostream& operator << ( std::ostream& os, const spect::SymbolType& symbol_type)
{
    switch (symbol_type) {
    case SymbolType::LABEL:
        return os << "Label";
    case SymbolType::CONSTANT:
        return os << "Constant";
    case SymbolType::UNKNOWN:
        return os << "Unknown";
    default:
        return os << "Invalid symbol type!";
    }
}

}