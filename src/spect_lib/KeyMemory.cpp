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
{
    // Initialize Key Memory to all 1s and Slot Status to empty
    for (uint32_t type = 0; type < KEY_MEM_TYPE_NUM; type++) {
        for (uint32_t slot = 0; slot < KEY_MEM_SLOT_NUM; slot++) {
            slot_status_[type][slot] = SlotStatus::EMPTY;
            for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++) {
                key_mem_[type][slot][offset] = 0xFFFFFFFF;
            }
        }
    }
}

spect::KeyMemory::~KeyMemory()
{}

uint32_t spect::KeyMemory::Get(uint8_t type, uint8_t slot, uint8_t offset)
{
    return key_mem_[type][slot][offset];
}

void spect::KeyMemory::Set(uint8_t type, uint8_t slot, uint8_t offset, uint32_t data)
{
    key_mem_[type][slot][offset] = data;
    slot_status_[type][slot] = SlotStatus::FULL;
}

int spect::KeyMemory::Read(uint32_t type, uint32_t slot, uint32_t offset, uint32_t &data)
{
    DebugInfo(VERBOSITY_HIGH, "Reading Key Memory type", type, ", slot", tohexs(slot, 4), "and offset", tohexs(offset, 4));
    if (slot_status_[type][slot] == SlotStatus::EMPTY) {
        DebugInfo(VERBOSITY_HIGH, "Key Memory type", type, "and slot", tohexs(slot, 4), "is empty, reading failed");
        return 1;
    }

    for (uint32_t ofst = 0; ofst < KEY_MEM_OFFSET_NUM; ofst++)
        ram_buffer_[ofst] = key_mem_[type][slot][ofst];
    data = ram_buffer_[offset];
    return 0;
}

int spect::KeyMemory::Write(uint32_t offset, uint32_t data)
{
    DebugInfo(VERBOSITY_HIGH, "Writing RAM Buffer[", tohexs(offset, 4), "] =", tohexs(data, 8));
    ram_buffer_[offset] = data;
    return 0;
}

int spect::KeyMemory::Program(uint32_t type, uint32_t slot)
{
    DebugInfo(VERBOSITY_HIGH, "Erasing Key Memory type", type, "and slot", tohexs(slot, 4));
    if (slot_status_[type][slot] == SlotStatus::FULL) {
        DebugInfo(VERBOSITY_HIGH, "Key Memory type", type, "and slot", slot, "is full, programming failed");
        return 1;
    }

    for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++)
        key_mem_[type][slot][offset] = ram_buffer_[offset];
    slot_status_[type][slot] = SlotStatus::FULL;
    return 0;
}

int spect::KeyMemory::Erase(uint32_t type, uint32_t slot)
{
    DebugInfo(VERBOSITY_HIGH, "Erasing Key Memory type", type, "and slot", tohexs(slot, 4));
    for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++) {
        key_mem_[type][slot][offset] = 0xFFFFFFFF;
    }
    slot_status_[type][slot] = SlotStatus::EMPTY;
    return 0;
}

int spect::KeyMemory::VerifyErase(uint32_t type, uint32_t slot)
{
    DebugInfo(VERBOSITY_HIGH, "Verifying erase of Key Memory type", type, "and slot", tohexs(slot, 4));
    for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++) {
        if (key_mem_[type][slot][offset] != 0xFFFFFFFF) {
            DebugInfo(VERBOSITY_HIGH, "Verifying erase of Key Memory failed");
            return 1;
        }
    }
    return 0;
}

int spect::KeyMemory::Flush()
{
    DebugInfo(VERBOSITY_HIGH, "Flushing RAM Buffer");
    for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++) {
        ram_buffer_[offset] = rand();
    }
    return 0;
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
        DebugInfo(VERBOSITY_LOW, "Dumping Key Memory to: ", path);

        ofs << std::hex;
        ofs << std::setfill('0');

        PUT_COMMENT_LINE("Key Memory:");
        for (uint32_t type = 0; type < KEY_MEM_TYPE_NUM; type++) {
            for (uint32_t slot = 0; slot < KEY_MEM_SLOT_NUM; slot++) {
                ss << "Type: " << type << " Slot: " << slot;
                PUT_COMMENT_LINE(ss.str());
                ofs << "Status: " << ((slot_status_[type][slot] == SlotStatus::EMPTY) ? "EMPTY" : "FULL") << "\n";
                if (slot_status_[type][slot] == SlotStatus::FULL) {
                    for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++) {
                      ofs << std::setw(8) << key_mem_[type][slot][offset] << "\n";
                    }
                }
                ss.str("");
            }
        }

        DebugInfo(VERBOSITY_LOW, "Finished dumping Key Memory.");
        DebugInfo(VERBOSITY_LOW, "\n");

        ofs.close();
    } else
        throw std::runtime_error("Unable to open a file: " + path);
}

void spect::KeyMemory::Load(const std::string &path)
{
    std::ifstream ifs(path);
    std::string line;

    if (ifs.is_open()) {
        DebugInfo(VERBOSITY_LOW, "Loading Key Memory to: ", path);

        // Key Memory
        SKIP_COMMENT_LINES
        for (uint32_t type = 0; type < KEY_MEM_TYPE_NUM; type++) {
            for (uint32_t slot = 0; slot < KEY_MEM_SLOT_NUM; slot++) {
                SKIP_COMMENT_LINES
                std::getline(ifs, line);
                std::string i_str = line.substr(8, line.size() - 1);
                slot_status_[type][slot] = (i_str == "EMPTY") ? SlotStatus::EMPTY : SlotStatus::FULL;
                if (slot_status_[type][slot] == SlotStatus::FULL) {
                    for (uint32_t offset = 0; offset < KEY_MEM_OFFSET_NUM; offset++) {
                        std::getline(ifs, line);
                        uint32_t num;
                        std::istringstream iss(line);
                        iss >> std::hex >> num;
                        key_mem_[type][slot][offset] = num;
                    }
                }
            }
        }

        DebugInfo(VERBOSITY_LOW, "Finished loading Key Memory.");
        DebugInfo(VERBOSITY_LOW, "\n");

        ifs.close();
    } else
        throw std::runtime_error("Unable to open a file: " + path);
}

void spect::KeyMemory::PrintArgs()
{
    printf("\n");
}

template<typename Arg>
void spect::KeyMemory::PrintArgs(Arg arg)
{
    std::stringstream ss;
    ss << std::hex << arg << std::endl;
    printf(ss.str().c_str());
}

template<typename First, typename... Args>
void spect::KeyMemory::PrintArgs(First first, Args... args)
{
    std::stringstream ss;
    ss << std::hex << first << " ";
    printf(ss.str().c_str());
    PrintArgs(args...);
}

template<typename... Args>
void spect::KeyMemory::DebugInfo(uint32_t verbosity_level, const Args ...args)
{
    if (verbosity_ >= verbosity_level)
        PrintArgs(MODEL_LABEL, args...);
}

