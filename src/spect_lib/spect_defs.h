/**************************************************************************************************
*
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
*
* @todo: License
*
* @author Ondrej Ille, <ondrej.ille@tropicsquare.com>
* @date 19.9.2022
*
**************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////
// CPU config
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SPECT_RAR_DEPTH
#define SPECT_RAR_DEPTH 5
#endif

#ifndef SPECT_GPR_CNT
#define SPECT_GPR_CNT 32
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Memory space parameters
///////////////////////////////////////////////////////////////////////////////////////////////////

// Total size of SPECTs memory space.
#define SPECT_TOTAL_MEM_SIZE (64 * 1024)


// DATA RAM IN
#ifndef SPECT_DATA_RAM_IN_BASE
#define SPECT_DATA_RAM_IN_BASE 0x0
#endif

#ifndef SPECT_DATA_RAM_IN_SIZE
#define SPECT_DATA_RAM_IN_SIZE 0x800
#endif


// DATA RAM OUT
#ifndef SPECT_DATA_RAM_OUT_BASE
#define SPECT_DATA_RAM_OUT_BASE 0x1000
#endif

#ifndef SPECT_DATA_RAM_OUT_SIZE
#define SPECT_DATA_RAM_OUT_SIZE 0x200
#endif


// Configuration Registers
#ifndef SPECT_CONFIG_REGS_BASE
#define SPECT_CONFIG_REGS_BASE 0x2000
#endif

#ifndef SPECT_CONFIG_REGS_SIZE
#define SPECT_CONFIG_REGS_SIZE 0xF
#endif


// CONSTANT ROM
#ifndef SPECT_CONST_ROM_BASE
#define SPECT_CONST_ROM_BASE 0x3000
#endif

#ifndef SPECT_CONST_ROM_SIZE
#define SPECT_CONST_ROM_SIZE 0x800
#endif


// External Memory In
#ifndef SPECT_EMEM_IN_BASE
#define SPECT_EMEM_IN_BASE 0x4000
#endif

#ifndef SPECT_EMEM_IN_SIZE
// 0x90 - 144 bytes - Size of TROPIC01 Mailbox Command Buffer
#define SPECT_EMEM_IN_SIZE 0x90
#endif


// External Memory Out
#ifndef SPECT_EMEM_OUT_BASE
#define SPECT_EMEM_OUT_BASE 0x5000
#endif

#ifndef SPECT_EMEM_OUT_SIZE
// 0x80 - 128 bytes - Size of TROPIC01 Mailbox Result Buffer
#define SPECT_EMEM_OUT_SIZE 0x80
#endif


// Instruction Memory
#ifndef SPECT_INSTR_MEM_BASE
#define SPECT_INSTR_MEM_BASE 0x8000
#endif

#ifndef SPECT_INSTR_MEM_SIZE
// 32X3072 - Size of Instruction RAM in TROPIC01
#define SPECT_INSTR_MEM_SIZE 0x3000
#endif

// If true, SPECT instruction memory is readable via AHB
#define SPECT_INSTR_MEM_AHB_R true

// If true, SPECT instruction memory is writable via AHB
#define SPECT_INSTR_MEM_AHB_W true

///////////////////////////////////////////////////////////////////////////////////////////////////
// Key memory parameters
///////////////////////////////////////////////////////////////////////////////////////////////////
// Number of types
#ifndef KEY_MEM_TYPE_NUM
#define KEY_MEM_TYPE_NUM 16
#endif

// Number of slots
#ifndef KEY_MEM_SLOT_NUM
#define KEY_MEM_SLOT_NUM 256
#endif

// Number of offsets
#ifndef KEY_MEM_OFFSET_NUM
#define KEY_MEM_OFFSET_NUM 256
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Internal defines
///////////////////////////////////////////////////////////////////////////////////////////////////

#define IENC_PARITY_BITS        1
#define IENC_TYPE_BITS          2
#define IENC_OPCODE_BITS        4
#define IENC_FUNC_BITS          3
#define IENC_OP_BITS            5
#define IENC_IMMEDIATE_BITS     12
#define IENC_ADDR_BITS          16
#define IENC_NEW_PC_BITS        16

#define IENC_PARITY_MASK        0x1
#define IENC_TYPE_MASK          0x3
#define IENC_OPCODE_MASK        0xF
#define IENC_FUNC_MASK          0x7
#define IENC_OP_MASK            0x1F
#define IENC_IMMEDIATE_MASK     0xFFF
#define IENC_ADDR_MASK          0xFFFF
#define IENC_NEW_PC_MASK        0xFFFF

#define IENC_PARITY_OFFSET      31
#define IENC_TYPE_OFFSET        29
#define IENC_OPCODE_OFFSET      25
#define IENC_FUNC_OFFSET        22
#define IENC_OP1_OFFSET         17
#define IENC_OP2_OFFSET         12
#define IENC_OP3_OFFSET         7
#define IENC_IMMEDIATE_OFFSET   0
#define IENC_ADDR_OFFSET        0
#define IENC_NEW_PC_OFFSET      0
