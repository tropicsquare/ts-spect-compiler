/**************************************************************************************************
** 
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <iostream>

#include "OptionParser.h"

#include "Compiler.h"
#include "CpuModel.h"
#include "CpuProgram.h"
#include "HexHandler.h"

#include "spect_iss_dpi.h"

spect::Compiler *compiler = nullptr;
spect::CpuModel *model = nullptr;

extern "C" {

    uint32_t spect_dpi_init()
    {
        try {
            compiler = new spect::Compiler(SPECT_INSTR_MEM_BASE);
            model = new spect::CpuModel(SPECT_INSTR_MEM_AHB_RW); 
        } catch (const std::bad_alloc& e) {
            std::cout << "Failed to initialize SPECT DPI model:" << std::endl;
            std::cout << e.what() << std::endl;
            return 1;
        }
        return 0;
    }

    void spect_dpi_exit()
    {
        delete compiler;
        delete model;
    }

    void spect_dpi_reset()
    {
        model->Reset();
    }

    uint32_t spect_dpi_get_memory(uint32_t addr)
    {
        return model->GetMemory(addr);
    }

    void spect_dpi_set_memory(uint32_t addr, uint32_t data)
    {
        model->SetMemory(addr, data);
    }

    uint32_t spect_dpi_ahb_read(uint32_t addr)
    {
        return model->ReadMemoryAhb(addr);
    }

    void spect_dpi_ahb_write(uint32_t addr, uint32_t data)
    {
        model->WriteMemoryAhb(addr, data);
    }

    uint32_t spect_dpi_get_mem_base(dpi_mem_type_t mem_type)
    {
        switch (mem_type) {
        case DPI_SPECT_DATA_RAM_IN:
            return SPECT_DATA_RAM_IN_BASE;
        case DPI_SPECT_DATA_RAM_OUT:
            return SPECT_DATA_RAM_OUT_BASE;
        case DPI_SPECT_CONST_ROM:
            return SPECT_CONST_ROM_BASE;
        case DPI_SPECT_INSTRUCTION_MEM:
            return SPECT_INSTR_MEM_BASE;
        default:
            return 0;
        }
    }

    uint32_t spect_dpi_get_mem_size(dpi_mem_type_t mem_type)
    {
        switch (mem_type) {
        case DPI_SPECT_DATA_RAM_IN:
            return SPECT_DATA_RAM_IN_SIZE;
        case DPI_SPECT_DATA_RAM_OUT:
            return SPECT_DATA_RAM_OUT_SIZE;
        case DPI_SPECT_CONST_ROM:
            return SPECT_CONST_ROM_SIZE;
        case DPI_SPECT_INSTRUCTION_MEM:
            return SPECT_INSTR_MEM_SIZE;
        default:
            return 0;
        }
    }

    uint32_t spect_dpi_get_gpr_part(uint32_t gpr, uint32_t part)
    {
        uint256_t tmp = (model->gpr_[gpr % 32] >> (part * 32)) & uint256_t("0xFFFFFFFF");
        return (uint32_t)tmp;
    }

    void spect_dpi_set_gpr_part(uint32_t gpr, uint32_t part, uint32_t data)
    {
        uint256_t mask = ~(uint256_t("0xFFFFFFFF") << (part * 32));
        uint256_t tmp = model->gpr_[gpr % 32] & mask;\
        tmp = tmp | (uint256_t(data) << (part * 32));
        model->gpr_[gpr % 32] = tmp;
    }

    uint32_t spect_dpi_get_flag(dpi_flag_type_t flag_type)
    {
        switch (flag_type) {
        case DPI_SPECT_FLAG_ZERO:
            return model->flags_.zero;
        case DPI_SPECT_FLAG_CARRY:
            return model->flags_.carry;
        default:
            return 0;
        }
    }

    uint32_t spect_dpi_get_pc()
    {
        return (uint32_t)model->pc_;
    }

    void spect_dpi_set_pc(uint32_t value)
    {
        model->pc_ = (uint16_t)value;
    }

    void spect_dpi_dump_instruction(uint32_t address, char *buf)
    {
        spect::Instruction *i = spect::Instruction::DisAssemble(model->memory_[((uint16_t)address) >> 2]);
        // TODO: How to handle free here? System Verilog side needs to free the buffer!
        buf = new char[32];
        std::stringstream ss;
        i->Dump(ss);
        strcpy(buf, ss.str().c_str());
        delete i;
    }

    uint32_t spect_dpi_get_srr(uint32_t part)
    {
        uint256_t tmp = (model->srr_ >> (part * 32)) & uint256_t("0xFFFFFFFF");
        return (uint32_t)tmp;
    }

    uint32_t spect_dpi_get_rar_value(uint32_t address)
    {
        return model->rar_stack_[address];
    }

    uint32_t spect_dpi_get_rar_sp()
    {
        return model->rar_sp_;
    }

    void spect_dpi_push_grv_queue(uint32_t data)
    {
        model->grv_q_.push(data);
    }

    void spect_dpi_push_gpk_queue(uint32_t data, uint32_t index)
    {
        model->gpk_q_[index].push(data);
    }

    uint32_t spect_dpi_get_interrupt(dpi_int_type_t int_type)
    {
        switch (int_type) {
        case DPI_SPECT_INT_DONE :
            return model->int_done_;
        case DPI_SPECT_INT_ERR :
            return model->int_err_;
        default:
            return 0;
        }
    }

    uint32_t spect_dpi_assemble_and_load_firmware(const char *path)
    {
        uint32_t err;
        
        try {
            compiler->Compile(std::string(path));

            err = compiler->CompileFinish();
            if (err)
                return err;
            
            uint32_t *first_addr = model->memory_ + (compiler->first_addr_ >> 2);
            compiler->program_->Assemble(first_addr);

        } catch(std::runtime_error &err) {
            return 1;
        }

        return 0;
    }

    uint32_t spect_dpi_load_hex(const char *path)
    {
        spect::HexHandler::LoadHexFile(std::string(path), model->memory_);
        // TODO: Here it might be good to check that hex file spans out of
        //       memory.
        return 0;
    }

    uint32_t spect_dpi_program_step(uint32_t cycle_count)
    {
        return model->StepSingle(cycle_count);
    }

    void spect_dpi_set_change_reporting(uint32_t enable)
    {
        model->change_reporting_ = enable;
    }

    uint32_t spect_dpi_get_model_change(dpi_state_change_t *change)
    {
        if (model->change_q_.empty())
            return 1;

        *change = model->change_q_.front();
        model->change_q_.pop();
        return 0;
    }

    uint32_t spect_dpi_program_run(uint32_t instructions)
    {
        return model->Step(instructions);
    }
}