/**************************************************************************************************
** 
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef SPECT_LIB_HEX_HANDLER_H_
#define SPECT_LIB_HEX_HANDLER_H_

#include <string.h>

#include "spect.h"

class spect::HexHandler
{
    public:
        static void LoadHexFile(const std::string &path, uint32_t *mem);

    private:
};

#endif