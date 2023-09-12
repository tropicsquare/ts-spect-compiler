/**************************************************************************************************
**
**
** TODO: License
**
** Author: Marek Santa
**************************************************************************************************/

#ifndef SPECT_LIB_KEY_MEMORY_H_
#define SPECT_LIB_KEY_MEMORY_H_

#include <vector>
#include <iostream>

class spect::KeyMemory
{
    public:
        // Slot status
        enum SlotStatus {
            EMPTY,
            FULL
        };

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief New Key Memory constructor
        /// @returns New model object
        ///////////////////////////////////////////////////////////////////////////////////////////
        KeyMemory();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Key Memory destructor
        ///////////////////////////////////////////////////////////////////////////////////////////
        ~KeyMemory();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get Key Memory
        /// @param type Key type
        /// @param slot Slot in memory
        /// @param offset Offset within the slot
        /// @returns Read data.
        ///////////////////////////////////////////////////////////////////////////////////////////
        uint32_t Get(uint8_t type, uint8_t slot, uint8_t offset);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Set Key Memory
        /// @param type Key type
        /// @param slot Slot in memory
        /// @param offset Offset within the slot
        /// @param data Data to be written
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Set(uint8_t type, uint8_t slot, uint8_t offset, uint32_t data);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Read Key Memory
        /// @param type Key type
        /// @param slot Slot in memory
        /// @param offset Offset within the slot
        /// @param data Read data
        /// @returns Error flag (0 - no error, else error)
        ///////////////////////////////////////////////////////////////////////////////////////////
        int Read(uint32_t type, uint32_t slot, uint32_t offset, uint32_t &data);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Write RAM Buffer
        /// @param offset Offset within the slot
        /// @param data Data to be written
        /// @returns Error flag (0 - no error, else error)
        ///////////////////////////////////////////////////////////////////////////////////////////
        int Write(uint32_t offset, uint32_t data);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Program Key Memory
        /// @param type Key type
        /// @param slot Slot in memory
        /// @returns Error flag (0 - no error, else error)
        ///////////////////////////////////////////////////////////////////////////////////////////
        int Program(uint32_t type, uint32_t slot);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Erase Key Memory
        /// @param type Key type
        /// @param slot Slot in memory
        /// @returns Error flag (0 - no error, else error)
        ///////////////////////////////////////////////////////////////////////////////////////////
        int Erase(uint32_t type, uint32_t slot);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Verify Erase of Key Memory
        /// @param type Key type
        /// @param slot Slot in memory
        /// @returns Error flag (0 - no error, else error)
        ///////////////////////////////////////////////////////////////////////////////////////////
        int VerifyErase(uint32_t type, uint32_t slot);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Flush RAM Buffer
        /// @returns Error flag (0 - no error, else error)
        ///////////////////////////////////////////////////////////////////////////////////////////
        int Flush();

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Dump Key Memory
        /// @param path File where to dump memory content
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Dump(const std::string &path);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Load Key Memory
        /// @param path File from where to load memory content
        ///////////////////////////////////////////////////////////////////////////////////////////
        void Load(const std::string &path);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Print debug message in the model
        /// @param verbosity_level Verbosity of the message
        ///         VERBOSITY_NONE      - Always shown
        ///         VERBOSITY_LOW       - Shown when verbosity is 1 or higher
        ///         VERBOSITY_MEDIUM    - Shown when verbosity is 2 or higher
        ///         VERBOSITY_HIGH      - Shown when verbosity is 3 or higher
        ///////////////////////////////////////////////////////////////////////////////////////////
        template<class... T>
        void DebugInfo(uint32_t verbosity_level, const T ...args);

        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @section Public attributes
        ///////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////

        // Verbosity level of the model
        uint32_t verbosity_ = 0;

    private:

        // Key memory
        uint32_t key_mem_[KEY_MEM_TYPE_NUM][KEY_MEM_SLOT_NUM][KEY_MEM_OFFSET_NUM];

        // RAM Buffer
        uint32_t ram_buffer_[KEY_MEM_OFFSET_NUM];

        // Slot Status
        SlotStatus slot_status_[KEY_MEM_TYPE_NUM][KEY_MEM_SLOT_NUM];

        void PrintArgs();

        template<typename Arg>
        void PrintArgs(Arg arg);

        template<typename First, typename... Args>
        void PrintArgs(First first, Args... args);
};

#endif