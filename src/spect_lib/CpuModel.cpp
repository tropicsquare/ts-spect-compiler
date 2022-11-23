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
#include <cstdarg>

#include "CpuModel.h"

#include "InstructionDefs.h"
#include "InstructionFactory.h"


spect::CpuModel::CpuModel(bool instr_mem_ahb_w, bool instr_mem_ahb_r) :
    instr_mem_ahb_w_(instr_mem_ahb_w),
    instr_mem_ahb_r_(instr_mem_ahb_r)
{
    memory_ = new uint32_t[SPECT_TOTAL_MEM_SIZE / 4];
    regs_ = new ordt_root();
    print_fnc = &(printf);
    Reset();
}

spect::CpuModel::~CpuModel()
{
    delete memory_;
    delete regs_;
}

void spect::CpuModel::Start()
{
    DebugInfo(VERBOSITY_LOW, "Starting program execution...");
    DebugInfo(VERBOSITY_LOW, "First instruction address:", tohexs(start_pc_, 4));
    SetPc(start_pc_);

    DebugInfo(VERBOSITY_MEDIUM, "SPECT is clearing STATUS[IDLE] = 0.");
    regs_->r_status.f_idle.data = 0;
    UpdateInterrupts();

    end_executed_ = false;

    // To make browsing logs easier
    DebugInfo(VERBOSITY_LOW, "");
}

void spect::CpuModel::Finish(int status_err)
{
    end_executed_ = true;
    DebugInfo(VERBOSITY_LOW, "Finishing program execution...");

    DebugInfo(VERBOSITY_MEDIUM, "SPECT setting STATUS[IDLE] = 1.");
    regs_->r_status.f_idle.data = 1;

    DebugInfo(VERBOSITY_MEDIUM, "SPECT setting STATUS[DONE] = 1.");
    regs_->r_status.f_done.data = 1;

    DebugInfo(VERBOSITY_MEDIUM, "SPECT setting STATUS[ERR] = ", status_err);
    regs_->r_status.f_err.data = status_err;

    DebugInfo(VERBOSITY_MEDIUM, "SPECT setting SRR = ", gpr_[31]);
    SetSrr(gpr_[31]);
}

bool spect::CpuModel::IsFinished()
{
    return end_executed_;
}

void spect::CpuModel::SetStartPc(uint16_t start_pc)
{
    DebugInfo(VERBOSITY_MEDIUM, "Setting Start PC to:", tohexs(start_pc, 4));
    start_pc_ = start_pc;
}

void spect::CpuModel::SetMemory(uint16_t address, uint32_t data)
{
    DebugInfo(VERBOSITY_MEDIUM, "Setting memory, address:", tohexs(address, 4),
                                 "data:", tohexs(data, 8));

    memory_[address >> 2] = data;

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data wdata(1, data);
        regs_->write(address - SPECT_CONFIG_REGS_BASE, wdata);
        UpdateInterrupts();
        UpdateRegisterEffects();
    }
}

uint32_t spect::CpuModel::GetMemory(uint16_t address)
{
    uint32_t rv = memory_[address >> 2];

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data rdata(1, 0);
        regs_->read(address - SPECT_CONFIG_REGS_BASE, rdata);
        rv = rdata[0];
    }

    DebugInfo(VERBOSITY_MEDIUM, "Getting memory, address:", tohexs(address, 4),
                                    "data:", tohexs(rv, 8));

    return rv;
}

uint32_t* spect::CpuModel::GetMemoryPtr()
{
    return memory_;
}

uint32_t spect::CpuModel::WriteMemoryAhb(uint16_t address, uint32_t data)
{
    DebugInfo(VERBOSITY_MEDIUM, "AHB Write", tohexs(address, 4), "data:", tohexs(data, 8));

    DEFINE_CHANGE(ch_mem, DPI_CHANGE_MEM, address);
    ch_mem.old_val[0] = memory_[address >> 2];
    uint32_t written = 0;

    if ( IsWithinMem(CpuMemory::DATA_RAM_IN, address) ||
        (IsWithinMem(CpuMemory::INSTR_MEM, address) && instr_mem_ahb_w_)) {
        memory_[address >> 2] = data;
        written = data;
    }

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data wdata(1, data);
        regs_->write(address - SPECT_CONFIG_REGS_BASE, wdata);
        UpdateInterrupts();
        UpdateRegisterEffects();

        ordt_data rdata(1, 0);
        regs_->read(address - SPECT_CONFIG_REGS_BASE, rdata);
        written = rdata[0];
    }

    ch_mem.new_val[0] = written;
    if (ch_mem.new_val[0] != ch_mem.old_val[0])
        ReportChange(ch_mem);

    return written;
}

uint32_t spect::CpuModel::ReadMemoryAhb(uint16_t address)
{
    uint32_t rv = 0;

    if ( IsWithinMem(CpuMemory::DATA_RAM_OUT, address) ||
        (IsWithinMem(CpuMemory::INSTR_MEM, address) && instr_mem_ahb_r_))
        rv = memory_[address >> 2];

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data rdata(1, 0);
        regs_->read(address - SPECT_CONFIG_REGS_BASE, rdata);
        rv = rdata[0];
    }

    DebugInfo(VERBOSITY_MEDIUM, "AHB Read", tohexs(address, 4), "data:", tohexs(rv, 8));
    return rv;
}

uint32_t spect::CpuModel::ReadMemoryCoreData(uint16_t address)
{
    uint32_t rv = 0;
    if (IsWithinMem(CpuMemory::DATA_RAM_IN, address) ||
        IsWithinMem(CpuMemory::CONST_ROM, address))
        rv = memory_[address >> 2];

    DebugInfo(VERBOSITY_MEDIUM, "Core Read", tohexs(address, 4), "data:", tohexs(rv, 8));

    return rv;
}

uint32_t spect::CpuModel::WriteMemoryCoreData(uint16_t address, uint32_t data)
{
    DebugInfo(VERBOSITY_MEDIUM, "Core Write", tohexs(address, 4), "data:", tohexs(data, 8));

    uint32_t written = 0;
    DEFINE_CHANGE(ch_mem, DPI_CHANGE_MEM, address);
    ch_mem.old_val[0] = memory_[address >> 2];

    if (IsWithinMem(CpuMemory::DATA_RAM_IN, address) ||
        IsWithinMem(CpuMemory::DATA_RAM_OUT, address)) {
        memory_[address >> 2] = data;
        written = data;
    }

    ch_mem.new_val[0] = written;
    ReportChange(ch_mem);

    return written;
}

uint32_t spect::CpuModel::ReadMemoryCoreFetch(uint16_t address)
{
    DebugInfo(VERBOSITY_MEDIUM, "Fetching instruction, address: ", tohexs(address, 4));
    if (IsWithinMem(CpuMemory::INSTR_MEM, address)) {
        return memory_[address >> 2];
    }
    return 0x0;
}

const uint256_t& spect::CpuModel::GetGpr(int index)
{
    return gpr_[index];
}

void spect::CpuModel::SetGpr(int index, const uint256_t &val)
{
    std::stringstream ss;
    ss << static_cast<CpuGpr>(index);
    DebugInfo(VERBOSITY_MEDIUM, "Setting", ss.str(), "to", tohexs(val));
    gpr_[index] = val;
}

uint16_t spect::CpuModel::GetPc()
{
    return pc_;
}

void spect::CpuModel::SetPc(uint16_t val)
{
    DebugInfo(VERBOSITY_MEDIUM, "Setting PC to", tohexs(val, 4));
    pc_ = val;
}

void spect::CpuModel::SetCpuFlag(CpuFlagType type, bool val)
{
    switch (type) {
    case CpuFlagType::ZERO:
        DebugInfo(VERBOSITY_MEDIUM, "Setting Z flag to", val);
        flags_.zero = val;
        break;
    case CpuFlagType::CARRY:
        DebugInfo(VERBOSITY_MEDIUM, "Setting C flag to", val);
        flags_.carry = val;
        break;
    default:
        break;
    }
}

bool spect::CpuModel::GetCpuFlag(CpuFlagType type)
{
    switch (type) {
    case CpuFlagType::ZERO:
        return flags_.zero;
    case CpuFlagType::CARRY:
        return flags_.carry;
    default:
        return false;
    }
}

spect::CpuFlags spect::CpuModel::GetCpuFlags()
{
    return flags_;
}

const uint256_t& spect::CpuModel::GetSrr()
{
    return srr_;
}

void spect::CpuModel::SetSrr(const uint256_t &val)
{
    DebugInfo(VERBOSITY_MEDIUM, "Setting SRR flag to", val);
    srr_ = val;
}

void spect::CpuModel::RarPush(uint16_t ret_addr)
{
    DebugInfo(VERBOSITY_MEDIUM, "Pushing", tohexs(ret_addr, 4), "to RAR stack.");

    if (GetRarSp() == SPECT_RAR_DEPTH)
        DebugInfo(VERBOSITY_LOW, "FATAL: RAR stack overflow");

    rar_stack_[rar_sp_] = ret_addr;
    SetRarSp(GetRarSp() + 1);
}

uint16_t spect::CpuModel::RarPop()
{
    if (GetRarSp() == 0)
        DebugInfo(VERBOSITY_LOW, "FATAL: RAR stack underflow");

    SetRarSp(GetRarSp() - 1);
    uint16_t rv = GetRarAt(GetRarSp());

    DebugInfo(VERBOSITY_MEDIUM, "Poping ", tohexs(rv, 4), "from RAR stack.");

    return rv;
}

uint16_t spect::CpuModel::GetRarAt(uint16_t index)
{
    return rar_stack_[index];
}

uint16_t spect::CpuModel::GetRarSp()
{
    return rar_sp_;
}

void spect::CpuModel::SetRarSp(uint16_t val)
{
    DebugInfo(VERBOSITY_MEDIUM, "Setting RAR SP to:", val);
    rar_sp_ = val;
}

bool spect::CpuModel::GetInterrrupt(CpuIntType int_type)
{
    switch (int_type) {
    case CpuIntType::INT_DONE:
        return int_done_;
    case CpuIntType::INT_ERR:
        return int_err_;
    default:
        break;
    }
    return false;
}

void spect::CpuModel::GrvQueuePush(uint32_t data)
{
    DebugInfo(VERBOSITY_HIGH, "Pushing to GRV queue:", tohexs(data, 8));
    grv_q_.push(data);
}

uint32_t spect::CpuModel::GrvQueuePop()
{
    uint32_t rv = 0;
    if (!grv_q_.empty()) {
        rv = grv_q_.front();
        grv_q_.pop();
        DebugInfo(VERBOSITY_HIGH, "Popping from GRV queue:", tohexs(rv, 8));
    } else
        DebugInfo(VERBOSITY_LOW, "Popping from empty GRV queue, returning 0");
    return rv;
}

void spect::CpuModel::GpkQueuePush(uint32_t index, uint32_t data)
{
    DebugInfo(VERBOSITY_HIGH, "Pushing to GPK queue", index, ":", data);
    gpk_q_[index].push(data);
}

uint32_t spect::CpuModel::GpkQueuePop(uint32_t index)
{
    uint32_t rv = 0;
    if (!gpk_q_[index].empty()) {
        rv = gpk_q_[index].front();
        gpk_q_[index].pop();
        DebugInfo(VERBOSITY_HIGH, "Popping from GPK queue", index, ":", tohexs(rv, 8));
    } else
        DebugInfo(VERBOSITY_LOW, "Popping from empty GPK queue", index, ": returning 0");
    return rv;
}

void spect::CpuModel::ReportChange(dpi_state_change_t change)
{
    if (change_reporting_) {
        DebugInfo(VERBOSITY_HIGH, "Pushing change to model change queue:");
        PrintChange(change);
        change_q_.push(change);
    }
}

dpi_state_change_t spect::CpuModel::ConsumeChange()
{
    dpi_state_change_t rv = {};

    if (!HasChange()) {
        DebugInfo(VERBOSITY_LOW, "WARNING: Change queue empty, nothing to Pop, returning invalid change!");
        return rv;
    }

    rv = change_q_.front();
    change_q_.pop();

    DebugInfo(VERBOSITY_HIGH, "Popping change from model change queue:");
    PrintChange(rv);

    return rv;
}

void spect::CpuModel::PrintChange(dpi_state_change_t change)
{
    DebugInfo(VERBOSITY_HIGH, "     kind:      ", dpi_change_kind_to_str(change.kind));
    DebugInfo(VERBOSITY_HIGH, "     obj:       ", dpi_change_obj_to_str(change.kind, change.obj));

    char buf[100];
    sprintf(buf, "old_val: %08x %08x %08x %08x %08x %08x %08x %08x",
                change.old_val[7], change.old_val[6], change.old_val[5], change.old_val[4],
                change.old_val[3], change.old_val[2], change.old_val[1], change.old_val[0]);
    DebugInfo(VERBOSITY_HIGH, "    ", buf);

    sprintf(buf, "new_val: %08x %08x %08x %08x %08x %08x %08x %08x",
                change.new_val[7], change.new_val[6], change.new_val[5], change.new_val[4],
                change.new_val[3], change.new_val[2], change.new_val[1], change.new_val[0]);
    DebugInfo(VERBOSITY_HIGH, "    ", buf);
}

void spect::CpuModel::PrintHashContext(uint32_t verbosity_level)
{
    for (int i = 0; i < 8; i++) {
        std::stringstream ss;
        ss << "W[" << i << "] = ";
        ss << tohexs(sha_512_.getContext(i), 16);
        DebugInfo(verbosity_level, ss.str().c_str());
    }
}

bool spect::CpuModel::HasChange()
{
    return !change_q_.empty();
}

void spect::CpuModel::GetLastInstruction(dpi_instruction_t *dpi_instr)
{
    memcpy(dpi_instr, &(last_instr), sizeof(dpi_instruction_t));
    std::cout << "C SIZE: " << sizeof(dpi_instruction_t) << "\n";
}

int spect::CpuModel::Step(int n)
{
    if (n > 0) {
        std::stringstream ss;
        ss << std::dec << n;
        DebugInfo(VERBOSITY_HIGH, "Executing", ss.str(), "instructions:");
    } else
        DebugInfo(VERBOSITY_HIGH, "Executing all instructions till end of program:");

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
    std::stringstream ss;
    ss << std::dec << cycles;
    DebugInfo(VERBOSITY_HIGH, "Executing single instruction in ", ss.str(), " RTL clock cycles:");

    return ExecuteNextInstruction(cycles);
}

void spect::CpuModel::Reset()
{
    DebugInfo(VERBOSITY_LOW, "Reseting CPU Model.");

    for (int i = 0; i < SPECT_GPR_CNT; i++)
        SetGpr(i, uint256_t("0x0"));

    SetCpuFlag(CpuFlagType::CARRY, false);
    SetCpuFlag(CpuFlagType::ZERO, false);
    SetSrr("0x0");

    for (int i = 0; i < SPECT_RAR_DEPTH; i++)
        rar_stack_[i] = 0x0;

    SetRarSp(0);
    end_executed_ = false;
    sha_512_ = Sha512();
    sha_512_.init();
    SetPc(0x0);

    // Re-create new register model -> Erase registers to reset values.
    delete regs_;
    regs_ = new ordt_root();

    // To make browsing logs easier
    DebugInfo(VERBOSITY_LOW, "");

    // Don't fill the content of memories intentionally!
    // This more realistically corresponds to un-inited memory having
    // all Xs in RTL sim. In model, content will be random based on previous
    // content of memory!
}

void spect::CpuModel::UpdateInterrupts()
{
    DebugInfo(VERBOSITY_LOW, "Updating CPU Interrupt values");

    ordt_data en(1,0);
    ordt_data status(1,0);

    DEFINE_CHANGE(ch_int_done, DPI_CHANGE_INT, DPI_SPECT_INT_DONE);
    DEFINE_CHANGE(ch_int_err, DPI_CHANGE_INT, DPI_SPECT_INT_ERR);
    ch_int_done.old_val[0] = int_done_;
    ch_int_err.old_val[0] = int_err_;

    int_done_ = (regs_->r_int_ena.f_int_done_en.data == 1 &&
                 regs_->r_status.f_done.data == 1);
    DebugInfo(VERBOSITY_MEDIUM, "Setting int_done     =", int_done_);

    int_err_ = (regs_->r_int_ena.f_int_err_en.data == 1 &&
                regs_->r_status.f_err.data == 1);
    DebugInfo(VERBOSITY_MEDIUM, "Setting int_err      =", int_err_);

    // Construct and report model change
    ch_int_done.new_val[0] = int_done_;
    ch_int_err.new_val[0] = int_err_;

    if (ch_int_done.old_val[0] != ch_int_done.new_val[0])
        ReportChange(ch_int_done);
    if (ch_int_err.old_val[0] != ch_int_err.new_val[0])
        ReportChange(ch_int_err);
}

void spect::CpuModel::UpdateRegisterEffects()
{
    DebugInfo(VERBOSITY_LOW, "Updating Register effects");

    // COMMAND[START] == 1
    if (regs_->r_command.f_start.data == 1) {
        DebugInfo(VERBOSITY_LOW, "Written COMMAND[START] = 1.");
        Start();
    }

    // COMMAND[SOFT_RESET] == 1
    if (regs_->r_command.f_soft_reset.data == 1) {
        DebugInfo(VERBOSITY_LOW, "Written COMMAND[SOFT_RESET] = 1.");
        Reset();
    }
}

int spect::CpuModel::ExecuteNextInstruction(int cycles)
{
    uint32_t wrd = ReadMemoryCoreFetch(GetPc());

    DebugInfo(VERBOSITY_MEDIUM, "Disassembling instruction:     ", tohexs(wrd, 8));
    Instruction *instr = spect::Instruction::DisAssemble(wrd);

    // Detect invalid instruction and finish
    if (instr == nullptr) {
        DebugInfo(VERBOSITY_LOW, "Detected invalid instruction!");
        Finish(1);
        UpdateInterrupts();

        return 0;
    }

    DebugInfo(VERBOSITY_LOW, "Executing instruction:         ", instr->Dump());

    // Sample input operands and values for DPI readout
    instr->SampleInputs(&(last_instr), this);

    // Execute instruction
    instr->model_ = this;
    if (instr->Execute())
        SetPc(GetPc() + 0x4);

    // Check last execution time of instruction with the same mnemonic
    // Hold execution time of instruction per-mnemonic in Instruction Factory.
    // Ignore cases where:
    //      1. Instruction is executed first time (gold->cycles_ == 0)
    //      2. We stop measurement for whatever reason (cycles == 0).
    //      3. Instruction has 'c_time' = false. Such instruction can last
    //         variable amount of clock cycles.
    int rv = 0;
    Instruction *gold = spect::InstructionFactory::GetInstruction(instr->mnemonic_);
    if (cycles > 0 && gold->cycles_ > 0 && cycles != gold->cycles_)
        rv = gold->cycles_;
    if (!gold->c_time_)
        rv = 0;

    gold->cycles_ = cycles;

    // Sample output operands and values for DPI readout
    instr->SampleOutputs(&(last_instr), this);

    // Separate instructions by empty line -> More readable output
    DebugInfo(VERBOSITY_MEDIUM, "");

    delete instr;
    return rv;
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

void spect::CpuModel::PrintArgs()
{
    print_fnc("\n");
}

template<typename Arg>
void spect::CpuModel::PrintArgs(Arg arg)
{
    std::stringstream ss;
    ss << std::hex << arg << std::endl;
    print_fnc(ss.str().c_str());
}

template<typename First, typename... Args>
void spect::CpuModel::PrintArgs(First first, Args... args)
{
    std::stringstream ss;
    ss << std::hex << first << " ";
    print_fnc(ss.str().c_str());
    PrintArgs(args...);
}

template<typename... Args>
void spect::CpuModel::DebugInfo(uint32_t verbosity_level, const Args ...args)
{
    if (verbosity_ >= verbosity_level)
        PrintArgs(MODEL_LABEL, args...);
}