/******************************************************************************
*
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
*
* @todo: License
*
* @author Ondrej Ille, <ondrej.ille@tropicsquare.com>
* @date 19.9.2022
*
*****************************************************************************/

#ifndef SPECT_LIB_SPECT_H_
#define SPECT_LIB_SPECT_H_

#include "uintwide_t.h"
#include "spect_defs.h"

using math::wide_integer::uint256_t;
using math::wide_integer::uint512_t;
using math::wide_integer::int512_t;
using math::wide_integer::uint1024_t;

namespace spect {

    enum class CpuGpr {
        R0  = 0,
        R1  = 1,
        R2  = 2,
        R3  = 3,
        R4  = 4,
        R5  = 5,
        R6  = 6,
        R7  = 7,
        R8  = 8,
        R9  = 9,
        R10 = 10,
        R11 = 11,
        R12 = 12,
        R13 = 13,
        R14 = 14,
        R15 = 15,
        R16 = 16,
        R17 = 17,
        R18 = 18,
        R19 = 19,
        R20 = 20,
        R21 = 21,
        R22 = 22,
        R23 = 23,
        R24 = 24,
        R25 = 25,
        R26 = 26,
        R27 = 27,
        R28 = 28,
        R29 = 29,
        R30 = 30,
        R31 = 31,

        R_INVALID = 32
    };

    std::ostream& operator << ( std::ostream& os, const spect::CpuGpr& gpr);

    enum class InstructionType {
        R = 0b11,
        I = 0b01,
        M = 0b10,
        J = 0b00
    };

    struct CpuFlags {
        bool zero;
        bool carry;
        bool error;
    };

    enum class CpuFlagType {
        ZERO,
        CARRY,
        ERROR
    };

    enum class CpuIntType {
        INT_DONE,
        INT_ERR
    };

    enum class CpuMemory {
        DATA_RAM_IN,
        DATA_RAM_OUT,
        CONST_ROM,
        CONFIG_REGS,
        INSTR_MEM
    };

    enum class SymbolType {
        LABEL,
        CONSTANT,
        UNKNOWN
    };

    std::ostream& operator << ( std::ostream& os, const spect::SymbolType& symbol_type);

    enum class HexFileType {

        // Instruction set simulator compatible. Word addressing
        //  Example:
        //     @8000 ABCDE1234
        //     @8004 56781234
        //  The example above is an example of loading data into first two addresses of
        //  SPECT Instruction Memory.
        ISS_WORD                    = 0,

        // $readmemh compatible with flat verilog model of 32 bit RAM. No addressing
        // Example:
        //      123AEEFF
        VERILOG_RAW_WORD            = 1,

        // $readmemh compatible with flat verilog model of 32 bit RAM. +0x1 for each address
        // Example:
        //      @0000 123AEEFF
        //      @0001 45789ABC
        //  Note: First address is relative to base address of memory.
        VERILOG_ADDR_WORD           = 2
    };

    enum class ParityType {
        NONE,
        ODD,
        EVEN
    };

    std::ostream& operator << ( std::ostream& os, const spect::ParityType& parity_type);

    class Instruction;
    class InstructionFactory;
    class InstructionR;
    class InstructionI;
    class InstructionJ;
    class InstructionM;

    SPECT_SUM_INSTRUCTIONS

    class CpuModel;
    class CpuSimulator;
    class CpuProgram;
    class HexHandler;

    class Compiler;
    class Symbol;
    class SymbolTable;
    class SourceFile;

    uint32_t stoint(std::string str);
    std::string tohexs(uint64_t i, int width);
    std::string tohexs(uint256_t val);

    #define TO_INT(x) static_cast<int>(x)
    #define TO_CPU_GPR(x) static_cast<spect::CpuGpr>(x)

    #define INSTR_ENCODE(func, opcode, itype)                                   \
        ((func           & IENC_FUNC_MASK)      << IENC_FUNC_OFFSET)        |   \
        ((opcode         & IENC_OPCODE_MASK)    << IENC_OPCODE_OFFSET)      |   \
        ((TO_INT(itype)  & IENC_TYPE_MASK)      << IENC_TYPE_OFFSET)

    #define IDENT_REGEX "([a-zA-Z_][a-zA-Z_0-9]*)"
    #define FILE_REGEX ".+"
    #define VAL_REGEX "(0x|0b)*[0-9a-fA-F]+"
    #define OP_REGEX "(r|R)(0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|24|25|26|27|28|29|30|31)"
    #define NUM_REGEX "(0x[a-fA-F0-9]+|0b[0-1]+|[0-9]+)"
    #define START_SYMBOL "_start"
    #define INCLUDE_KEYWORD "(\\.include)"
    #define EQ_KEYWORD "(\\.eq)"

    #define VERBOSITY_NONE 0
    #define VERBOSITY_LOW 1
    #define VERBOSITY_MEDIUM 2
    #define VERBOSITY_HIGH 3

    #define MODEL_LABEL "SPECT_MODEL: "

    #define KECCAK_CAPACITY 256
    #define KECCAK_RATE     144

#if defined(__has_cpp_attribute)
    #if 	__has_cpp_attribute(maybe_unused)
        #define A_UNUSED [[maybe_unused]]
    #else
        #define A_UNUSED __attribute__((unused))
    #endif
#else
    #define A_UNUSED __attribute__((unused))
#endif

    #define A_PACKED __attribute__((packed))

}

#endif
