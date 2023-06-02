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

    DebugInfo(VERBOSITY_MEDIUM, "SPECT is clearing COMMAND[START] = 0.");
    regs_->r_command.f_start.data = 0;

    DebugInfo(VERBOSITY_MEDIUM, "SPECT is clearing STATUS[IDLE] = 0.");
    regs_->r_status.f_idle.data = 0;
    UpdateInterrupts();

    end_executed_ = false;

    // Erase track of execution count
    auto it = spect::InstructionFactory::GetInstructionIterator();
    for (; !spect::InstructionFactory::IteratorIsLast(it); it++) {
        Instruction *instr = it->second;
        instr->exec_cnt_ = 0;
    }

    // Erase track of number of executed instructions
    instr_cnt_ = 0;

    // To make browsing logs easier
    DebugInfo(VERBOSITY_LOW, "");
}

void spect::CpuModel::Finish(int status_err)
{
    end_executed_ = true;
    DebugInfo(VERBOSITY_LOW, "Finishing program execution...");

    DebugInfo(VERBOSITY_MEDIUM, "SPECT setting STATUS[IDLE] = 1.");
    regs_->r_status.f_idle.data = 1;

    DebugInfo(VERBOSITY_MEDIUM, "SPECT setting STATUS[DONE] = ", !status_err);
    regs_->r_status.f_done.data = !status_err;

    DebugInfo(VERBOSITY_MEDIUM, "SPECT setting STATUS[ERR] = ", status_err);
    regs_->r_status.f_err.data = status_err;


    DebugInfo(VERBOSITY_HIGH, "Program statistics:");
    DebugInfo(VERBOSITY_HIGH, "     Instruction         No. of Executions       Cycles per instruction");
    auto it = spect::InstructionFactory::GetInstructionIterator();
    for (; !spect::InstructionFactory::IteratorIsLast(it); it++) {
        Instruction *instr = it->second;
        if (instr->exec_cnt_ > 0) {
            char buf[128];
            sprintf(buf, "   %10s                   %4lu                        %4d", instr->mnemonic_.c_str(),
                    instr->exec_cnt_, instr->cycles_);
            DebugInfo(VERBOSITY_HIGH, buf);
        }
    }
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

void spect::CpuModel::WriteMemoryAhb(uint16_t address, uint32_t data)
{
    DebugInfo(VERBOSITY_MEDIUM, "AHB Write", tohexs(address, 4), "data:", tohexs(data, 8));

    DEFINE_CHANGE(ch_mem, DPI_CHANGE_MEM, address);

    if ( IsWithinMem(CpuMemory::DATA_RAM_IN, address) ||
        (IsWithinMem(CpuMemory::INSTR_MEM, address) && instr_mem_ahb_w_)) {
        ch_mem.old_val[0] = memory_[address >> 2];
        memory_[address >> 2] = data;
        ch_mem.new_val[0] = data;
        ReportChange(ch_mem);
    }

    if (IsWithinMem(CpuMemory::CONFIG_REGS, address)) {
        ordt_data wdata(1, data);
        regs_->write(address - SPECT_CONFIG_REGS_BASE, wdata);
        UpdateInterrupts();
        UpdateRegisterEffects();
    }
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

    if (IsWithinMem(CpuMemory::EMEM_IN, address)) {
        DEFINE_CHANGE(ch_emem, DPI_CHANGE_EMEM_IN, address);
        rv = memory_[address >> 2];
        ReportChange(ch_emem);
    }

    DebugInfo(VERBOSITY_MEDIUM, "Core Read", tohexs(address, 4), "data:", tohexs(rv, 8));

    return rv;
}

void spect::CpuModel::WriteMemoryCoreData(uint16_t address, uint32_t data)
{
    DebugInfo(VERBOSITY_MEDIUM, "Core Write", tohexs(address, 4), "data:", tohexs(data, 8));

    if (IsWithinMem(CpuMemory::DATA_RAM_IN, address) ||
        IsWithinMem(CpuMemory::DATA_RAM_OUT, address)) {
        DEFINE_CHANGE(ch_mem, DPI_CHANGE_MEM, address);
        ch_mem.old_val[0] = memory_[address >> 2];
        memory_[address >> 2] = data;
        ch_mem.new_val[0] = data;
        ReportChange(ch_mem);
    }

    if (IsWithinMem(CpuMemory::EMEM_OUT, address)) {
        DEFINE_CHANGE(ch_emem, DPI_CHANGE_EMEM_OUT, address);
        ch_emem.new_val[0] = data;
        ReportChange(ch_emem);
    }
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
    case CpuFlagType::ERROR:
        DebugInfo(VERBOSITY_MEDIUM, "Setting E flag to", val);
        flags_.error = val;
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
    case CpuFlagType::ERROR:
        return flags_.error;
    default:
        return false;
    }
}

spect::CpuFlags spect::CpuModel::GetCpuFlags()
{
    return flags_;
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

void spect::CpuModel::SetRarAt(uint16_t index, uint16_t val)
{
    rar_stack_[index] = val;
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

void spect::CpuModel::SetParityType(ParityType type)
{
    DebugInfo(VERBOSITY_MEDIUM, "Setting Parity Type to:", type);
    parity_type_ = type;
}

spect::ParityType spect::CpuModel::GetParityType()
{
    return parity_type_;
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
        DebugInfo(VERBOSITY_LOW, "Popping from empty GRV queue, GRV returns 0");
    return rv;
}

void spect::CpuModel::LdkQueuePush(uint32_t data)
{
    DebugInfo(VERBOSITY_HIGH, "Pushing to LDK queue:", data);
    ldk_q_.push(data);
}

uint32_t spect::CpuModel::LdkQueuePop()
{
    uint32_t rv = 0;
    if (!ldk_q_.empty()) {
        rv = ldk_q_.front();
        ldk_q_.pop();
        DebugInfo(VERBOSITY_HIGH, "Popping from LDK queue:", tohexs(rv, 8));
    } else
        DebugInfo(VERBOSITY_LOW, "Popping from empty LDK queue, LDK returns 0");
    return rv;
}

void spect::CpuModel::KbusErrorQueuePush(bool error)
{
    DebugInfo(VERBOSITY_HIGH, "Pushing to KBUS Error queue:", error);
    kbus_error_q_.push(error);
}

bool spect::CpuModel::KbusErrorQueuePop()
{
    bool rv = false;
    if (!kbus_error_q_.empty()) {
        rv = kbus_error_q_.front();
        kbus_error_q_.pop();
        DebugInfo(VERBOSITY_HIGH, "Popping from KBUS Error queue:", rv);
    } else
        DebugInfo(VERBOSITY_LOW, "Popping from empty KBUS Error queue, returns false");
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

#define PUT_COMMENT_LINE(comment)           \
    ofs << std::string(80, '*') << "\n";    \
    ofs << comment << "\n";                 \
    ofs << std::string(80, '*') << "\n";

#define SKIP_COMMENT_LINES          \
    for (int i = 0; i < 3; i++)     \
        std::getline(ifs, line);

void spect::CpuModel::DumpContext(const std::string &path)
{
    std::ofstream ofs;
    ofs.open(path);

    if (ofs.is_open()) {
        DebugInfo(VERBOSITY_LOW, "Dumping model context to: ", path);

        ofs << std::hex;
        ofs << std::setfill('0');

        PUT_COMMENT_LINE("GPR registers:");
        for (int i = 0; i < 32; i++)
            ofs << std::setw(64) << GetGpr(i) << "\n";

        PUT_COMMENT_LINE("SHA 512 context:");
        for (int i = 0; i < 8; i++)
            ofs << std::setw(16) << sha_512_.getContext(i) << "\n";

        PUT_COMMENT_LINE("RAR stack:");
        for (int i = 0; i < SPECT_RAR_DEPTH; i++)
            ofs << std::setw(4) << GetRarAt(i) << "\n";

        PUT_COMMENT_LINE("RAR stack pointer:");
        ofs << std::setw(4) << GetRarSp() << "\n";

        PUT_COMMENT_LINE("FLAGS (Z, C, E):");
        CpuFlags flgs = GetCpuFlags();
        ofs << flgs.zero << "\n";
        ofs << flgs.carry << "\n";
        ofs << flgs.error << "\n";

        PUT_COMMENT_LINE("Memory:");
        uint32_t backup = verbosity_;
        for (int i = 0; i < (SPECT_TOTAL_MEM_SIZE / 4) ; i++) {
            if (i == 10) {
                int num_accesses = (SPECT_TOTAL_MEM_SIZE / 4) - 10;
                DebugInfo(VERBOSITY_LOW, "Executed", std::to_string(num_accesses),
                        "further acesses to memory that were not printed...");
                verbosity_ = 0;
            }
            ofs << std::setw(8) << GetMemory(i * 4) << "\n";
        }
        verbosity_ = backup;

        DebugInfo(VERBOSITY_LOW, "Finished Dumping model context.");
        DebugInfo(VERBOSITY_LOW, "\n");

        ofs.close();
    } else
        throw std::runtime_error("Unable to open a file: " + path);
}

void spect::CpuModel::LoadContext(const std::string &path)
{
    std::ifstream ifs(path);
    std::string line;

    if (ifs.is_open()) {
        DebugInfo(VERBOSITY_LOW, "Loading model context from: ", path);

        // GPRs
        SKIP_COMMENT_LINES
        for (int i = 0; i < 32; i++) {
            std::getline(ifs, line);
            std::string num = std::string("0x") + line;
            SetGpr(i, uint256_t(num.c_str()));
        }

        // SHA512
        SKIP_COMMENT_LINES
        for (int i = 0; i < 8; i++) {
            uint64_t num;
            std::getline(ifs, line);
            std::istringstream iss(line);
            iss >> std::hex >> num;
            DebugInfo(VERBOSITY_LOW, "Setting SHA512 context (", i, ") to 0x", line.c_str());
            sha_512_.setContext(i, num);
        }

        // RAR stack
        SKIP_COMMENT_LINES
        for (int i = 0; i < SPECT_RAR_DEPTH; i++) {
            std::getline(ifs, line);
            uint16_t num;
            std::istringstream iss(line);
            iss >> std::hex >> num;
            SetRarAt(i, num);
        }

        // RAR stack pointer
        SKIP_COMMENT_LINES
        std::getline(ifs, line);
        uint16_t num;
        std::istringstream iss(line);
        iss >> std::hex >> num;
        SetRarSp(num);

        // Flags
        SKIP_COMMENT_LINES
        bool val;

        std::getline(ifs, line);
        std::istringstream iss2(line);
        iss2 >> val;
        SetCpuFlag(CpuFlagType::ZERO, val);

        std::getline(ifs, line);
        std::istringstream iss3(line);
        iss3 >> val;
        SetCpuFlag(CpuFlagType::CARRY, val);

        std::getline(ifs, line);
        std::istringstream iss4(line);
        iss4 >> val;
        SetCpuFlag(CpuFlagType::ERROR, val);

        // Memory
        SKIP_COMMENT_LINES
        uint32_t backup = verbosity_;
        for (int i = 0; i < (SPECT_TOTAL_MEM_SIZE / 4) ; i++) {
            if (i == 10) {
                int num_accesses = (SPECT_TOTAL_MEM_SIZE / 4) - 10;
                DebugInfo(VERBOSITY_LOW, "Executed", std::to_string(num_accesses),
                        "further acesses to memory that were not printed...");
                verbosity_ = 0;
            }
            uint32_t mem_val;
            std::getline(ifs, line);
            std::istringstream iss5(line);
            iss5 >> std::hex >> mem_val;
            SetMemory(i * 4, mem_val);
        }
        verbosity_ = backup;

        DebugInfo(VERBOSITY_LOW, "Finished Loading model context.");
        DebugInfo(VERBOSITY_LOW, "\n");

        ifs.close();
    } else
        throw std::runtime_error("Unable to open a file: " + path);
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
    SetCpuFlag(CpuFlagType::ZERO,  false);
    SetCpuFlag(CpuFlagType::ERROR, false);

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
    Instruction *instr = spect::Instruction::DisAssemble(GetParityType(), wrd);

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
    gold->exec_cnt_++;

    // Sample output operands and values for DPI readout
    instr->SampleOutputs(&(last_instr), this);

    // Check number of executed instructions
    instr_cnt_++;
    if (instr_cnt_ == max_instr_cnt_) {
        DebugInfo(VERBOSITY_LOW, "Limit of executed instruction (", max_instr_cnt_, ") reached!");
        Finish(1);
        UpdateInterrupts();
        return 0;
    }

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
        break;
    case CpuMemory::EMEM_IN:
        start = SPECT_EMEM_IN_BASE;
        end = start + SPECT_EMEM_IN_SIZE;
        break;
    case CpuMemory::EMEM_OUT:
        start = SPECT_EMEM_OUT_BASE;
        end = start + SPECT_EMEM_OUT_SIZE;
        break;
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
