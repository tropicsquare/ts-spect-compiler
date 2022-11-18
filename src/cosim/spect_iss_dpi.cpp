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
#include "CpuSimulator.h"
#include "HexHandler.h"

#include "vpi_user.h"

#include "spect_iss_dpi.h"

spect::CpuSimulator *simulator = nullptr;

#define DPI_CALL_LOG_ENTER                                                                          \
    assert (simulator->model_ != nullptr &&                                                         \
            MODEL_LABEL " Model not initalized. Did you forget to call 'spect_dpi_init'?");         \
    if (simulator->model_->verbosity_ >= VERBOSITY_HIGH)                                            \
        vpi_printf("%s DPI function '%s' entered.\n", MODEL_LABEL, __func__);                       \

#define DPI_CALL_LOG_EXIT                                                                           \
    if (simulator->model_->verbosity_ >= VERBOSITY_HIGH)                                            \
        vpi_printf("%s DPI function '%s' exiting.\n", MODEL_LABEL, __func__);                       \

extern "C" {

    uint32_t spect_dpi_init()
    {
        vpi_printf("%s DPI function '%s' entered.\n", MODEL_LABEL, __func__);

        uint32_t rv = 0;
        try {
            simulator = new spect::CpuSimulator();

            // According to C standard, typecasting function pointer is undefined behavior,
            // but we only get rid of "const", so we hope its fine :)
            simulator->model_->print_fnc = (int (*)(const char *format, ...))(&(vpi_printf));
            simulator->compiler_->print_fnc = (int (*)(const char *format, ...))(&(vpi_printf));
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

        delete simulator;

        vpi_printf("%s DPI function '%s' exiting.\n", MODEL_LABEL, __func__);
    }

    void spect_dpi_reset()
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->Reset();
        DPI_CALL_LOG_EXIT
    }

    void spect_dpi_start()
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->Start();
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_is_program_finished()
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = simulator->model_->IsFinished();
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_get_memory(uint32_t addr)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = simulator->model_->GetMemory(addr);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_memory(uint32_t addr, uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->SetMemory(addr, data);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_ahb_read(uint32_t addr)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = simulator->model_->ReadMemoryAhb(addr);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_ahb_write(uint32_t addr, uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->WriteMemoryAhb(addr, data);
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
        uint256_t tmp = (simulator->model_->GetGpr(gpr) >> (part * 32)) & uint256_t("0xFFFFFFFF");
        DPI_CALL_LOG_EXIT
        return (uint32_t)tmp;
    }

    void spect_dpi_set_gpr_part(uint32_t gpr, uint32_t part, uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        uint256_t mask = ~(uint256_t("0xFFFFFFFF") << (part * 32));
        uint256_t tmp = simulator->model_->GetGpr(gpr) & mask;
        tmp = tmp | (uint256_t(data) << (part * 32));
        simulator->model_->SetGpr(gpr, tmp);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_flag(dpi_flag_type_t flag_type)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv;
        switch (flag_type) {
        case DPI_SPECT_FLAG_ZERO:
            rv = simulator->model_->GetCpuFlags().zero;
            break;
        case DPI_SPECT_FLAG_CARRY:
            rv = simulator->model_->GetCpuFlags().carry;
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
        uint32_t rv = (uint32_t)simulator->model_->GetPc();
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_pc(uint32_t value)
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->SetPc((uint16_t)value);
        DPI_CALL_LOG_EXIT
    }

    void spect_dpi_dump_instruction(uint32_t address, char *buf)
    {
        DPI_CALL_LOG_ENTER
        spect::Instruction *i = spect::Instruction::DisAssemble(
            simulator->model_->GetMemory(((uint16_t)address) >> 2));
        // TODO: How to handle free here? System Verilog side needs to free the buffer!
        buf = new char[32];
        std::stringstream ss;
        i->Dump(ss);
        strcpy(buf, ss.str().c_str());
        delete i;
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_srr(uint32_t part)
    {
        DPI_CALL_LOG_ENTER
        uint256_t tmp = (simulator->model_->GetSrr() >> (part * 32)) & uint256_t("0xFFFFFFFF");
        DPI_CALL_LOG_EXIT
        return (uint32_t)tmp;
    }

    uint32_t spect_dpi_get_rar_value(uint32_t address)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = simulator->model_->GetRarAt(address);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_get_rar_sp()
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = simulator->model_->GetRarSp();
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_push_grv_queue(uint32_t data)
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->GrvQueuePush(data);
        DPI_CALL_LOG_EXIT
    }

    void spect_dpi_push_gpk_queue(uint32_t data, uint32_t index)
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->GpkQueuePush(index, data);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_interrupt(dpi_int_type_t int_type)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = 0;
        switch (int_type) {
        case DPI_SPECT_INT_DONE :
            rv = simulator->model_->GetInterrrupt(spect::CpuIntType::INT_DONE);
            break;
        case DPI_SPECT_INT_ERR :
            rv = simulator->model_->GetInterrrupt(spect::CpuIntType::INT_ERR);
            break;
        default:
            rv = 0;
        }
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_compile_program(const char *program_path, const char* hex_path,
                                       const dpi_hex_file_type_t hex_format)
    {
        DPI_CALL_LOG_ENTER
        uint32_t err;

        try {
            simulator->compiler_->Compile(std::string(program_path));

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
            err = simulator->compiler_->CompileFinish();
            if (!err)
                simulator->compiler_->program_->Assemble(
                    std::string(hex_path), (spect::HexFileType) internal_hex_type);

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
        spect::Symbol* s_start_addr = simulator->compiler_->symbols_->GetSymbol(START_SYMBOL);
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
        simulator->model_->SetStartPc(start_pc);
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_load_hex_file(const char *path, const uint32_t offset)
    {
        DPI_CALL_LOG_ENTER
        spect::HexHandler::LoadHexFile(std::string(path), simulator->model_->GetMemoryPtr(), offset);
        // TODO: Here it might be good to check that hex file spans out of
        //       memory.
        DPI_CALL_LOG_EXIT
        return 0;
    }

    uint32_t spect_dpi_program_step(uint32_t cycle_count)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = simulator->model_->StepSingle(cycle_count);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_change_reporting(uint32_t enable)
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->change_reporting_ = enable;
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_get_model_change(dpi_state_change_t *change)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv;
        if (!simulator->model_->HasChange()) {
            rv = 1;
        } else {
            *change = simulator->model_->ConsumeChange();
            rv = 0;
        }
        DPI_CALL_LOG_EXIT
        return rv;
    }

    uint32_t spect_dpi_program_run(uint32_t instructions)
    {
        DPI_CALL_LOG_ENTER
        uint32_t rv = simulator->model_->Step(instructions);
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_verbosity(uint32_t level)
    {
        DPI_CALL_LOG_ENTER
        simulator->model_->verbosity_ = level;
        DPI_CALL_LOG_EXIT
    }
}