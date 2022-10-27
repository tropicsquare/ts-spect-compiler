/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <fstream>
#include <iostream>

#include "SourceFile.h"


spect::SourceFile::SourceFile(const std::string &path, uint32_t first_addr) :
    first_addr_(first_addr),
    path_(path)
{
    std::ifstream ifs;
    ifs.open(path);
    if (ifs.is_open()) {
        std::string line;
        while (std::getline(ifs, line))
            lines_.push_back(line);
    } else {
        throw std::runtime_error("Unable to open a file: " + path);
    }
    ifs.close();
}
