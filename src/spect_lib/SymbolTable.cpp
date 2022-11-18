/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <cassert>

#include "Symbol.h"
#include "SymbolTable.h"

spect::SymbolTable::SymbolTable()
{}

spect::SymbolTable::~SymbolTable()
{
    for (const auto &sym : symbol_map_)
        delete sym.second;
}

spect::Symbol* spect::SymbolTable::AddSymbol(std::string identifier, spect::SymbolType type, int line_nr)
{
    std::cout << "Adding symbol: " << identifier << std::endl;
    symbol_map_[identifier] = new spect::Symbol(identifier, type, curr_file_, line_nr);
    return symbol_map_[identifier];
}

spect::Symbol* spect::SymbolTable::AddSymbol(std::string identifier, spect::SymbolType type, uint32_t val, int line_nr)
{
    std::cout << "Adding symbol: " << identifier << "(" << val << ")" << std::endl;
    symbol_map_[identifier] = new spect::Symbol(identifier, type, val, curr_file_, line_nr);
    return symbol_map_[identifier];
}

void spect::SymbolTable::ResolveSymbol(spect::Symbol *s, spect::SymbolType type, uint32_t val)
{
    assert (!s->resolved_);

    std::cout << "Resolving symbol: " << s->identifier_ << "(" << val << ")" << std::endl;
    s->f_ = curr_file_;
    s->resolved_ = true;
    s->type_ = type;
    s->val_ = val;
}

bool spect::SymbolTable::IsDefined(const std::string &identifier)
{
    auto it = symbol_map_.find(identifier);
    if (it == symbol_map_.end())
        return false;
    return true;
}

spect::Symbol* spect::SymbolTable::GetSymbol(const std::string &identifier)
{
    if (IsDefined(identifier))
        return symbol_map_[identifier];
    return nullptr;
}

spect::Symbol* spect::SymbolTable::GetSymbol(const uint32_t val, spect::SymbolType type)
{
    for (const auto &elem : symbol_map_) {
        Symbol *s = (Symbol*)elem.second;
        if (s->val_ == val && s->type_ == type)
            return s;
    }
    return nullptr;
}

void spect::SymbolTable::Dump(std::ostream& os)
{
    for (const auto &elem : symbol_map_) {
        Symbol *s = (Symbol*)elem.second;
        os << elem.first << std::hex << " .eq 0x" << s->val_;
        os << "     ; " << s->type_ << "\n";
    }
}

void spect::SymbolTable::Print(std::ostream& os)
{
    using namespace std;

    os << hex << setw(4);
    for (const auto &elem : symbol_map_) {
        Symbol *s = (Symbol*)elem.second;
        os << "    ";
        os << left  << setw(15) << elem.first << " = ";
        stringstream ss;
        ss << "0x" << hex << s->val_;
        os << left << setw(10) << ss.str();
        os << " - " << s->type_;
        os << "\n";
    }
}