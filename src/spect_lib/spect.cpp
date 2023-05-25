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
    return os << "R" << std::dec << TO_INT(gpr);
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

std::ostream& operator << ( std::ostream& os, const spect::ParityType& parity_type)
{
    switch (parity_type) {
    case ParityType::ODD:
        return os << "Odd";
    case ParityType::EVEN:
        return os << "Even";
    case ParityType::NONE:
        return os << "None";
    default:
        return os << "Invalid parity type!";
    }
}

uint32_t stoint(std::string str)
{
    if (str.size() < 2) {
        return std::stoi (str, nullptr);
    } else if (str[0] == '0') {
        if (str[1] == 'x' || str[1] == 'X') {
            return std::stoi(str, nullptr, 16);
        } else if (str[1] == 'b' || str[1] == 'B') {
            return std::stoi(str, nullptr, 2);
        } else {
            return std::stoi(str, nullptr);
        }
    } else {
        return std::stoi(str, nullptr);
    }
}

std::string tohexs(uint64_t i, int width)
{
    std::stringstream ss;
    ss << std::hex << "0x" << std::setfill('0') << std::setw(width) << i;
    return ss.str();
}

std::string tohexs(uint256_t val)
{
    std::stringstream ss;
    ss << std::hex << "0x" << std::setfill('0') << std::setw(64) << val;
    return ss.str();
}

}
