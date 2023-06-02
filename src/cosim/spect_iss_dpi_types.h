/**************************************************************************************************
** DPI Interface to SPECT Instruction model
**
**  Common notes to the API:
**      - All addresses are passed byte aligned (each next 32-bit word is +0x4 higher).
**      - Sizes of all memories are handled in bytes.
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef COSIM_SPECT_ISS_DPI_TYPES_H_
#define COSIM_SPECT_ISS_DPI_TYPES_H_

#include <string>

#define DPIENC_KBUS_OP_MASK        0x3F
#define DPIENC_KBUS_TYPE_MASK      0xF
#define DPIENC_KBUS_SLOT_MASK      0xFF
#define DPIENC_KBUS_OFFSET_MASK    0x1F

#define DPIENC_KBUS_OP_OFFSET      0
#define DPIENC_KBUS_TYPE_OFFSET    8
#define DPIENC_KBUS_SLOT_OFFSET    12
#define DPIENC_KBUS_OFFSET_OFFSET  20

typedef enum {
    DPI_SPECT_DATA_RAM_IN       = (1 << 0),
    DPI_SPECT_DATA_RAM_OUT      = (1 << 1),
    DPI_SPECT_CONST_ROM         = (1 << 2),
    DPI_SPECT_INSTRUCTION_MEM   = (1 << 3)
} dpi_mem_type_t;

typedef enum {
    DPI_SPECT_FLAG_ZERO         = (1 << 0),
    DPI_SPECT_FLAG_CARRY        = (1 << 1),
    DPI_SPECT_FLAG_ERROR        = (1 << 2)
} dpi_flag_type_t;

typedef enum {
    DPI_SPECT_INT_DONE          = (1 << 0),
    DPI_SPECT_INT_ERR           = (1 << 1),
} dpi_int_type_t;

typedef enum {
    DPI_RAR_PUSH                = (1 << 0),
    DPI_RAR_POP                 = (1 << 1)
} dpi_rar_change_kind_t;

typedef enum {
    DPI_KBUS_WRITE              = (1 << 0),
    DPI_KBUS_READ               = (1 << 1),
    DPI_KBUS_PROGRAM            = (1 << 2),
    DPI_KBUS_ERASE              = (1 << 3),
    DPI_KBUS_VERIFY             = (1 << 4),
    DPI_KBUS_FLUSH              = (1 << 5)
} dpi_kbus_change_kind_t;

typedef enum {
    DPI_CHANGE_GPR              = (1 << 0),
    DPI_CHANGE_FLAG             = (1 << 1),
    DPI_CHANGE_MEM              = (1 << 2),
    DPI_CHANGE_INT              = (1 << 4),
    DPI_CHANGE_RAR              = (1 << 5),
    DPI_CHANGE_EMEM_IN          = (1 << 6),
    DPI_CHANGE_EMEM_OUT         = (1 << 7),
    DPI_CHANGE_RBUS             = (1 << 8),
    DPI_CHANGE_KBUS             = (1 << 9)
} dpi_change_kind_t;

typedef enum {
    DPI_HEX_ISS_WORD            = (1 << 0),
    DPI_HEX_VERILOG_RAW_WORD    = (1 << 1),
    DPI_HEX_VERILOG_ADDR_WORD   = (1 << 2)
} dpi_hex_file_type_t;

typedef struct {
    uint32_t i_type;
    uint32_t opcode;
    uint32_t func;

    // Register indices:
    //      0 - 31     - Valid register indices
    uint32_t op1;
    uint32_t op2;
    uint32_t op3;

    uint32_t immediate;
    uint32_t addr;
    uint32_t old_pc;
    uint32_t new_pc;

    // Operand values
    uint32_t op1_v[8];
    uint32_t op2_v[8];
    uint32_t op3_v[8];
    uint32_t r31_v[8];
} dpi_instruction_t;

inline std::string dpi_change_kind_to_str(dpi_change_kind_t in) {
    switch (in){
    case DPI_CHANGE_GPR      : return std::string("DPI_CHANGE_GPR");
    case DPI_CHANGE_FLAG     : return std::string("DPI_CHANGE_FLAG");
    case DPI_CHANGE_MEM      : return std::string("DPI_CHANGE_MEM");
    case DPI_CHANGE_INT      : return std::string("DPI_CHANGE_INT");
    case DPI_CHANGE_RAR      : return std::string("DPI_CHANGE_RAR");
    case DPI_CHANGE_EMEM_IN  : return std::string("DPI_CHANGE_EMEM_IN");
    case DPI_CHANGE_EMEM_OUT : return std::string("DPI_CHANGE_EMEM_OUT");
    case DPI_CHANGE_RBUS     : return std::string("DPI_CHANGE_RBUS");
    case DPI_CHANGE_KBUS     : return std::string("DPI_CHANGE_KBUS");
    }
    return "";
}

inline std::string dpi_change_obj_to_str(dpi_change_kind_t in, uint32_t obj) {
    switch (in){
    case DPI_CHANGE_GPR:
        return std::string("R") + std::to_string(obj);
        break;

    case DPI_CHANGE_FLAG:
        if (obj == DPI_SPECT_FLAG_ZERO)
            return std::string("DPI_SPECT_FLAG_ZERO");
        else if (obj == DPI_SPECT_FLAG_CARRY)
            return std::string("DPI_SPECT_FLAG_CARRY");
        else if (obj == DPI_SPECT_FLAG_ERROR)
            return std::string("DPI_SPECT_FLAG_ERROR");
        else
            return std::string("UNKNOWN FLAG TYPE!");
        break;

    case DPI_CHANGE_MEM:
    case DPI_CHANGE_EMEM_IN:
    case DPI_CHANGE_EMEM_OUT:
        return std::to_string(obj);
        break;

    case DPI_CHANGE_INT:
        if (obj == DPI_SPECT_INT_DONE)
            return std::string("DPI_SPECT_INT_DONE");
        else if (obj == DPI_SPECT_INT_ERR)
            return std::string("DPI_SPECT_INT_ERR");
        else
            return std::string("UNKNOWN INTERRUPT TYPE!");
        break;

    case DPI_CHANGE_RAR:
        if (obj == DPI_RAR_PUSH)
            return std::string("DPI_RAR_PUSH");
        else if (obj == DPI_RAR_POP)
            return std::string("DPI_RAR_POP");
        else
            return std::string("UNKNOWN RAR CHANGE TYPE!");
        break;

    case DPI_CHANGE_RBUS:
        return "n/a";
        break;

    case DPI_CHANGE_KBUS:
        uint32_t op     = (obj >> DPIENC_KBUS_OP_OFFSET)     & DPIENC_KBUS_OP_MASK;
        uint32_t type   = (obj >> DPIENC_KBUS_TYPE_OFFSET)   & DPIENC_KBUS_TYPE_MASK;
        uint32_t slot   = (obj >> DPIENC_KBUS_SLOT_OFFSET)   & DPIENC_KBUS_SLOT_MASK;
        uint32_t offset = (obj >> DPIENC_KBUS_OFFSET_OFFSET) & DPIENC_KBUS_OFFSET_MASK;
        std::string operation;

        if (op == DPI_KBUS_WRITE)
            operation = std::string("KBUS_WRITE");
        else if (op == DPI_KBUS_READ)
            operation = std::string("KBUS_READ");
        else if (op == DPI_KBUS_PROGRAM)
            operation = std::string("KBUS_PROGRAM");
        else if (op == DPI_KBUS_ERASE)
            operation = std::string("KBUS_ERASE");
        else if (op == DPI_KBUS_VERIFY)
            operation = std::string("KBUS_VERIFY");
        else if (op == DPI_KBUS_FLUSH)
            operation = std::string("KBUS_FLUSH");
        else
            operation = std::string("UNKNOWN KBUS OPERATION!");

        return std::string("Operation: ") + operation +
               std::string(" Type: ") + std::to_string(type) +
               std::string(" Slot: ") + std::to_string(slot) +
               std::string(" Offset: ") + std::to_string(offset);
        break;
    }
    return "";
}

typedef struct {
    dpi_change_kind_t kind = DPI_CHANGE_GPR;

    // Identifies object on which change occured based on kind
    //  DPI_CHANGE_GPR:
    //      0  - GPR0
    //      ...
    //      31 - GPR31
    //
    //  DPI_CHANGE_FLAG:
    //      DPI_SPECT_FLAG_ZERO
    //      DPI_SPECT_FLAG_CARRY
    //      DPI_SPECT_FLAG_ERROR
    //
    //  DPI_CHANGE_MEM:
    //  DPI_CHANGE_EMEM_IN:
    //  DPI_CHANGE_EMEM_OUT:
    //      Address in memory on which change occured.
    //
    //  DPI_CHANGE_INT:
    //      DPI_SPECT_INT_DONE
    //      DPI_SPECT_INT_ERR
    //
    //  DPI_CHANGE_RAR:
    //      DPI_RAR_PUSH - Push on stack
    //      DPI_RAR_POP - Pop from stack
    //
    //  DPI_CHANGE_RBUS:
    //      no meaning
    //
    //  DPI_CHANGE_KBUS:
    //       5: 0 - operation
    //      11: 8 - type
    //      19:12 - slot
    //      31:20 - offset
    uint32_t          obj = 0;

    // Old / New value of the object based  on 'kind':
    //  DPI_CHANGE_GPR:
    //      0 - Bits 31:0
    //      ...
    //      7 - Bits 255:232
    //
    //  DPI_CHANGE_FLAG:
    //      0 - Value of flag
    //
    //  DPI_CHANGE_MEM:
    //  DPI_CHANGE_EMEM_IN:
    //  DPI_CHANGE_EMEM_OUT:
    //      0 - Bits 31:0 of memory location
    //
    //  DPI_CHANGE_INT:
    //      0 - Value of the interrupt
    //
    //  DPI_CHANGE_RAR:
    //      obj == DPI_RAR_PUSH (push) - Data pushed on stack
    //      obj == DPI_RAR_POP (pop) - Data popped from stack
    //      both are valid only in "new_val".
    //
    //  DPI_CHANGE_RBUS:
    //      no meaning
    //
    //  DPI_CHANGE_KBUS:
    //      operation == DPI_KBUS_WRITE - data to write
    //      otherwise no meaning
    uint32_t          old_val[8] = {0};
    uint32_t          new_val[8] = {0};
} dpi_state_change_t;

#endif
