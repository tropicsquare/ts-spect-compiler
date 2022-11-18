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


void spect::HexHandler::LoadHexFile(const std::string &path, uint32_t *mem, uint32_t offset)
{
    std::ifstream ifs;
    ifs.open(path);
    if (ifs.is_open()) {
        std::string line;
        bool first_line = true;
        uint32_t *mem_c = mem;
        while (std::getline(ifs, line)) {
            uint32_t val;

            // On first line figure out type of HEX file (+0x4 addressed, or non-addressed)
            // take into acount offset for non-addressed HEX-file
            if (first_line) {
                if (line[0] != '@')
                    mem_c += (offset >> 2);
                first_line = false;
            }

            // Absolute address in the HEX file, +0x4 offset
            if (line.size() > 0 && line[0] == '@') {
                uint32_t addr;
                line.erase(0,1);
                std::istringstream iss(line);
                if (!(iss >> std::hex >> addr >> val))
                    throw std::runtime_error("Unable to read line:" + line + " from file: " + path + "\n");
                mem_c[addr >> 2] = val;

            // No address in hex file
            } else {
                std::istringstream iss(line);
                if (!(iss >> std::hex >> val))
                    throw std::runtime_error("Unable to read line:" + line + " from file: " + path + "\n");
                *mem_c = val;
                mem_c++;
            }
        }
    } else {
        throw std::runtime_error("Unable to open a file:" + path);
    }
}
