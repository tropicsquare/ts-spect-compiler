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
        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Loads HEX file
        /// @param path Path to HEX file to be loaded
        /// @param mem Pointer to memory where to load the HEX file.
        /// @param offset Offset from 'mem' (in byte addressing) where start loading HEX file:
        ///         Interpretation depends on HEX file format:
        ///                 DPI_HEX_ISS_WORD -
        ///                      Has no effect
        ///                 DPI_HEX_VERILOG_RAW_WORD -
        ///                      Set to base address of the memory that you want to
        ///                      preload (obtained via spect_dpi_get_mem_base).
        ///                      This is usefull if you want to preload constant ROM
        ///                      by HEX file which only contains constants, but not
        ///                      their addresses. The same hex file can be loaded
        /// @throw std::runtime_error when failed to open the file, or when it has invalid format
        ///////////////////////////////////////////////////////////////////////////////////////////
        static void LoadHexFile(const std::string &path, uint32_t *mem, uint32_t offset);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Loads HEX file
        /// @param path Path to HEX file to be loaded. Ignore address, load always from start!
        /// @param mem Content of HEX file in vector.
        ///
        /// @throw std::runtime_error when failed to open the file, or when it has invalid format
        ///////////////////////////////////////////////////////////////////////////////////////////
        static void LoadHexFile(const std::string &path, std::vector<uint32_t> &mem);

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Dumps HEX file
        /// @param path Path to HEX file to be dumped
        /// @param hex_type Type of HEX file to dump
        /// @param mem Pointer to memory to be dumped.
        /// @param offset Offset from 'mem' (in byte addressing) where to start dumping.
        /// @param size Number of bytes to be dumped (Number of words * 4)
        /// @throw std::runtime_error when failed to open the file.
        ///////////////////////////////////////////////////////////////////////////////////////////
        static void DumpHexFile(const std::string &path, HexFileType hex_type, uint32_t *mem,
                                uint32_t offset, size_t size);
};

#endif