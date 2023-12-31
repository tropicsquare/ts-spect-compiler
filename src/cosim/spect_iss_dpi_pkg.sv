/******************************************************************************
* SPECT - DPI Package
*
* TODO: License
*
* Author: Vit Masek
******************************************************************************/
`ifndef SPECT_ISS_DPI_PKG_SV
`define SPECT_ISS_DPI_PKG_SV

package spect_iss_dpi_pkg;

  /////////////////////////////////////////////////////////////////////////////
  // Typedefs
  /////////////////////////////////////////////////////////////////////////////
  typedef enum {
    DPI_SPECT_DATA_RAM_IN       = (1 << 0),
    DPI_SPECT_DATA_RAM_OUT      = (1 << 1),
    DPI_SPECT_CONST_ROM         = (1 << 2),
    DPI_SPECT_INSTRUCTION_MEM   = (1 << 3)
  } dpi_mem_type_t;

  typedef enum {
    DPI_SPECT_FLAG_ZERO         = (1 << 0),
    DPI_SPECT_FLAG_CARRY        = (1 << 1),
    DPI_SPECT_FLAG_ERROR        = (1 << 2)
  } dpi_flag_type_t;

  typedef enum {
    DPI_SPECT_INT_DONE          = (1 << 0),
    DPI_SPECT_INT_ERR           = (1 << 1)
  } dpi_int_type_t;

  typedef enum {
    DPI_SPECT_RAR_PUSH          = (1 << 0),
    DPI_SPECT_RAR_POP           = (1 << 1)
  } dpi_rar_change_kind_t;

typedef enum {
    DPI_RBUS_FRESH_ENT          = (1 << 0),
    DPI_RBUS_NO_FRESH_ENT       = (1 << 1)
} dpi_rbus_change_kind_t;

typedef enum {
    DPI_KBUS_WRITE              = 0,
    DPI_KBUS_READ               = 1,
    DPI_KBUS_PROGRAM            = 2,
    DPI_KBUS_ERASE              = 3,
    DPI_KBUS_VERIFY             = 4,
    DPI_KBUS_FLUSH              = 5,
    DPI_KBUS_UNDEFINED[10]      = 6,
    DPI_KBUS_STK_WRITE          = 16,
    DPI_KBUS_LDK_READ           = 17
} dpi_kbus_change_kind_t;

  typedef enum {
    DPI_CHANGE_GPR              = (1 << 0),
    DPI_CHANGE_FLAG             = (1 << 1),
    DPI_CHANGE_MEM              = (1 << 2),
    DPI_CHANGE_INT              = (1 << 4),
    DPI_CHANGE_RAR              = (1 << 5),
    DPI_CHANGE_RBUS             = (1 << 6),
    DPI_CHANGE_KBUS             = (1 << 7)
  } dpi_change_kind_t;

  typedef enum {
    DPI_HEX_ISS_WORD            = (1 << 0),
    DPI_HEX_VERILOG_RAW_WORD    = (1 << 1),
    DPI_HEX_VERILOG_ADDR_WORD   = (1 << 2)
  } dpi_hex_file_type_t;

typedef enum {
    DPI_PARITY_ODD              = (1 << 0),
    DPI_PARITY_EVEN             = (1 << 1),
    DPI_PARITY_NONE             = (1 << 2)
} dpi_parity_type_t;

  typedef struct {
    dpi_change_kind_t kind = DPI_CHANGE_GPR;

    // Identifies object on which change occured based on kind
    //  DPI_CHANGE_GPR:
    //      0  - GPR0
    //      ...
    //      31 - GPR31
    //
    //  DPI_CHANGE_FLAG:
    //      DPI_SPECT_FLAG_ZERO
    //      DPI_SPECT_FLAG_CARRY
    //
    //  DPI_CHANGE_MEM:
    //      Address in memory on which change occured.
    //
    //  DPI_CHANGE_INT:
    //      DPI_SPECT_INT_DONE
    //      DPI_SPECT_INT_ERR
    //
    //  DPI_CHANGE_RAR:
    //      DPI_SPECT_RAR_PUSH - Push on stack
    //      DPI_SPECT_RAR_POP - Pop from stack
    //
    //  DPI_CHANGE_RBUS:
    //      DPI_RBUS_FRESH_ENT - Fresh entropy
    //      DPI_RBUS_NO_FRESH_ENT - No fresh entropy
    //
    //  DPI_CHANGE_KBUS:
    //       4: 0 - operation
    //       8: 5 - type
    //      16: 9 - slot
    //      31:17 - offset
    int unsigned      obj = 0;

    // Old / New value of the object based  on 'kind':
    //  DPI_CHANGE_GPR:
    //      0 - Bits 31:0
    //      ...
    //      7 - Bits 255:232
    //
    //  DPI_CHANGE_FLAG:
    //      0 - Value of flag
    //
    //  DPI_CHANGE_MEM:
    //      0 - Bits 31:0 of memory location
    //
    //  DPI_CHANGE_INT:
    //      0 - Value of the interrupt
    //
    //  DPI_CHANGE_RAR:
    //      obj == DPI_SPECT_RAR_PUSH (push) - Data pushed on stack
    //      obj == DPI_SPECT_RAR_POP (pop) - Data popped from stack
    //      both are valid only in "new_val".
    //
    //  DPI_CHANGE_RBUS:
    //      no meaning
    //
    //  DPI_CHANGE_KBUS:
    //      operation == DPI_KBUS_WRITE - data to write
    //      otherwise no meaning
    int unsigned      old_val[8] = '{default: 0};
    int unsigned      new_val[8] = '{default: 0};
  } dpi_state_change_t;

  typedef struct {
    int unsigned i_type = 0;
    int unsigned opcode = 0;
    int unsigned func = 0;

    // Register indices: 0 -31
    int unsigned op1 = '{default: 0};
    int unsigned op2 = '{default: 0};
    int unsigned op3 = '{default: 0};

    int unsigned immediate;
    int unsigned addr;
    int unsigned old_pc;
    int unsigned new_pc;

    // Operand values
    int unsigned op1_v[8] = '{default: 0};
    int unsigned op2_v[8] = '{default: 0};
    int unsigned op3_v[8] = '{default: 0};
    int unsigned r31_v[8] = '{default: 0};
  } dpi_instruction_t;

  /////////////////////////////////////////////////////////////////////////////
  // Imports section
  /////////////////////////////////////////////////////////////////////////////
  // UVM library package
  import uvm_pkg::*;

  /**
   *  @brief Initialize Simulation model.
   *  @returns 0 - Initialization succefull,
   *           1 - Initialization failed.
   */
  import "DPI-C" function int unsigned spect_dpi_init();

  /**
   *  @brief Finish using simulation model (performs memory clean-up)
   */
  import "DPI-C" function void spect_dpi_exit();

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
  import "DPI-C" function void spect_dpi_reset();

  /**
   * Checks if program has finished (END)
   * @returns 0 - Program is not finished (it is being executed)
   *          1 - Program is finished (END instruction as been executed)
   */
   import "DPI-C" function int unsigned spect_dpi_is_program_finished();

  /**
   * @brief Returns word from SPECT memory space.
   * @param addr Address to return
   * @returns Value read from the SPECT memory space.
   * @note Allows to read any existing memory from SPECT memory space.
   */
  import "DPI-C" function int unsigned spect_dpi_get_memory(int unsigned addr);

  /**
   * @brief Stores word to SPECT memory space
   * @param addr Address to write
   * @param data Data to write
   * @note Allows to write any existing memory from SPECT memory space (even ROM).
   */
  import "DPI-C" function void spect_dpi_set_memory(int unsigned addr, int unsigned data);

  /**
   * @brief Executes read from SPECT memory space via AHB.
   * @param addr Address to read
   * @returns Value read via AHB.
   * @note Models access rights of SPECT on AHB (e.g. Data RAM IN is not readable).
   *       Read from invalid space returns all 0x0.
   */
  import "DPI-C" function int unsigned spect_dpi_ahb_read(int unsigned addr);

  /**
   * @brief Executes write to SPECT memory space via AHB.
   * @param addr Address to write
   * @param data Data to write
   * @note Models access rights of SPECT on AHB (e.g. Constant ROM is not writable)
   *       Write to invalid space is ignored.
   */
  import "DPI-C" function void spect_dpi_ahb_write(int unsigned addr, int unsigned data);

  /**
   * @brief Return Base address of a Memory within SPECT memory space
   * @param mem_type Type of memory to return base address
   * @returns Base address of memory within SPECT memory space.
   */
  import "DPI-C" function int unsigned spect_dpi_get_mem_base(dpi_mem_type_t mem_type);

  /**
   * @brief Return size of a Memory within SPECT memory space
   * @param mem_type Type of memory to return base address
   * @returns Size of 'mem_type' memory of SPECT.
   */
  import "DPI-C" function int unsigned spect_dpi_get_mem_size(dpi_mem_type_t mem_type);

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
  import "DPI-C" function int unsigned spect_dpi_get_gpr_part(int unsigned gpr, int unsigned part);

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
  import "DPI-C" function void spect_dpi_set_gpr_part(int unsigned gpr, int unsigned part, int unsigned data);

  /**
   * @brief Get value of SPECT flag.
   * @param flag_type Type of flag to return value from
   * @returns 0 - Flag is not set.
   *          1 - Flag is set.
   */
  import "DPI-C" function int unsigned spect_dpi_get_flag(dpi_flag_type_t flag_type);

  /**
   * @brief Get SPECT program counter
   * @returns Value of program counter
   */
  import "DPI-C" function int unsigned spect_dpi_get_pc();

  /**
   * @brief Set SPECT program counter
   * @param value New value of program counter
   */
  import "DPI-C" function void spect_dpi_set_pc(int unsigned value);

  /**
   * @brief Dump instruction.
   * @param address Adress of instruction in the memory.
   * @returns Instruction as it would be written in .S file.
   * @note return value is valid for whole duration of simulation.
   */
  import "DPI-C" function void spect_dpi_dump_instruction(int unsigned address, string buffer);

  /**
   * @brief Get value in RAR stack.
   * @param address Address to get from RAR stack:
   *                  0 - Bottom of stack,
   *                  N - top of stack, N = N(CALL) - N(RET)
   *                 where:
   *                  N(X) - Number of X instructions executed since start of program.
   * @returns Value in RAR_STACK on 'address'.
   */
  import "DPI-C" function int unsigned spect_dpi_get_rar_value(int unsigned address);

  /**
   * @brief Get RAR stack pointer.
   * @returns Current value of RAR stack pointer.
   */
  import "DPI-C" function int unsigned spect_dpi_get_rar_sp();

  /**
   * @brief Push data for GRV instruction queried via RNG Handshake interface.
   * @param data Data to push to GRV queue.
   */
  import "DPI-C" function void spect_dpi_push_grv_queue(int unsigned data);

  /**
   * @brief Push data for LDK instruction queried via KBUS Handshake interface.
   * @param data Data to push to one of LDK queues.
   */
  import "DPI-C" function void spect_dpi_push_ldk_queue(int unsigned data);

  /**
   * @brief Push KBUS error flag for LDK/STK/ERK instruction queried via KBUS Handshake interface.
   * @param error Error to push to KBUS error queue.
   */
  import "DPI-C" function void spect_dpi_push_kbus_error_queue(byte unsigned error);

  /**
   * @brief Get value of SPECT interrupt output
   * @param type Type of interrupt to obtain the value
   * @returns 0 - interrupt not asserted.
   *          1 - interrupt asserted.
   */
  import "DPI-C" function int unsigned spect_dpi_get_interrupt(dpi_int_type_t int_type);

  /**
   *  @brief Compile SPECT Program (.s assembly file) to hex file.
   *  @param program_path Path to .s file
   *  @param hex_path Path to output hex file
   *  @param hex_format Format of Hex file to be generated.
   *              DPI_HEX_ISS_WORD            - Instruction set simulator
   *              DPI_HEX_VERILOG_RAW_WORD    - Verilog unadressed
   *              DPI_HEX_VERILOG_ADDR_WORD   - Verilog addressed
   *  @param parity_type Parity type to be applied.
   *              DPI_PARITY_ODD              - Odd parity
   *              DPI_PARITY_EVEN             - Even parity
   *              DPI_PARITY_NONE             - No parity calculated
   *  @param first_addr Address where to place first compiled instruction.
   *  @returns 0 - Program compiled succesfully
   *           non-zero - Compilation failed.
   *  @note This function fails if the S file does not define '_start' symbol.
   */
  import "DPI-C" function int unsigned spect_dpi_compile_program(string program_path, string hex_path,
                                                                 dpi_hex_file_type_t hex_format,
                                                                 dpi_parity_type_t parity_type,
                                                                 int unsigned first_addr);

  /**
   *  @returns Start address from previously compiled program (value of `_start` symbol.)
   *  @note Return value from this function is valid after previous call of
   *        'spect_dpi_compile_program'.
   */
  import "DPI-C" function int unsigned spect_dpi_get_compiled_program_start_address();

  /**
   * Sets value of models "Start PC"
   * @param value Value to be set as start_pc of model
   */
  import "DPI-C" function void spect_dpi_set_model_start_pc(int unsigned start_pc);

  /**
   *  @brief Load HEX file to SPECT memory. This could be firmware or data RAM,
   *         constant ROM content.
   *  @param path Path to .hex file
   *  @param offset Offset in the memory where to place the hex file.
   *                Set based on HEX file format:
   *                  DPI_HEX_ISS_WORD -
   *                      Has no effect
   *                 DPI_HEX_VERILOG_RAW_WORD -
   *                      Set to base address of the memory that you want to
   *                      preload (obtained via spect_dpi_get_mem_base).
   *                      This is usefull if you want to preload constant ROM
   *                      by HEX file which only contains constants, but not
   *                      their addresses. The same hex file can be loaded
   *                      to verilog memory model.
   *  @returns 0 - Program assembled and loaded correctly
   *           non-zero - Loading of assembly failed.
   */
  import "DPI-C" function int unsigned spect_dpi_load_hex_file(string path, int unsigned offset);

  /**
   *  @brief Execute single instruction of program, and remember how many clock cycles
   *         execution of instruction with this mnemonic took.
   *  @param cycle_count Number of clock cycles it took to execute this instruction.
   *  @returns 0 - Instruction executed, and 'cycle_count' is equal to 'cycle_count' during
   *               previous execution of instruction with the same mnemonic.
   *               Returned also if:
   *                  1. Instruction with the same mnemonic is executed first time since
   *                     model initialization.
   *                  2. The instruction was previously executed with 'cycle_count' = 0.
   *                  3. Instruction shall not be executed in constant time.
   *           N - Number of instructions previous execution of instruction with the same
   *               took.
   */
  import "DPI-C" function int unsigned spect_dpi_program_step(int unsigned cycle_count);

  /**
   *  @brief Returns last executed instruction, including its parameters (operands) and
   *         operand values.
   *  @param dpi_instruction Pointer to instruction structure.
   *  @returns 0 - Returned instruction is valid
   *           1 - Returned instruction is invalid (no instruction has been previously
   *               executed).
   *  @note This function shall be called after 'spect_dpi_program_step' or after
   *        'spect_dpi_program_run' to query last executed instruction for functional
   *        coverage measurement. If this function is queried without previous invocation
   *        of one of these functions, behavior of this function is undefined.
   *  @note Input parameters of instruction (op2, op3, immediate, newpc, addr) are
   *        sampled BEFORE execution of the instruction. Output parameters (op1) is
   *        sampled AFTER execution of the instruction!
   *  @note Model fills 0xFF to all attributes of 'dpi_instruction' which are not used by
   *        last executed instruction (e.g. 'addr' attribute will be filled to 0xFFFFFFFF
   *        for R type instructions, or all elements of op1_v will be filled by 0xFFFFFFFF
   *        for CMP instruction since CMP instruction does not use operand 1)
   */
  import "DPI-C" function void spect_dpi_get_last_instr(output dpi_instruction_t dpi_instruction);

  /**
   *  @brief Set Change reporting by model (pushing change events to SCHF - State Change FIFO)
   *  @param enable 0 - Disable change reporting
   *                1 - Enable change reporting
   */
  import "DPI-C" function void spect_dpi_set_change_reporting(int unsigned enable);

  /**
   *  @brief Pop state change event from SCHF (State Change FIFO).
   *  @param handle Pointer to state change event
   *  @returns 0 - State change event popped, 'handler' contains valid state change event.
   *           1 - SCHF empty.
   *  @note Model does not report changes in PC (Program counter), since PC changes after
   *        every instruction. It is thus expected simulation side will anyway need to read
   *        program counter directly after each instruction!
   */
  import "DPI-C" function int unsigned spect_dpi_get_model_change(output dpi_state_change_t change);

  /**
   *  @brief Execute SPECT instruction(s).
   *  @param instructions Number of instructions to execute. If 0, execute until END
   *                      instruction is executed.
   *  @returns Number of instructions actually executed by this call.
   *  @note Does not check execution time of the instruction.
   */
  import "DPI-C" function int unsigned spect_dpi_program_run(int unsigned instructions);

  /**
   *  @brief Sets verbosity level of SPECT model
   *  @param level Verbosity level to set:
   *                  0 - None        - No debug prints
   *                  1 - Low         - Only most important debug prints
   *                  2 - Medium      - Regular debug prints
   *                  3 - High        - Print everything
   *  @note Debug prints from model execution to standard output.
   */
  import "DPI-C" function void spect_dpi_set_verbosity(int unsigned level);

  /**
   * @brief Returns parity type checked by model
   * @returns Parity type.
   */
  import "DPI-C" function dpi_parity_type_t spect_dpi_get_parity_type();

  /**
   * @brief Set parity type checked by model
   * @param parity_type Parity type:
   *              DPI_PARITY_ODD  - Odd parity
   *              DPI_PARITY_EVEN - Even parity
   *              DPI_PARITY_NONE - No parity calculated
   */
  import "DPI-C" function void spect_dpi_set_parity_type(dpi_parity_type_t parity_type);

endpackage : spect_iss_dpi_pkg

`endif // SPECT_ISS_DPI_PKG_SV
