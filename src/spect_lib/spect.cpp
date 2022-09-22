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

}