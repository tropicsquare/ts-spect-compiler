/**************************************************************************************************
**
**
** TODO: License
**
** Author: Marek Santa
**************************************************************************************************/

#include <regex>
#include <fstream>
#include <iostream>

#include "spect.h"
#include "KeyMemory.h"

spect::KeyMemory::KeyMemory()
{}

spect::KeyMemory::~KeyMemory()
{}

uint32_t spect::KeyMemory::Get(uint8_t type, uint8_t slot, uint8_t offset)
{
    return key_mem_[type][slot][offset];
}

void spect::KeyMemory::Set(uint8_t type, uint8_t slot, uint8_t offset, uint32_t data)
{
    key_mem_[type][slot][offset] = data;
}

uint32_t spect::KeyMemory::Read(uint32_t type, uint32_t slot, uint32_t offset)
{
    return key_mem_[type][slot][offset];
}

void spect::KeyMemory::Write(uint32_t type, uint32_t slot, uint32_t offset, uint32_t data)
{
    key_mem_[type][slot][offset] &= data;
}

void spect::KeyMemory::Erase(uint32_t type, uint32_t slot)
{
    for (uint32_t i = 0; i < KEY_MEM_OFFSET_NUM; i++) {
      key_mem_[type][slot][i] = 0xFFFFFFFF;
    }
}

#define PUT_COMMENT_LINE(comment)           \
    ofs << std::string(80, '*') << "\n";    \
    ofs << comment << "\n";                 \
    ofs << std::string(80, '*') << "\n";

#define SKIP_COMMENT_LINES          \
    for (int i = 0; i < 3; i++)     \
        std::getline(ifs, line);

void spect::KeyMemory::Dump(const std::string &path)
{
    std::ofstream ofs;
    std::stringstream ss;
    ofs.open(path);

    if (ofs.is_open()) {
        std::cout << MODEL_LABEL << " Dumping Key Memory to: " << path << "\n";

        ofs << std::hex;
        ofs << std::setfill('0');

        PUT_COMMENT_LINE("Key Memory:");
        for (uint32_t type = 0; type < KEY_MEM_TYPE_NUM; type++) {
            for (uint32_t slot = 0; slot < KEY_MEM_SLOT_NUM; slot++) {
                ss << "Type: " << type << " Slot: " << slot;
                PUT_COMMENT_LINE(ss.str());
                for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++) {
                  ofs << std::setw(8) << key_mem_[type][slot][offset] << "\n";
                }
                ss.str("");
            }
        }

        std::cout << MODEL_LABEL << " Finished dumping Key Memory.\n";
        std::cout << MODEL_LABEL << " \n";

        ofs.close();
    } else
        throw std::runtime_error("Unable to open a file: " + path);
}

void spect::KeyMemory::Load(const std::string &path)
{
    std::ifstream ifs(path);
    std::string line;

    if (ifs.is_open()) {
        std::cout << MODEL_LABEL << " Loading Key Memory from: " << path << "\n";

        // Key Memory
        SKIP_COMMENT_LINES
        for (uint32_t type = 0; type < KEY_MEM_TYPE_NUM; type++) {
            for (uint32_t slot = 0; slot < KEY_MEM_SLOT_NUM; slot++) {
                SKIP_COMMENT_LINES
                for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++) {
                    std::getline(ifs, line);
                    uint32_t num;
                    std::istringstream iss(line);
                    iss >> std::hex >> num;
                    key_mem_[type][slot][offset] = num;
                }
            }
        }

        std::cout << MODEL_LABEL << " Finished loading Key Memory.\n";
        std::cout << MODEL_LABEL << " \n";

        ifs.close();
    } else
        throw std::runtime_error("Unable to open a file: " + path);
}

