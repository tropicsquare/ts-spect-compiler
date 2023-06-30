/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <stdio.h>
#include <iostream>
#include <cassert>

#include "OptionParser.h"

#include "Compiler.h"
#include "CpuModel.h"
#include "CpuProgram.h"
#include "HexHandler.h"

#include "vpi_user.h"

#include "spect_iss_dpi.h"

spect::Compiler *compiler = nullptr;
spect::CpuModel *model    = nullptr;

#define DPI_CALL_LOG_ENTER                                                                          \
    assert (model != nullptr &&                                                         \
            MODEL_LABEL " Model not initalized. Did you forget to call 'spect_dpi_init'?");         \
    if (model->verbosity_ >= VERBOSITY_HIGH)                                            \
        vpi_printf("%s DPI function '%s' entered.\n", MODEL_LABEL, __func__);                       \

#define DPI_CALL_LOG_EXIT                                                                           \
    if (model->verbosity_ >= VERBOSITY_HIGH)                                            \
        vpi_printf("%s DPI function '%s' exiting.\n", MODEL_LABEL, __func__);                       \

extern "C" {

    uint32_t spect_dpi_init()
    {
        vpi_printf("%s DPI function '%s' entered.\n", MODEL_LABEL, __func__);

        uint32_t rv = 0;
        try {
            model    = new spect::CpuModel(SPECT_INSTR_MEM_AHB_W, SPECT_INSTR_MEM_AHB_R);
            compiler = new spect::Compiler(SPECT_INSTR_MEM_BASE);

            // According to C standard, typecasting function pointer is undefined behavior,
            // but we only get rid of "const", so we hope its fine :)
            model->print_fnc = (int (*)(const char *format, ...))(&(vpi_printf));
            compiler->print_fnc = (int (*)(const char *format, ...))(&(vpi_printf));
        } catch (const std::bad_alloc& e) {
            vpi_printf("%s Failed to initialize SPECT DPI model: %s\n", MODEL_LABEL, e.what());
            rv = 1;
        }

        vpi_printf("%s DPI function '%s' exiting.\n", MODEL_LABEL, __func__);
        return rv;
    }

    void spect_dpi_exit()
    {
        vpi_printf("%s DPI function '%s' entered.\n", MODEL_LABEL, __func__);

        delete model;
        delete compiler;

        vpi_printf("%s DPI function '%s' exiting.\n", MODEL_LABEL, __func__);
    }

    void spect_dpi_reset()
    {
        DPI_CALL_LOG_ENTER
        model->Reset();
        DPI_CALL_LOG_EXIT
    }

    void spect_dpi_start()
    {
        DPI_CALL_LOG_ENTER
        model->Start();
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_is_program_finished()
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = model->IsFinished();
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_get_memory(uint32_t addr)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = model->GetMemory(addr);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_memory(uint32_t addr, uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        model->SetMemory(addr, data);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_ahb_read(uint32_t addr)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = model->ReadMemoryAhb(addr);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_ahb_write(uint32_t addr, uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        model->WriteMemoryAhb(addr, data);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_mem_base(dpi_mem_type_t mem_type)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = 0;
        switch (mem_type) {
        case DPI_SPECT_DATA_RAM_IN:
            rv = SPECT_DATA_RAM_IN_BASE;
            break;
        case DPI_SPECT_DATA_RAM_OUT:
            rv = SPECT_DATA_RAM_OUT_BASE;
            break;
        case DPI_SPECT_CONST_ROM:
            rv = SPECT_CONST_ROM_BASE;
            break;
        case DPI_SPECT_INSTRUCTION_MEM:
            rv = SPECT_INSTR_MEM_BASE;
            break;
        default:
            rv = 0;
        }
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_get_mem_size(dpi_mem_type_t mem_type)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = 0;
        switch (mem_type) {
        case DPI_SPECT_DATA_RAM_IN:
            rv = SPECT_DATA_RAM_IN_SIZE;
            break;
        case DPI_SPECT_DATA_RAM_OUT:
            rv = SPECT_DATA_RAM_OUT_SIZE;
            break;
        case DPI_SPECT_CONST_ROM:
            rv = SPECT_CONST_ROM_SIZE;
            break;
        case DPI_SPECT_INSTRUCTION_MEM:
            rv = SPECT_INSTR_MEM_SIZE;
            break;
        default:
            rv = 0;
        }
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_get_gpr_part(uint32_t gpr, uint32_t part)
    {
        DPI_CALL_LOG_ENTER
        uint256_t tmp = (model->GetGpr(gpr) >> (part * 32)) & uint256_t("0xFFFFFFFF");
        DPI_CALL_LOG_EXIT
        return (uint32_t)tmp;
    }

    void spect_dpi_set_gpr_part(uint32_t gpr, uint32_t part, uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        uint256_t mask = ~(uint256_t("0xFFFFFFFF") << (part * 32));
        uint256_t tmp = model->GetGpr(gpr) & mask;
        tmp = tmp | (uint256_t(data) << (part * 32));
        model->SetGpr(gpr, tmp);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_flag(dpi_flag_type_t flag_type)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv;
        switch (flag_type) {
        case DPI_SPECT_FLAG_ZERO:
            rv = model->GetCpuFlags().zero;
            break;
        case DPI_SPECT_FLAG_CARRY:
            rv = model->GetCpuFlags().carry;
            break;
        case DPI_SPECT_FLAG_ERROR:
            rv = model->GetCpuFlags().error;
            break;
        default:
            rv = 0;
        }
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_get_pc()
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = (uint32_t)model->GetPc();
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_pc(uint32_t value)
    {
        DPI_CALL_LOG_ENTER
        model->SetPc((uint16_t)value);
        DPI_CALL_LOG_EXIT
    }

    void spect_dpi_dump_instruction(uint32_t address, char *buf)
    {
        DPI_CALL_LOG_ENTER
        spect::Instruction *i = spect::Instruction::DisAssemble(
            model->GetParityType(),
            model->GetMemory(((uint16_t)address) >> 2));
        // TODO: How to handle free here? System Verilog side needs to free the buffer!
        buf = new char[32];
        std::stringstream ss;
        i->Dump(ss);
        strcpy(buf, ss.str().c_str());
        delete i;
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_rar_value(uint32_t address)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = model->GetRarAt(address);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_get_rar_sp()
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = model->GetRarSp();
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_push_grv_queue(uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        model->GrvQueuePush(data);
        DPI_CALL_LOG_EXIT
    }

    void spect_dpi_push_ldk_queue(uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        model->LdkQueuePush(data);
        DPI_CALL_LOG_EXIT
    }

    void spect_dpi_push_kbus_error_queue(uint8_t error)
    {
        DPI_CALL_LOG_ENTER
        model->KbusErrorQueuePush(error);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_interrupt(dpi_int_type_t int_type)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = 0;
        switch (int_type) {
        case DPI_SPECT_INT_DONE :
            rv = model->GetInterrrupt(spect::CpuIntType::INT_DONE);
            break;
        case DPI_SPECT_INT_ERR :
            rv = model->GetInterrrupt(spect::CpuIntType::INT_ERR);
            break;
        default:
            rv = 0;
        }
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_compile_program(const char *program_path, const char* hex_path,
                                       const dpi_hex_file_type_t hex_format,
                                       const dpi_parity_type_t parity_type)
    {
        DPI_CALL_LOG_ENTER
        uint32_t err;

        try {
            compiler->Compile(std::string(program_path));

            spect::HexFileType internal_hex_type;
            switch (hex_format) {
            case DPI_HEX_ISS_WORD:
                internal_hex_type = spect::HexFileType::ISS_WORD;
                break;
            case DPI_HEX_VERILOG_RAW_WORD:
                internal_hex_type = spect::HexFileType::VERILOG_RAW_WORD;
                break;
            case DPI_HEX_VERILOG_ADDR_WORD:
                internal_hex_type = spect::HexFileType::VERILOG_ADDR_WORD;
                break;
            }

            spect::ParityType internal_parity_type;
            switch (parity_type) {
            case DPI_PARITY_ODD:
                internal_parity_type = spect::ParityType::ODD;
                break;
            case DPI_PARITY_EVEN:
                internal_parity_type = spect::ParityType::EVEN;
                break;
            case DPI_PARITY_NONE:
                internal_parity_type = spect::ParityType::NONE;
                break;
            }
            err = compiler->CompileFinish();
            if (!err)
                compiler->program_->Assemble(
                    std::string(hex_path),
                    (spect::HexFileType) internal_hex_type,
                    internal_parity_type);

        } catch(std::runtime_error &exception) {
            vpi_printf("%s Failed to compile program: %s\n",
                        MODEL_LABEL, exception.what());
            err = 1;
        }

        DPI_CALL_LOG_EXIT
        return err;
    }

    uint32_t spect_dpi_get_compiled_program_start_address()
    {
        DPI_CALL_LOG_ENTER
        spect::Symbol* s_start_addr = compiler->symbols_->GetSymbol(START_SYMBOL);
        uint32_t rv = 0;
        if (s_start_addr)
            rv = s_start_addr->val_;
        else
            vpi_printf("%s: '%s' symbol not defined! Returning 0.\n", MODEL_LABEL, START_SYMBOL);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_model_start_pc(uint32_t start_pc)
    {
        DPI_CALL_LOG_ENTER
        model->SetStartPc(start_pc);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_load_hex_file(const char *path, const uint32_t offset)
    {
        DPI_CALL_LOG_ENTER
        spect::HexHandler::LoadHexFile(std::string(path), model->GetMemoryPtr(), offset);
        // TODO: Here it might be good to check that hex file spans out of
        //       memory.
        DPI_CALL_LOG_EXIT
        return 0;
    }

    uint32_t spect_dpi_program_step(uint32_t cycle_count)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = model->StepSingle(cycle_count);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_get_last_instr(dpi_instruction_t *dpi_instruction)
    {
        DPI_CALL_LOG_ENTER
        model->GetLastInstruction(dpi_instruction);
        DPI_CALL_LOG_EXIT
    }

    void spect_dpi_set_change_reporting(uint32_t enable)
    {
        DPI_CALL_LOG_ENTER
        model->change_reporting_ = enable;
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_model_change(dpi_state_change_t *change)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv;
        if (!model->HasChange()) {
            rv = 1;
        } else {
            *change = model->ConsumeChange();
            rv = 0;
        }
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_program_run(uint32_t instructions)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = model->Step(instructions);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_verbosity(uint32_t level)
    {
        DPI_CALL_LOG_ENTER
        model->verbosity_ = level;
        DPI_CALL_LOG_EXIT
    }

    dpi_parity_type_t spect_dpi_get_parity_type()
    {
        DPI_CALL_LOG_ENTER
        dpi_parity_type_t rv;
        switch (model->GetParityType()) {
        case spect::ParityType::ODD:
            rv = DPI_PARITY_ODD;
            break;
        case spect::ParityType::EVEN:
            rv = DPI_PARITY_EVEN;
            break;
        default:
            rv = DPI_PARITY_NONE;
            break;
        }
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_parity_type(dpi_parity_type_t parity_type)
    {
        DPI_CALL_LOG_ENTER
        switch (parity_type) {
        case DPI_PARITY_ODD:
            model->SetParityType(spect::ParityType::ODD);
            break;
        case DPI_PARITY_EVEN:
            model->SetParityType(spect::ParityType::EVEN);
            break;
        default:
            model->SetParityType(spect::ParityType::NONE);
            break;
        }
        DPI_CALL_LOG_EXIT
    }
}
