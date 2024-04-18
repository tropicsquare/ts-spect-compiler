/**************************************************************************************************
** SPECT Instruction set simulator - Shared Library version
**
** Use API from this file to embed "spect_iss" into your compiled application
**
***************************************************************************************************
**
** Operation protocol consists of following phases:
**      1. Initialization
**      2. Program execution
**      3. Finalization (Dump state of the device and cleanup)
**
** Initialization:
**
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#ifndef SPECT_ISS_LIB_H_
#define SPECT_ISS_LIB_H_

#include <string>

#include "Compiler.h"
#include "CpuModel.h"
#include "CpuProgram.h"
#include "CpuSimulator.h"
#include "HexHandler.h"
#include "InstructionFactory.h"
#include "KeyMemory.h"


/**************************************************************************************************
 **************************************************************************************************
 * Initialization
 **************************************************************************************************
 *************************************************************************************************/

/**
 * @brief Initialzie SPECT Instruction Set Simulator
 *
 * @param isa_version SPECT Instruction Set version. Valid values:
 *                      1 - For SPECT design spec version <= 1.0 (TROPIC01 MPW1)
 *                      2 - For SPECT design spec version > 1.0
 */
void spect_iss_init(int isa_version);

/**
 * @brief Set address where first instruction of compiled programm will be placed.
 *
 * @param first_addr Absolute byte oriented adress where instruction will be placed.
 * @note If not called, Compiler will place first instruction at the beginning of
 *       SPECT Instruction Memory.
 * @note Equivalent to "--first-addr" argument of "spect_iss"
 */
void spect_iss_set_first_addr(int first_addr);

/**
 * @brief Compile ".s" SPECT FW file and load to Instruction memory
 *
 * @param s_file path to the SPECT ".s" file
 * @note Equivalent to "--program" argument of "spect_iss"
 */
void spect_iss_load_s_file(std::string s_file);

/**
 * @brief Set Parity type of SPECT Instruction Memory
 *
 * @param parity_type Instruction parity:
 *                      1 - Odd parity
 *                      2 - Even parity

 * @note Equivalent to "--parity" argument of "spect_iss"
 */
void spect_iss_set_parity_type(int parity_type);

/**
 * @brief Load ".hex" SPECT FW file to Instruction Memory
 *
 * @param hex_file HEX file to load
 * @note Equivalent to "--instruction-mem" argument of "spect_iss"
 */
void spect_iss_load_hex_file(std::string hex_file);

/**
 * @brief Load ".hex" file to SPECT Constant ROM.
 *
 * @param hex_file Const ROM HEX file to load
 * @note Equivalent to "--const-rom" argument of "spect_iss"
 */
void spect_iss_set_const_rom_hex_file(std::string const_rom_hex_file);

/**
 * @brief Load ".hex" file to SPECT Data RAM IN
 *
 * @param hex_file Data RAM Out HEX file to load
 * @note Equivalent to "--data-ram-in" argument of "spect_iss"
 */
void spect_iss_set_data_ram_in_hex_file(std::string data_ram_out_hex_file);

/**
 * @brief Load ".hex" file to SPECT External Memory IN
 *
 * @param hex_file External Memory In HEX file to load
 * @note Equivalent to "--emem-in" argument of "spect_iss"
 */
void spect_iss_set_emem_in_hex_file(std::string emem_in_hex_file);

/**
 * @brief Set maximal number of instructions to be executed before
 *        quitting.
 *
 * @param max_instr_cnt Maximal number of instructions to be executed.
 * @note This is simulator parameter only to avoid infinite loops stucking
 *       the program when un-intentional dead-loop is being simulated.
 *       There is no instruction limit in HW!
 * @note Equivalent to "--max-instr-cnt" argument of "spect_iss"
 */
void spect_iss_set_max_instr_cnt(int max_instr_cnt);

/**
 * @brief Configure "timing accurate" mode of SPECT Instruction Simulator.
 *
 * In timing accurate mode, each instruction execution consumes time that
 * is proportional relative to its actual HW execution.
 *
 * @param enable True - Enable "timing accurate" mode,
 *               False - Disable "timing accurate" mode
 * @param exec_time_step Execution time step (multiples of 1us for each
 *                       clock cycle instruction takes in HW).
 * @note Equivalent to "--timing-accurate" and "--exec-time-step" arguments of "spect_iss"
 */
void spect_iss_set_timing_accurate(bool enable, int exec_time_step);

/**
 * @brief Set random values to be read by GRV (Get Random Value) instruction.
 *        Each GRV instruction returns 256 bytes.
 * @param grv_hex_file Hex file to be pushed.
 * @note Equivalent to "--grv-hex" arguments of "spect_iss"
 */
void spect_iss_set_grv_hex_file(std::string grv_hex_file);

/**
 * @brief Set content of Key memory model (Accessed by KBO and LDK instructions).
 *
 * @param key_mem_file Hex file of Key Memory to be loaded.
 * @note Equivalent to "--load-keymem" arguments of "spect_iss"
 */
void spect_iss_set_key_mem_hex_file(std::string key_mem_file);

/**
 * @brief Set address of first instruction to be fetched.
 *
 * @param start_pc Adress of first instruction to be fetched.
 * @note Equivalent to "--start-pc" arguments of "spect_iss"
 */
void spect_iss_set_start_pc(int start_pc);


/**************************************************************************************************
 **************************************************************************************************
 * Program execution
 **************************************************************************************************
 *************************************************************************************************/

/**
 * @brief Execute SPECT Instruction set simulator command file
 *
 * @note Equivalent to "--cmd-file" arguments of "spect_iss"
 */
void spect_iss_execute_cmd_file(std::string cmd_file);

/**
 * @brief Start CPU program execution.
 *
 * @param out Output stream where to print the command outptut
 * @note Equivalent to "start" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_start(std::ostream &out);

/**
 * @brief Get information about state of Cpu Simulator
 *
 * @param out Output stream where to print the command outptut
 * @param arg1 Shall be one of:
 *                - breakpoints     : Breakpoints
 *                - registers       : CPU Registers
 *                - flags           : CPU Flags
 *                - pc              : Program counter
 *                - rar             : Return address register stack
 *                - symbols         : Symbol table
 *
 * @note Equivalent to "info" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_info(std::ostream &out, std::string arg1);

/**
 * @brief Add breakpoint to the program
 *
 * @param out Output stream where to print the command outptut
 * @param arg1 Shall be one of:
 *                  - break <label>       - Put breakpoint at position of <label>\n"
 *                  - break address       - Put breakpoint at absolute address\n"
 *                  - break -+number      - Put breakpoint +- n instructions from current PC.\n
 * @note Equivalent to "break" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_break(std::ostream &out, std::string arg1);

/**
 * @brief Run the program
 *
 * @param out Output stream where to print the command outptut
 */
void spect_iss_cmd_run(std::ostream &out);

/**
 * @brief Delete breakpoints
 *
 * @param out Output stream where to print the command outptut
 * @param arg1 Shall be one of:
 *              delete <label>      - Delete breakpoint at <label>.\n"
 *              delete <address>    - Delete breakpoint at address.\n"
 * @param all If set to true, all breakpoints will be deleted.
 * @note Equivalent to "delete" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_delete(std::ostream &out, std::string arg1, bool all);

/**
 * @brief Jump with the program to an address.
 *
 * @param out Output stream where to print the command outptut
 * @param arg1 Shall be the address where to jump.
 * @note Equivalent to "jump" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_jump(std::ostream &out, std::string arg1);

/**
 * @brief Get information about object in the SPECT Instruction Set simulator.
 *
 * @param out Output stream where to print the command outptut
 * @param arg1 Shall be one of:
 *                  - RX                           - Get GPR register X value."
 *                  - mem[address]                 - Get value at memory address."
 *                  - mem[address]+X               - Get value at memory address + X next addresses"
 *                  - keymem[type][slot][offset]   - Get value of key memory for given type, slot and offset."
 *                  - keymem[type][slot][offset]+X - Get value of key memory for given type, slot and offset + X next offsets");
 * @note Equivalent to "get" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_get(std::ostream &out, std::string arg1);

/**
 * @brief Set object in the SPECT Instruction Set simulator.
 *
 * @param out Output stream where to print the command outptut
 * @param arg1,arg2 Shall be one:
 *                   - RX <value>                         - Set GPR register X to <value>.\n"
 *                   - mem[address] <value>               - Set memory address to <value>\n"
 *                   - keymem[type][slot][offset] <value> - Set key memory for given type, slot and offset to to <value>\n");
 * @note Equivalent to "set" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_set(std::ostream &out, std::string arg1, std::string arg2);

/**
 * @brief Load HEX file to SPECT Instruction set simulator Physical Memory space
 *
 * @param out Output stream where to print the command outptut
 * @param arg1 Shall be path to the HEX file to load
 * @param offset Byte offset within SPECT physical memory space.
 * @note Equivalent to "load" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_load(std::ostream &out, std::string arg1, int offset);

/**
 * @brief Step "n" instructions.
 *
 * @param out Output stream where to print the command outptut
 * @param n   Number of instructions to step through
 * @note Equivalent to "step" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_step(std::ostream &out, int n);

/**
 * @brief Dump arbitrary physical memory range to an HEX file
 *
 * @param out Output stream where to print the command outptut
 * @param arg1 shall be path to hex file to dump
 * @param start_addr First physical address to dump
 * @param size Number of 32-bit memory words to dump.
 * @note Equivalent to "dump" command in "spect_iss" interactive shell
 */
void spect_iss_cmd_dump(std::ostream &out, std::string arg1, uint32_t address, uint32_t size);


/**************************************************************************************************
 **************************************************************************************************
 * Finalization
 **************************************************************************************************
 *************************************************************************************************/

/**
 * @brief Dump Data RAM Out to a hex file
 *
 * @param data_ram_out_hex_file - Path to hex file where to dump Data RAM Out
 * @note Equivalent to "--data-ram-out" switch of "spect_iss" executable
 */
void spect_iss_dump_data_ram_out_hex(std::string data_ram_out_hex_file);

/**
 * @brief Dump External Memory Out to a hex file
 *
 * @param emem_out_hex_file - Path to hex file where to dump External Memory Out
 * @note Equivalent to "--emem-out" switch of "spect_iss" executable
 */
void spect_iss_dump_emem_out_hex(std::string emem_out_hex_file);

/**
 * @brief Dump Key Memory to a hex file
 *
 * @param kmem_hex_file - Path to hex file where to dump Key Memory Out
 * @note Equivalent to "--dump-kmem" switch of "spect_iss" executable
 */
void spect_iss_dump_key_mem_out_hex(std::string kmem_hex_file);

/**
 * @brief Exit SPECT Instruction set simulator.
 */
void spect_iss_exit(void);


/**************************************************************************************************
 **************************************************************************************************
 * Common function
 **************************************************************************************************
 *************************************************************************************************/

/**
 * @returns Current GIT hash of "ts-spect-compiler" repository.
 */
int spect_iss_get_git_hash(void);

/**
 * @returns Current version of "spect_iss" and "spect_compiler".
 */
std::string spect_iss_get_version(void);

#endif SPECT_ISS_LIB_H_