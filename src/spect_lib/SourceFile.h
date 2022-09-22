/**************************************************************************************************
** 
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef SPECT_LIB_SOURCE_FILE_H_
#define SPECT_LIB_SOURCE_FILE_H_

#include <vector>
#include <string>

#include "spect.h"


class spect::SourceFile
{
    public:
        uint32_t first_addr_;
        std::string path_;
        SourceFile(const std::string &path, uint32_t first_addr);
        std::vector<std::string> lines_;
};

#endif