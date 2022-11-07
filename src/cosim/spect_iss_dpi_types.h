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


typedef enum {
    DPI_SPECT_DATA_RAM_IN       = (1 << 0),
    DPI_SPECT_DATA_RAM_OUT      = (1 << 1),
    DPI_SPECT_CONST_ROM         = (1 << 2),
    DPI_SPECT_INSTRUCTION_MEM   = (1 << 3)
} dpi_mem_type_t;

typedef enum {
    DPI_SPECT_FLAG_ZERO         = (1 << 0),
    DPI_SPECT_FLAG_CARRY        = (1 << 1)
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
    DPI_CHANGE_GPR              = (1 << 0),
    DPI_CHANGE_FLAG             = (1 << 1),
    DPI_CHANGE_MEM              = (1 << 2),
    DPI_CHANGE_INT              = (1 << 4),
    DPI_CHANGE_RAR              = (1 << 5)
} dpi_change_kind_t;

typedef enum {
    DPI_HEX_ISS_WORD            = (1 << 0),
    DPI_HEX_VERILOG_RAW_WORD    = (1 << 1),
    DPI_HEX_VERILOG_ADDR_WORD   = (1 << 2)
} dpi_hex_file_type_t;

inline std::string dpi_change_kind_to_str(dpi_change_kind_t in) {
    switch (in){
    case DPI_CHANGE_GPR  : return std::string("DPI_CHANGE_GPR");
    case DPI_CHANGE_FLAG : return std::string("DPI_CHANGE_FLAG");
    case DPI_CHANGE_MEM  : return std::string("DPI_CHANGE_MEM");
    case DPI_CHANGE_INT  : return std::string("DPI_CHANGE_INT");
    case DPI_CHANGE_RAR  : return std::string("DPI_CHANGE_RAR");
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
        else
            return std::string("UNKNOWN FLAG TYPE!");
        break;

    case DPI_CHANGE_MEM:
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
    //
    //  DPI_CHANGE_MEM:
    //      Address in memory on which change occured.
    //
    //  DPI_CHANGE_INT:
    //      DPI_SPECT_INT_DONE
    //      DPI_SPECT_INT_ERR
    //
    //  DPI_CHANGE_RAR:
    //      DPI_RAR_PUSH - Push on stack
    //      DPI_RAR_POP - Pop from stack
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
    //      0 - Bits 31:0 of memory location
    //
    //  DPI_CHANGE_INT:
    //      0 - Value of the interrupt
    //
    //  DPI_CHANGE_RAR:
    //      obj == DPI_RAR_PUSH (push) - Data pushed on stack
    //      obj == DPI_RAR_POP (pop) - Data popped from stack
    //      both are valid only in "new_val".
    uint32_t          old_val[8] = {0};
    uint32_t          new_val[8] = {0};
} dpi_state_change_t;

#endif