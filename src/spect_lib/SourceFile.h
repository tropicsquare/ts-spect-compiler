/******************************************************************************
*
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
*
* @license For the license see file LICENSE.txt file in the root directory of this source tree.
*
*
*****************************************************************************/

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