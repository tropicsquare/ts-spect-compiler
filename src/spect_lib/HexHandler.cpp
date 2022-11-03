/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <string.h>
#include <sstream>
#include <fstream>
#include <iostream>

#include "spect.h"
#include "HexHandler.h"


void spect::HexHandler::LoadHexFile(const std::string &path, uint32_t *mem)
{
    std::ifstream ifs;
    ifs.open(path);
    if (ifs.is_open()) {
        std::string line;
        uint32_t *mem_c = mem;
        while (std::getline(ifs, line)) {
            uint32_t val;

            // Absolute address in the HEX file, +0x4 offset
            if (line.size() > 0 && line[0] == '@') {
                uint32_t addr;
                line.erase(0,1);
                std::istringstream iss(line);
                if (!(iss >> std::hex >> addr >> val)) {
                    throw std::runtime_error("Unable to read line:" + line + " from file: " + path);
                }
                mem_c[addr >> 2] = val;

            // No address in hex file
            } else {
                std::istringstream iss(line);
                if (!(iss >> val)) {
                    throw std::runtime_error("Unable to read line:" + line + " from file: " + path);
                }
                *mem_c = val;
                mem_c++;
            }
        }
    } else {
        throw std::runtime_error("Unable to open a file:" + path);
    }
}
