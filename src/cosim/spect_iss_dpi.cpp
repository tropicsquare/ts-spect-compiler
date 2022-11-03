/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <iostream>
#include <cassert>

#include "OptionParser.h"

#include "Compiler.h"
#include "CpuModel.h"
#include "CpuProgram.h"
#include "HexHandler.h"

#include "spect_iss_dpi.h"

spect::Compiler *compiler = nullptr;
spect::CpuModel *model = nullptr;

#define DPI_CALL_LOG_ENTER                                                                          \
    assert (model != nullptr &&                                                                     \
            MODEL_LABEL " Model not initalized. Did you forget to call 'spect_dpi_init'?");         \
    if (model->verbosity_ >= VERBOSITY_HIGH)                                                        \
        std::cout << MODEL_LABEL << "DPI function '" << __func__ << "' entered." << std::endl;      \

#define DPI_CALL_LOG_EXIT                                                                           \
    if (model->verbosity_ >= VERBOSITY_HIGH)                                                        \
        std::cout << MODEL_LABEL << "DPI function '" << __func__ << "' exiting." << std::endl;      \

extern "C" {

    uint32_t spect_dpi_init()
    {
        std::cout << MODEL_LABEL << "DPI function '" << __func__ << "' entered." << std::endl;

        uint32_t rv = 0;
        try {
            compiler = new spect::Compiler(SPECT_INSTR_MEM_BASE);
            model = new spect::CpuModel(SPECT_INSTR_MEM_AHB_RW);
        } catch (const std::bad_alloc& e) {
            std::cout << "Failed to initialize SPECT DPI model:" << std::endl;
            std::cout << e.what() << std::endl;
            rv = 1;
        }

        std::cout << MODEL_LABEL << "DPI function '" << __func__ << "' exiting." << std::endl;
        return rv;
    }

    void spect_dpi_exit()
    {
        std::cout << MODEL_LABEL << "DPI function '" << __func__ << "' entered." << std::endl;

        delete compiler;
        delete model;

        std::cout << MODEL_LABEL << "DPI function '" << __func__ << "' exiting." << std::endl;
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
        uint256_t tmp = model->GetGpr(gpr) & mask;\
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
        spect::Instruction *i = spect::Instruction::DisAssemble(model->GetMemory(((uint16_t)address) >> 2));
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
        uint256_t tmp = (model->GetSrr() >> (part * 32)) & uint256_t("0xFFFFFFFF");
        DPI_CALL_LOG_EXIT
        return (uint32_t)tmp;
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

    void spect_dpi_push_gpk_queue(uint32_t data, uint32_t index)
    {
        DPI_CALL_LOG_ENTER
        model->GpkQueuePush(index, data);
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
                                       const int hex_format)
    {
        DPI_CALL_LOG_ENTER
        uint32_t err;

        try {
            compiler->Compile(std::string(program_path));

            err = compiler->CompileFinish();
            if (!err)
                compiler->program_->Assemble(std::string(hex_path), (spect::HexFileType) hex_format);

        } catch(std::runtime_error &exception) {
            std::cout << exception.what() << std::endl;
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
            std::cout << MODEL_LABEL << "'" << START_SYMBOL << "' symbol not defined! Returning 0.";
        DPI_CALL_LOG_EXIT
        return rv;
    }

    void spect_dpi_set_model_start_pc(uint32_t start_pc)
    {
        DPI_CALL_LOG_ENTER
        model->start_pc_ = start_pc;
        DPI_CALL_LOG_EXIT
    }

    uint32_t spect_dpi_load_hex_file(const char *path)
    {
        DPI_CALL_LOG_ENTER
        spect::HexHandler::LoadHexFile(std::string(path), model->GetMemoryPtr());
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
}