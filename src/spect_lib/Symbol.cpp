/**************************************************************************************************
** 
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include "Symbol.h"

spect::Symbol::Symbol(std::string identifier, spect::SymbolType type, spect::SourceFile *f, int line_nr) :
    identifier_(identifier),
    type_(type),
    val_(0),
    resolved_(false),
    f_(f),
    line_nr_(line_nr)
{}

spect::Symbol::Symbol(std::string identifier, spect::SymbolType type, uint32_t val, spect::SourceFile *f, int line_nr) :
    identifier_(identifier),
    type_(type),
    val_(val),
    resolved_(true),
    f_(f),
    line_nr_(line_nr) 
{}

std::ostream& operator << ( std::ostream& os, const spect::Symbol& sym)
{
    os << sym.identifier_ << ":";
    if (sym.resolved_)
        os << "     " << sym.val_;
    else
        os << "     undefined";
    return os;
}