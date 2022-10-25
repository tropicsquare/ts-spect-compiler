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

#ifndef COSIM_SPECT_ISS_DPI_H_
#define COSIM_SPECT_ISS_DPI_H_

#include "spect_iss_dpi_types.h"

extern "C" {

    /**
     *  @brief Initialize Simulation model.
     *  @returns 0 - Initialization succefull,
     *           1 - Initialization failed.
     */
    uint32_t spect_dpi_init();

    /**
     *  @brief Finish using simulation model (performs memory clean-up)
     */
    void spect_dpi_exit();

    /**
     *  @brief Reset simulation model.
     *
     *  Executes following:
     *      1. All GPRs become 0x0.
     *      2. All flags are cleared.
     *      3. Hash unit context is erased.
     *      4. Program counter is set to 0x0.
     *      5. RAR stack erased.
     *      6. Erase cycle counter for each instruction mnemonic.
     */
    void spect_dpi_reset();

    /**
     * @brief Returns word from SPECT memory space.
     * @param addr Address to return
     * @returns Value read from the SPECT memory space.
     * @note Allows to read any existing memory from SPECT memory space.
     */
    uint32_t spect_dpi_get_memory(uint32_t addr);

    /**
     * @brief Stores word to SPECT memory space
     * @param addr Address to write
     * @param data Data to write
     * @note Allows to write any existing memory from SPECT memory space (even ROM).
     */
    void spect_dpi_set_memory(uint32_t addr, uint32_t data);

    /**
     * @brief Executes read from SPECT memory space via AHB.
     * @param addr Address to read
     * @returns Value read via AHB.
     * @note Models access rights of SPECT on AHB (e.g. Data RAM IN is not readable).
     *       Read from invalid space returns all 0x0.
     */
    uint32_t spect_dpi_ahb_read(uint32_t addr);

    /**
     * @brief Executes write to SPECT memory space via AHB.
     * @param addr Address to write
     * @param data Data to write
     * @note Models access rights of SPECT on AHB (e.g. Constant ROM is not writable)
     *       Write to invalid space is ignored.
     */
    void spect_dpi_ahb_write(uint32_t addr, uint32_t data);

    /**
     * @brief Return Base address of a Memory within SPECT memory space
     * @param mem_type Type of memory to return base address
     * @returns Base address of memory within SPECT memory space.
     */
    uint32_t spect_dpi_get_mem_base(dpi_mem_type_t mem_type);

    /**
     * @brief Return size of a Memory within SPECT memory space
     * @param mem_type Type of memory to return base address
     * @returns Size of 'mem_type' memory of SPECT.
     */
    uint32_t spect_dpi_get_mem_size(dpi_mem_type_t mem_type);

    /**
     * @brief Returns value from part of General Purpose register.
     * @param gpr - GPR register index (0-31)
     * @param part - Part of the register
     *                  0 - Bits 31:0
     *                  1 - Bits 63:32
     *                  ...
     *                  8 - Bits 255:224
     * @returns Value in selected part of GPR register.
     */
    uint32_t spect_dpi_get_gpr_part(uint32_t gpr, uint32_t part);

    /**
     * @brief Stores value to part of General Purpose register.
     * @param gpr - GPR register index (0-31)
     * @param part - Part of the register:
     *                  0 - Bits 31:0
     *                  1 - Bits 63:32
     *                  ...
     *                  8 - Bits 255:224
     * @param data - Data to store to GPR register part.
     */
    void spect_dpi_set_gpr_part(uint32_t gpr, uint32_t part, uint32_t data);

    /**
     * @brief Get value of SPECT flag.
     * @param flag_type Type of flag to return value from
     * @returns 0 - Flag is not set.
     *          1 - Flag is set.
     */
    uint32_t spect_dpi_get_flag(dpi_flag_type_t flag_type);

    /**
     * @brief Get SPECT program counter
     * @returns Value of program counter
     */
    uint32_t spect_dpi_get_pc();

    /**
     * @brief Set SPECT program counter
     * @param value New value of program counter
     */
    void spect_dpi_set_pc(uint32_t value);

    /**
     * @brief Dump instruction.
     * @param address Adress of instruction in the memory.
     * @returns Instruction as it would be written in .S file.
     * @note return value is valid for whole duration of simulation.
     */
    void spect_dpi_dump_instruction(uint32_t address, char *buf);

    /**
     * @brief Get part of SPECTs SRR register.
     * @param part - Part of the SRR register:
     *                  0 - Bits 31:0
     *                  1 - Bits 63:32
     *                  ...
     *                  8 - Bits 255:224
     * @returns Value in selected part of SRR register.
     */
    uint32_t spect_dpi_get_srr(uint32_t part);

    /**
     * @brief Get value in RAR stack.
     * @param address Address to get from RAR stack:
     *                  0 - Bottom of stack,
     *                  N - top of stack, N = N(CALL) - N(RET)
     *                 where:
     *                  N(X) - Number of X instructions executed since start of program.
     * @returns Value in RAR_STACK on 'address'.
     */
    uint32_t spect_dpi_get_rar_value(uint32_t address);

    /**
     * @brief Get RAR stack pointer.
     * @returns Current value of RAR stack pointer.
     */
    uint32_t spect_dpi_get_rar_sp();

    /**
     * @brief Push data for GRV instruction queried via RNG Handshake interface.
     * @param data Data to push to GRV queue.
     */
    void spect_dpi_push_grv_queue(uint32_t data);

    /**
     * @brief Push data for GPK instruction queried via GPK Handshake interface.
     * @param data Data to push to one of GPK queues.
     * @param index Index of GPK queue (corresponds to value of spect_prk_type): 0 - 7.
     *              Corresponds to spect_prk_type[5:3] and immediate[2:0].
     */
    void spect_dpi_push_gpk_queue(uint32_t data, uint32_t index);

    /**
     * @brief Get value of SPECT interrupt output
     * @param type Type of interrupt to obtain the value
     * @returns 0 - Interrupt not asserted.
     *          1 - Interrupt asserted.
     */
    uint32_t spect_dpi_get_interrupt(dpi_int_type_t int_type);

    /**
     *  @brief Compile SPECT Program (.s assembly file) to hex file.
     *  @param program_path Path to .s file
     *  @param hex_path Path to output hex file
     *  @returns 0 - Program compiled succesfully
     *           non-zero - Compilation failed.
     *  @note This function fails if the S file does not define '_start' symbol.
     */
    uint32_t spect_dpi_compile_program(const char *program_path, const char* hex_path);

    /**
     *  @brief Load HEX file to SPECT memory. This could be firmware or data RAM,
     *         constant ROM content.
     *  @param path Path to .hex file
     *  @returns 0 - Program assembled and loaded correctly
     *           non-zero - Loading of assembly failed.
     */
    uint32_t spect_dpi_load_hex_file(const char *path);

    /**
     *  @brief Execute single instruction of program, and remember how many clock cycles
     *         execution of instruction with this mnemonic took.
     *  @param cycle_count Number of clock cycles it took to execute this instruction.
     *  @returns 0 - Instruction executed, and 'cycle_count' is equal to 'cycle_count' during
     *               previous execution of instruction with the same mnemonic. Returned also
     *               if instruction with the same mnemonic is executed first time since
     *               model initialization or, if the instruction was previously executed
     *               with 'cycle_count' = 0.
     *           N - Number of instructions previous execution of instruction with the same
     *               took.
     */
    uint32_t spect_dpi_program_step(uint32_t cycle_count);

    /**
     *  @brief Set Change reporting by model (pushing change events to SCHF - State Change FIFO)
     *  @param enable 0 - Disable change reporting
     *                1 - Enable change reporting
     */
    void spect_dpi_set_change_reporting(uint32_t enable);

    /**
     *  @brief Pop state change event from SCHF (State Change FIFO).
     *  @param handle Pointer to state change event
     *  @returns 0 - State change event popped, 'handler' contains valid state change event.
     *           1 - SCHF empty.
     *  @note Model does not report changes in PC (Program counter), since PC changes after
     *        every instruction. It is thus expected simulation side will anyway need to read
     *        program counter directly after each instruction!
     */
    uint32_t spect_dpi_get_model_change(dpi_state_change_t *change);

    /**
     *  @brief Execute SPECT instruction(s).
     *  @param instructions Number of instructions to execute. If 0, execute until END
     *                      instruction is executed.
     *  @returns Number of instructions actually executed by this call.
     *  @note Does not check execution time of the instruction.
     */
    uint32_t spect_dpi_program_run(uint32_t instructions);

    /**
     *  @brief Sets verbosity level of SPECT model
     *  @param level Verbosity level to set:
     *                  0 - None        - No debug prints
     *                  1 - Low         - Only most important debug prints
     *                  2 - Medium      - Regular debug prints
     *                  3 - High        - Print everything
     *  @note Debug prints from model execution to standard output.
     */
    void spect_dpi_set_verbosity(uint32_t level);

}

#endif