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

#include <fstream>

#include "CpuModel.h"

#include "InstructionDefs.h"
#include "InstructionFactory.h"


spect::CpuModel::CpuModel(bool instr_mem_rw) :
    instr_mem_rw_(instr_mem_rw)
{
    memory_ = new uint32_t[SPECT_TOTAL_MEM_SIZE / 4];
    regs_ = new ordt_root(SPECT_CONFIG_REGS_BASE, SPECT_CONFIG_REGS_BASE + SPECT_CONFIG_REGS_SIZE);
    Reset();
}

spect::CpuModel::~CpuModel()
{
    delete memory_;
    delete regs_;
}

void spect::CpuModel::Start()
{
    pc_ = start_pc_;

    ordt_data wdata(1, 0);
    regs_->r_status.f_idle.write(wdata);
    UpdateInterrupts();
}

void spect::CpuModel::SetMemory(uint16_t address, uint32_t data)
{
    memory_[address >> 2] = data;

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data wdata(1, 0);
        wdata[0] = data;
        regs_->write(address, wdata);
        UpdateInterrupts();
        UpdateRegisterEffects();
    }
}

uint32_t spect::CpuModel::GetMemory(uint16_t address)
{
    return memory_[address >> 2];

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data rdata(1, 0);
        regs_->read(address, rdata);
        return rdata[0];
    }
}

void spect::CpuModel::WriteMemoryAhb(uint16_t address, uint32_t data)
{
    if ( IsWithinMem(CpuMemory::DATA_RAM_IN, address) ||
        (IsWithinMem(CpuMemory::INSTR_MEM, address) && instr_mem_rw_))
        memory_[address >> 2] = data;

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data wdata(1, 0);
        wdata[0] = data;
        regs_->write(address, wdata);
        UpdateInterrupts();
        UpdateRegisterEffects();
    }
}

uint32_t spect::CpuModel::ReadMemoryAhb(uint16_t address)
{
    if ( IsWithinMem(CpuMemory::DATA_RAM_OUT, address) ||
        (IsWithinMem(CpuMemory::INSTR_MEM, address) && instr_mem_rw_))
        return memory_[address >> 2];

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data rdata(1, 0);
        regs_->read(address, rdata);
        return rdata[0];
    }

    return 0x0;
}

uint32_t spect::CpuModel::ReadMemoryCoreData(uint16_t address)
{
    if (IsWithinMem(CpuMemory::DATA_RAM_IN, address) ||
        IsWithinMem(CpuMemory::CONST_ROM, address))
        return memory_[address >> 2];

    return 0x0;
}

uint32_t spect::CpuModel::WriteMemoryCoreData(uint16_t address, uint32_t data)
{
    if (IsWithinMem(CpuMemory::DATA_RAM_IN, address) ||
        IsWithinMem(CpuMemory::DATA_RAM_OUT, address)) {
        memory_[address >> 2] = data;
    }
    return memory_[address >> 2];
}

uint32_t spect::CpuModel::ReadMemoryCoreFetch(uint16_t address)
{
    if (IsWithinMem(CpuMemory::INSTR_MEM, address)) {
        return memory_[address >> 2];
    }
    return 0x0;
}

int spect::CpuModel::Step(int n)
{
    int cnt = 0;
    if (n == 0) {
        do {
            ExecuteNextInstruction(0);
            cnt++;
        } while (!end_executed_);
    } else {
        for (int i = 0; i < n; i++) {
            ExecuteNextInstruction(0);
            cnt++;
            if (end_executed_)
                break;
        }
    }
    return cnt;
}

int spect::CpuModel::StepSingle(int cycles)
{
    return ExecuteNextInstruction(cycles);
}

void spect::CpuModel::FillMemory(CpuMemory mem, uint32_t val)
{
    int size;
    uint32_t *mem_ptr = MemToPtrs(mem, &size);

    for (int i = 0; i < size; i++) {
        mem_ptr[i] = val;
        mem_ptr++;
    }
}

uint32_t *spect::CpuModel::MemToPtrs(CpuMemory mem, int *size)
{
    switch (mem) {
    case CpuMemory::DATA_RAM_IN:
        *size = SPECT_DATA_RAM_IN_SIZE >> 2;
        return &(memory_[SPECT_DATA_RAM_IN_BASE >> 2]);
        break;
    case CpuMemory::DATA_RAM_OUT:
        *size = SPECT_DATA_RAM_OUT_SIZE >> 2;
        return &(memory_[SPECT_DATA_RAM_OUT_BASE >> 2]);
        break;
    case CpuMemory::CONST_ROM:
        *size = SPECT_CONST_ROM_SIZE >> 2;
        return &(memory_[SPECT_CONST_ROM_BASE >> 2]);
        break;
    case CpuMemory::INSTR_MEM:
        *size = SPECT_INSTR_MEM_SIZE >> 2;
        return &(memory_[SPECT_INSTR_MEM_BASE >> 2]);
        break;
    case CpuMemory::CONFIG_REGS:
        *size = SPECT_CONFIG_REGS_SIZE >> 2;
        return &(memory_[SPECT_CONFIG_REGS_BASE >> 2]);
        break;
    default:
        return NULL;
    }
}

bool spect::CpuModel::IsWithinMem(CpuMemory mem, uint16_t address)
{
    uint16_t start;
    uint16_t end;

    switch (mem) {
    case CpuMemory::DATA_RAM_IN:
        start = SPECT_DATA_RAM_IN_BASE;
        end = start + SPECT_DATA_RAM_IN_SIZE;
        break;
    case CpuMemory::DATA_RAM_OUT:
        start = SPECT_DATA_RAM_OUT_BASE;
        end = start + SPECT_DATA_RAM_OUT_SIZE;
        break;
    case CpuMemory::CONST_ROM:
        start = SPECT_CONST_ROM_BASE;
        end = start + SPECT_CONST_ROM_SIZE;
        break;
    case CpuMemory::INSTR_MEM:
        start = SPECT_INSTR_MEM_BASE;
        end = start + SPECT_INSTR_MEM_SIZE;
        break;
    case CpuMemory::CONFIG_REGS:
         start = SPECT_CONFIG_REGS_BASE;
         end = start + SPECT_CONFIG_REGS_SIZE;
    default:
        break;
    }

    if ((address >= start) && (address < end))
        return true;

    return false;
}

void spect::CpuModel::Reset()
{
    for (int i = 0; i < SPECT_GPR_CNT; i++)
        gpr_[i] = uint256_t("0x0");

    flags_.carry = false;
    flags_.zero = false;

    srr_ = uint256_t("0x0");

    for (int i = 0; i < SPECT_RAR_DEPTH; i++)
        rar_stack_[i] = 0x0;

    rar_sp_ = 0;
    end_executed_ = false;
    sha_512_ = Sha512();
    pc_ = 0x0;

    // Re-create new register model -> Erase registers to reset values.
    delete regs_;
    regs_ = new ordt_root(SPECT_CONFIG_REGS_BASE, SPECT_CONFIG_REGS_BASE + SPECT_CONFIG_REGS_SIZE);

    // Don't fill the content of memories intentionally!
    // This more realistically corresponds to un-inited memory having
    // all Xs in RTL sim. In model, content will be random based on previous
    // content of memory!
}

void spect::CpuModel::UpdateInterrupts()
{
    ordt_data en(1,0);
    ordt_data status(1,0);

    DEFINE_CHANGE(ch_int_done, DPI_CHANGE_INT, DPI_SPECT_INT_DONE);
    DEFINE_CHANGE(ch_int_err, DPI_CHANGE_INT, DPI_SPECT_INT_ERR);
    if (change_reporting_) {
        ordt_data rdata(1, 0);
        regs_->r_status.f_done.read(rdata);
        ch_int_done.old_val[0] = rdata[0];
        regs_->r_status.f_err.read(rdata);
        ch_int_err.old_val[0] = rdata[0];
    }

    regs_->r_status.f_done.read(status);
    regs_->r_int_ena.f_int_done_en.read(en);
    int_done_ = (en[0] == 1 && status[0] == 1);

    regs_->r_status.f_err.read(status);
    regs_->r_int_ena.f_int_err_en.read(en);
    int_err_ = (en[0] == 1 && status[0] == 1);

    if (change_reporting_) {
        ordt_data rdata(1, 0);
        regs_->r_status.f_done.read(rdata);
        ch_int_done.new_val[0] = rdata[0];
        regs_->r_status.f_err.read(rdata);
        ch_int_err.new_val[0] = rdata[0];
        change_q_.push(ch_int_done);
        change_q_.push(ch_int_err);
    }
}

void spect::CpuModel::UpdateRegisterEffects()
{
    // COMMAND[START] == 1
    ordt_data val(1,0);
    regs_->r_command.f_start.read(val);
    if (val[0] == 1)
        Start();

    // COMMAND[SOFT_RESET] == 1
    regs_->r_command.f_start.read(val);
    if (val[0] == 1)
        Reset();
}

int spect::CpuModel::ExecuteNextInstruction(int cycles)
{
    uint32_t wrd = ReadMemoryCoreFetch(pc_);
    Instruction *instr = spect::Instruction::DisAssemble(wrd);

    // Detect invalid instruction and finish
    if (instr == nullptr) {
        ordt_data wdata(1,0);
        wdata[0] = 1;
        regs_->r_status.f_idle.write(wdata);
        regs_->r_status.f_err.write(wdata);
        UpdateInterrupts();
        end_executed_ = true;
    }

    // Execute instruction
    instr->model_ = this;
    std::cout << std::hex << "Executing: 0x" << pc_ << ": ";
    instr->Dump(std::cout);
    std::cout << std::endl;
    if (instr->Execute())
        pc_ += 4;

    // Check last execution time of instruction with the same mnemonic
    // Hold execution time of instruction per-mnemonic in Instruction Factory.
    // Ignore cases where instruction is executed first time (gold->cycles_ == 0)
    // or we stop measurement for whatever reason (cycles == 0).
    int rv = 0;
    Instruction *gold = spect::InstructionFactory::GetInstruction(instr->mnemonic_);
    if (cycles > 0 && gold->cycles_ > 0 && cycles != gold->cycles_)
        rv = gold->cycles_;

    gold->cycles_ = cycles;

    delete instr;
    return rv;
}