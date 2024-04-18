/**************************************************************************************************
** SPECT Instruction set simulator - Shared Library version
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include "spect_iss_lib.h"


spect::CpuSimulator *simulator;

uint32_t first_addr_ = SPECT_INSTR_MEM_BASE;
spect::ParityType parity_type_ = spect::ParityType::NONE;


void spect_iss_init(int isa_version)
{
    spect::InstructionFactory::SetActiveISAVersion(isa_version);
    simulator = new spect::CpuSimulator();
    simulator->model_->SetParityType(parity_type_);
}

void spect_iss_set_first_addr(int first_addr)
{
    first_addr_ = first_addr;
}

void spect_iss_load_s_file(std::string s_file)
{
    simulator->compiler_->CompileInit(first_addr_);
    simulator->compiler_->Compile(s_file);
    simulator->compiler_->CompileFinish();

    uint32_t *p_start = simulator->model_->GetMemoryPtr();
    p_start += (first_addr_ >> 2);
    simulator->compiler_->program_->Assemble(p_start, parity_type_);

    // Set address of first instruction to be fetched
    uint32_t start_pc = simulator->compiler_->symbols_->GetSymbol(START_SYMBOL)->val_;
    simulator->model_->SetStartPc(start_pc);
}

void spect_iss_set_parity_type(int parity_type)
{
    if (parity_type == 1)
        parity_type_ = spect::ParityType::ODD;
    else if (parity_type == 2)
        parity_type_ = spect::ParityType::EVEN;
}

void spect_iss_load_hex_file(std::string hex_file)
{
    spect::HexHandler::LoadHexFile(hex_file,
        simulator->model_->GetMemoryPtr(), SPECT_INSTR_MEM_BASE);
}

void spect_iss_set_const_rom_hex_file(std::string const_rom_hex_file)
{
    spect::HexHandler::LoadHexFile(const_rom_hex_file,
        simulator->model_->GetMemoryPtr(), SPECT_CONST_ROM_BASE);
}

void spect_iss_set_data_ram_in_hex_file(std::string data_ram_out_hex_file)
{
    spect::HexHandler::LoadHexFile(data_ram_out_hex_file,
        simulator->model_->GetMemoryPtr(), SPECT_DATA_RAM_IN_BASE);
}

void spect_iss_set_emem_in_hex_file(std::string emem_in_hex_file)
{
    spect::HexHandler::LoadHexFile(emem_in_hex_file,
        simulator->model_->GetMemoryPtr(), SPECT_EMEM_IN_BASE);
}

void spect_iss_set_timing_accurate(bool enable, int exec_time_step)
{
    simulator->model_->timing_accurate_sim_ = enable;
    simulator->model_->execution_time_step_ = exec_time_step;
}

void spect_iss_set_grv_hex_file(std::string grv_hex_file)
{
    std::vector<uint32_t> mem;
    spect::HexHandler::LoadHexFile(grv_hex_file, mem);

    for (const auto &wrd : mem)
        simulator->model_->GrvQueuePush(wrd);
}

void spect_iss_set_key_mem_hex_file(std::string key_mem_file)
{
    simulator->key_memory_->Load(key_mem_file);
}

void spect_iss_set_start_pc(int start_pc)
{
    simulator->model_->SetStartPc(start_pc);
}

void spect_iss_execute_cmd_file(std::string cmd_file)
{
    cli::LoopScheduler scheduler;
    cli::CliLocalTerminalSession session(*(simulator->cli_), scheduler, std::cout);

    session.ExitAction(
            [&scheduler](A_UNUSED auto& out)
            {
                scheduler.Stop();
            }
        );

    simulator->cmd_file_ = cmd_file;
    simulator->ExecCmdFile(session);

    session.Feed("exit\n");
}

void spect_iss_cmd_start(std::ostream &out)
{
    simulator->CmdStart(out);
}

void spect_iss_cmd_info(std::ostream &out, std::string arg1)
{
    simulator->CmdInfo(out, arg1);
}

void spect_iss_cmd_run(std::ostream &out)
{
    simulator->CmdRun(out);
}

void spect_iss_cmd_delete(std::ostream &out, std::string arg1, bool all)
{
    simulator->CmdDelete(out, arg1, all);
}

void spect_iss_cmd_jump(std::ostream &out, std::string arg1)
{
    simulator->CmdJump(out, arg1);
}

void spect_iss_cmd_get(std::ostream &out, std::string arg1)
{
    simulator->CmdGet(out, arg1);
}

void spect_iss_cmd_load(std::ostream &out, std::string arg1, int offset)
{
    simulator->CmdLoad(out, arg1, offset);
}

void spect_iss_cmd_dump(std::ostream &out, std::string arg1, uint32_t address, uint32_t size)
{
    simulator->CmdDump(out, arg1, address, size);
}

void spect_iss_dump_data_ram_out_hex(std::string data_ram_out_hex_file)
{
    spect::HexHandler::DumpHexFile(data_ram_out_hex_file,
            spect::HexFileType::ISS_WORD, simulator->model_->GetMemoryPtr(),
            SPECT_DATA_RAM_OUT_BASE, SPECT_DATA_RAM_OUT_SIZE);
}

void spect_iss_exit(void)
{
    delete simulator;
}

int spect_iss_get_git_hash(void)
{
    std::stringstream ss;
    ss << std::hex;
    ss << std::string(TOOL_VERSION_HASH);
    int ver;
    ss >> ver;

    return ver;
}

std::string spect_iss_get_version(void)
{
    return std::string(TOOL_VERSION_TAG);
}