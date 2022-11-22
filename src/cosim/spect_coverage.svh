/******************************************************************************
* SPECT - Coverage
*
* TODO: License
*
* Author: Marek Santa
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Typedefs
///////////////////////////////////////////////////////////////////////////////

// Instructions
typedef enum bit[8:0] {
  
    I_ADD = 9'b11_0001_001, 
    I_SUB = 9'b11_0010_001, 
    I_CMP = 9'b11_0100_001, 
    I_AND = 9'b11_0001_010, 
    I_OR = 9'b11_0010_010, 
    I_XOR = 9'b11_0100_010, 
    I_NOT = 9'b11_1000_010, 
    I_LSL = 9'b11_0001_100, 
    I_LSR = 9'b11_0010_100, 
    I_ROL = 9'b11_0101_100, 
    I_ROR = 9'b11_0110_100, 
    I_ROL8 = 9'b11_1001_100, 
    I_ROR8 = 9'b11_1010_100, 
    I_SWE = 9'b11_1100_100, 
    I_MOV = 9'b11_0001_101, 
    I_CSWAP = 9'b11_0011_101, 
    I_HASH = 9'b11_0101_111, 
    I_GRV = 9'b11_1001_111, 
    I_SCB = 9'b11_1111_111, 
    I_MUL25519 = 9'b11_0011_110, 
    I_MUL256 = 9'b11_0111_110, 
    I_ADDP = 9'b11_1101_110, 
    I_SUBP = 9'b11_1110_110, 
    I_MULP = 9'b11_1111_110, 
    I_REDP = 9'b11_1100_110, 
    I_ADDI = 9'b01_0001_001, 
    I_SUBI = 9'b01_0010_001, 
    I_CMPI = 9'b01_0100_001, 
    I_ANDI = 9'b01_0001_010, 
    I_ORI = 9'b01_0010_010, 
    I_XORI = 9'b01_0100_010, 
    I_CMPA = 9'b01_1000_101, 
    I_MOVI = 9'b01_0001_101, 
    I_HASH_IT = 9'b01_0110_111, 
    I_GPK = 9'b01_1010_111, 
    I_LD = 9'b10_0010_101, 
    I_ST = 9'b10_0100_101, 
    I_CALL = 9'b00_0001_000, 
    I_RET = 9'b00_0010_000, 
    I_BRZ = 9'b00_0100_000, 
    I_BRNZ = 9'b00_0101_000, 
    I_BRC = 9'b00_0110_000, 
    I_BRNC = 9'b00_0111_000, 
    I_JMP = 9'b00_1100_000, 
    I_END = 9'b00_1001_111, 
    I_NOP = 9'b00_1010_111
} instr_t;


/////////////////////////////////////////////////////////////////////////////
// Covergroups
/////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------
// Cover each instruction
// -------------------------------------------------------------------------
covergroup instruction_cov with function sample(instr_t instr);
  // Instruction
  cp_instr: coverpoint instr;
endgroup : instruction_cov


// -------------------------------------------------------------------------
// Cover each bit of register toggled
// -------------------------------------------------------------------------
covergroup register_cov with function sample(logic[255:0] value, int position);
  // Track coverage information for each instance
  option.per_instance = 1;

  cp_register_zero: coverpoint position iff (value[position] == 0) {
    bins b[] = {[0:255]};
  }

  cp_register_one: coverpoint position iff (value[position] == 1) {
    bins b[] = {[0:255]};
  }
endgroup : register_cov


// -------------------------------------------------------------------------
// Cover each bit of immediate toggled
// -------------------------------------------------------------------------
covergroup immediate_cov with function sample(logic[11:0] value, int position);
  // Track coverage information for each instance
  option.per_instance = 1;

  cp_immediate_zero: coverpoint position iff (value[position] == 0) {
    bins b[] = {[0:12]};
  }

  cp_immediate_one: coverpoint position iff (value[position] == 1) {
    bins b[] = {[0:12]};
  }
endgroup : immediate_cov

// -------------------------------------------------------------------------
// Cover each bit of NewPC / Addr toggled
// -------------------------------------------------------------------------
covergroup addr_newpc_cov with function sample(logic[15:0] value, int position);
  // Track coverage information for each instance
  option.per_instance = 1;

  cp_immediate_zero: coverpoint position iff (value[position] == 0) {
    bins b[] = {[0:15]};
  }

  cp_immediate_one: coverpoint position iff (value[position] == 1) {
    bins b[] = {[0:15]};
  }
endgroup : addr_newpc_cov


// -------------------------------------------------------------------------
// Instruction with 3 operands
// -------------------------------------------------------------------------
covergroup operands_3_regs_cov with function sample(logic[4:0] op1, logic[4:0] op2, logic[4:0] op3);
  option.per_instance = 1;

  // Operand 1
  cp_op1: coverpoint op1 {
    bins register = {[0:31]};
  }

  // Operand 2
  cp_op2: coverpoint op2 {
    bins register = {[0:31]};
  }

  // Operand 3
  cp_op3: coverpoint op3 {
    bins register = {[0:31]};
  }

  // Two same operands
  cp_op1_eq_op2: coverpoint (op1 == op2) {
    bins true  = {1};
    bins false = {0};
  }

  cp_op1_eq_op3: coverpoint (op1 == op3) {
    bins true  = {1};
    bins false = {0};
  }

  cp_op2_eq_op3: coverpoint (op2 == op3) {
    bins true  = {1};
    bins false = {0};
  }

  // Three same operands
  cp_op1_eq_op2_eq_op3: coverpoint (op1 == op2 && op2 == op3) {
    bins true  = {1};
    bins false = {0};
  }

endgroup : operands_3_regs_cov


// -----------------------------
// Instruction with 2 operands
// -----------------------------

covergroup operands_2_regs_cov with function sample(logic[4:0] op1, logic[4:0] op2);
  option.per_instance = 1;

  // Operand 1
  cp_op1: coverpoint op1 {
    bins register = {[0:31]};
  }

  // Operand 2
  cp_op2: coverpoint op2 {
    bins register = {[0:31]};
  }

  // Two same operands
  cp_op1_eq_op2: coverpoint (op1 == op2) {
    bins true  = {1};
    bins false = {0};
  }

endgroup : operands_2_regs_cov

// -----------------------------
// Instruction with 1 operand
// -----------------------------

covergroup operands_1_reg_cov with function sample(logic[4:0] op1);
  option.per_instance = 1;

  // Operand 1
  cp_op1: coverpoint op1 {
    bins register = {[0:31]};
  }

endgroup : operands_1_reg_cov


// -----------------------------
// Operand values combination
// -----------------------------

covergroup op2_op3_cross_cov with function sample(logic[255:0] op2_val, logic[255:0] op3_val);
  option.per_instance = 1;

  // Operand 2
  cp_op2 : coverpoint op2_val {
    bins op2_val[10] = {[0:$]};
  }

  // Operand 3
  cp_op3 : coverpoint op3_val {
    bins op3_val[10] = {[0:$]};
  }

  // Cross
  cp_op2_op3_cross : cross cp_op2, cp_op3;

endgroup : op2_op3_cross_cov



/////////////////////////////////////////////////////////////////////////////
// Generated covergroups
/////////////////////////////////////////////////////////////////////////////


class spect_coverage extends uvm_component;
  // UVM Factory Registration
  `uvm_component_utils(spect_env_pkg::spect_coverage)

  /////////////////////////////////////////////////////////////////////////////
  // Data Members
  /////////////////////////////////////////////////////////////////////////////

  // Covergroups
  instruction_cov                     m_instruction_cov;

  // Coverage for instruction operand and their combinations
  operands_3_regs_cov       m_ADD_instruction_ops_cov;
  operands_3_regs_cov       m_SUB_instruction_ops_cov;
  operands_2_regs_cov       m_CMP_instruction_ops_cov;
  operands_3_regs_cov       m_AND_instruction_ops_cov;
  operands_3_regs_cov       m_OR_instruction_ops_cov;
  operands_3_regs_cov       m_XOR_instruction_ops_cov;
  operands_2_regs_cov       m_NOT_instruction_ops_cov;
  operands_2_regs_cov       m_LSL_instruction_ops_cov;
  operands_2_regs_cov       m_LSR_instruction_ops_cov;
  operands_2_regs_cov       m_ROL_instruction_ops_cov;
  operands_2_regs_cov       m_ROR_instruction_ops_cov;
  operands_2_regs_cov       m_ROL8_instruction_ops_cov;
  operands_2_regs_cov       m_ROR8_instruction_ops_cov;
  operands_2_regs_cov       m_SWE_instruction_ops_cov;
  operands_2_regs_cov       m_MOV_instruction_ops_cov;
  operands_2_regs_cov       m_CSWAP_instruction_ops_cov;
  operands_2_regs_cov       m_HASH_instruction_ops_cov;
  operands_1_reg_cov       m_GRV_instruction_ops_cov;
  operands_3_regs_cov       m_SCB_instruction_ops_cov;
  operands_3_regs_cov       m_MUL25519_instruction_ops_cov;
  operands_3_regs_cov       m_MUL256_instruction_ops_cov;
  operands_3_regs_cov       m_ADDP_instruction_ops_cov;
  operands_3_regs_cov       m_SUBP_instruction_ops_cov;
  operands_3_regs_cov       m_MULP_instruction_ops_cov;
  operands_3_regs_cov       m_REDP_instruction_ops_cov;
  operands_2_regs_cov       m_ADDI_instruction_ops_cov;
  operands_2_regs_cov       m_SUBI_instruction_ops_cov;
  operands_1_reg_cov       m_CMPI_instruction_ops_cov;
  operands_2_regs_cov       m_ANDI_instruction_ops_cov;
  operands_2_regs_cov       m_ORI_instruction_ops_cov;
  operands_2_regs_cov       m_XORI_instruction_ops_cov;
  operands_1_reg_cov       m_CMPA_instruction_ops_cov;
  operands_1_reg_cov       m_MOVI_instruction_ops_cov;
  operands_1_reg_cov       m_GPK_instruction_ops_cov;
  operands_1_reg_cov       m_LD_instruction_ops_cov;
  operands_1_reg_cov       m_ST_instruction_ops_cov;


  // Coverage for toggling on instruction operands
  register_cov        m_ADD_reg_cov[1:3];
  register_cov        m_SUB_reg_cov[1:3];
  register_cov        m_CMP_reg_cov[2:3];
  register_cov        m_AND_reg_cov[1:3];
  register_cov        m_OR_reg_cov[1:3];
  register_cov        m_XOR_reg_cov[1:3];
  register_cov        m_NOT_reg_cov[1:2];
  register_cov        m_LSL_reg_cov[1:2];
  register_cov        m_LSR_reg_cov[1:2];
  register_cov        m_ROL_reg_cov[1:2];
  register_cov        m_ROR_reg_cov[1:2];
  register_cov        m_ROL8_reg_cov[1:2];
  register_cov        m_ROR8_reg_cov[1:2];
  register_cov        m_SWE_reg_cov[1:2];
  register_cov        m_MOV_reg_cov[1:2];
  register_cov        m_CSWAP_reg_cov[1:2];
  register_cov        m_HASH_reg_cov[1:2];
  register_cov        m_GRV_reg_cov[1:1];
  register_cov        m_SCB_reg_cov[1:3];
  register_cov        m_MUL25519_reg_cov[1:3];
  register_cov        m_MUL256_reg_cov[1:3];
  register_cov        m_ADDP_reg_cov[1:3];
  register_cov        m_SUBP_reg_cov[1:3];
  register_cov        m_MULP_reg_cov[1:3];
  register_cov        m_REDP_reg_cov[1:3];
  register_cov        m_ADDI_reg_cov[1:2];
  register_cov        m_SUBI_reg_cov[1:2];
  register_cov        m_CMPI_reg_cov[1:1];
  register_cov        m_ANDI_reg_cov[1:2];
  register_cov        m_ORI_reg_cov[1:2];
  register_cov        m_XORI_reg_cov[1:2];
  register_cov        m_CMPA_reg_cov[1:1];
  register_cov        m_MOVI_reg_cov[2:2];
  register_cov        m_GPK_reg_cov[2:2];
  register_cov        m_LD_reg_cov[1:1];
  register_cov        m_ST_reg_cov[1:1];


  // Coverage for toggling on Immediate
  immediate_cov       m_ADDI_imm_cov;
  immediate_cov       m_SUBI_imm_cov;
  immediate_cov       m_CMPI_imm_cov;
  immediate_cov       m_ANDI_imm_cov;
  immediate_cov       m_ORI_imm_cov;
  immediate_cov       m_XORI_imm_cov;
  immediate_cov       m_CMPA_imm_cov;
  immediate_cov       m_MOVI_imm_cov;
  immediate_cov       m_GPK_imm_cov;


  // Coverage for toggling on NewPC
  addr_newpc_cov      m_CALL_newpc_cov;
  addr_newpc_cov      m_BRZ_newpc_cov;
  addr_newpc_cov      m_BRNZ_newpc_cov;
  addr_newpc_cov      m_BRC_newpc_cov;
  addr_newpc_cov      m_BRNC_newpc_cov;
  addr_newpc_cov      m_JMP_newpc_cov;


  // Coverage for toggling on Addr
  addr_newpc_cov      m_LD_addr_cov;
  addr_newpc_cov      m_ST_addr_cov;


  // Cross coverage for operands 2 and 3
  op2_op3_cross_cov   m_MUL25519_op_cross_cov;
  op2_op3_cross_cov   m_MUL256_op_cross_cov;
  op2_op3_cross_cov   m_ADDP_op_cross_cov;
  op2_op3_cross_cov   m_SUBP_op_cross_cov;
  op2_op3_cross_cov   m_MULP_op_cross_cov;
  op2_op3_cross_cov   m_REDP_op_cross_cov;


  /////////////////////////////////////////////////////////////////////////////
  // Methods
  /////////////////////////////////////////////////////////////////////////////

  //***************************************************************************
  // Function: new
  // Class constructor
  //
  // Parameters:
  // name   - Component's name
  // parent - Parent object in hierarchy
  //***************************************************************************
  function new(string name, uvm_component parent);
    super.new(name, parent);

    m_instruction_cov = new;

    m_ADD_instruction_ops_cov = new;
    m_SUB_instruction_ops_cov = new;
    m_CMP_instruction_ops_cov = new;
    m_AND_instruction_ops_cov = new;
    m_OR_instruction_ops_cov = new;
    m_XOR_instruction_ops_cov = new;
    m_NOT_instruction_ops_cov = new;
    m_LSL_instruction_ops_cov = new;
    m_LSR_instruction_ops_cov = new;
    m_ROL_instruction_ops_cov = new;
    m_ROR_instruction_ops_cov = new;
    m_ROL8_instruction_ops_cov = new;
    m_ROR8_instruction_ops_cov = new;
    m_SWE_instruction_ops_cov = new;
    m_MOV_instruction_ops_cov = new;
    m_CSWAP_instruction_ops_cov = new;
    m_HASH_instruction_ops_cov = new;
    m_GRV_instruction_ops_cov = new;
    m_SCB_instruction_ops_cov = new;
    m_MUL25519_instruction_ops_cov = new;
    m_MUL256_instruction_ops_cov = new;
    m_ADDP_instruction_ops_cov = new;
    m_SUBP_instruction_ops_cov = new;
    m_MULP_instruction_ops_cov = new;
    m_REDP_instruction_ops_cov = new;
    m_ADDI_instruction_ops_cov = new;
    m_SUBI_instruction_ops_cov = new;
    m_CMPI_instruction_ops_cov = new;
    m_ANDI_instruction_ops_cov = new;
    m_ORI_instruction_ops_cov = new;
    m_XORI_instruction_ops_cov = new;
    m_CMPA_instruction_ops_cov = new;
    m_MOVI_instruction_ops_cov = new;
    m_GPK_instruction_ops_cov = new;
    m_LD_instruction_ops_cov = new;
    m_ST_instruction_ops_cov = new;

    foreach (m_ADD_reg_cov[i]) m_ADD_reg_cov[i] = new;
    foreach (m_SUB_reg_cov[i]) m_SUB_reg_cov[i] = new;
    foreach (m_CMP_reg_cov[i]) m_CMP_reg_cov[i] = new;
    foreach (m_AND_reg_cov[i]) m_AND_reg_cov[i] = new;
    foreach (m_OR_reg_cov[i]) m_OR_reg_cov[i] = new;
    foreach (m_XOR_reg_cov[i]) m_XOR_reg_cov[i] = new;
    foreach (m_NOT_reg_cov[i]) m_NOT_reg_cov[i] = new;
    foreach (m_LSL_reg_cov[i]) m_LSL_reg_cov[i] = new;
    foreach (m_LSR_reg_cov[i]) m_LSR_reg_cov[i] = new;
    foreach (m_ROL_reg_cov[i]) m_ROL_reg_cov[i] = new;
    foreach (m_ROR_reg_cov[i]) m_ROR_reg_cov[i] = new;
    foreach (m_ROL8_reg_cov[i]) m_ROL8_reg_cov[i] = new;
    foreach (m_ROR8_reg_cov[i]) m_ROR8_reg_cov[i] = new;
    foreach (m_SWE_reg_cov[i]) m_SWE_reg_cov[i] = new;
    foreach (m_MOV_reg_cov[i]) m_MOV_reg_cov[i] = new;
    foreach (m_CSWAP_reg_cov[i]) m_CSWAP_reg_cov[i] = new;
    foreach (m_HASH_reg_cov[i]) m_HASH_reg_cov[i] = new;
    foreach (m_GRV_reg_cov[i]) m_GRV_reg_cov[i] = new;
    foreach (m_SCB_reg_cov[i]) m_SCB_reg_cov[i] = new;
    foreach (m_MUL25519_reg_cov[i]) m_MUL25519_reg_cov[i] = new;
    foreach (m_MUL256_reg_cov[i]) m_MUL256_reg_cov[i] = new;
    foreach (m_ADDP_reg_cov[i]) m_ADDP_reg_cov[i] = new;
    foreach (m_SUBP_reg_cov[i]) m_SUBP_reg_cov[i] = new;
    foreach (m_MULP_reg_cov[i]) m_MULP_reg_cov[i] = new;
    foreach (m_REDP_reg_cov[i]) m_REDP_reg_cov[i] = new;
    foreach (m_ADDI_reg_cov[i]) m_ADDI_reg_cov[i] = new;
    foreach (m_SUBI_reg_cov[i]) m_SUBI_reg_cov[i] = new;
    foreach (m_CMPI_reg_cov[i]) m_CMPI_reg_cov[i] = new;
    foreach (m_ANDI_reg_cov[i]) m_ANDI_reg_cov[i] = new;
    foreach (m_ORI_reg_cov[i]) m_ORI_reg_cov[i] = new;
    foreach (m_XORI_reg_cov[i]) m_XORI_reg_cov[i] = new;
    foreach (m_CMPA_reg_cov[i]) m_CMPA_reg_cov[i] = new;
    foreach (m_MOVI_reg_cov[i]) m_MOVI_reg_cov[i] = new;
    foreach (m_GPK_reg_cov[i]) m_GPK_reg_cov[i] = new;
    foreach (m_LD_reg_cov[i]) m_LD_reg_cov[i] = new;
    foreach (m_ST_reg_cov[i]) m_ST_reg_cov[i] = new;

    m_ADDI_imm_cov = new;
    m_SUBI_imm_cov = new;
    m_CMPI_imm_cov = new;
    m_ANDI_imm_cov = new;
    m_ORI_imm_cov = new;
    m_XORI_imm_cov = new;
    m_CMPA_imm_cov = new;
    m_MOVI_imm_cov = new;
    m_GPK_imm_cov = new;

    m_CALL_newpc_cov = new;
    m_BRZ_newpc_cov = new;
    m_BRNZ_newpc_cov = new;
    m_BRC_newpc_cov = new;
    m_BRNC_newpc_cov = new;
    m_JMP_newpc_cov = new;

    m_LD_addr_cov = new;
    m_ST_addr_cov = new;

    m_MUL25519_op_cross_cov = new;
    m_MUL256_op_cross_cov = new;
    m_ADDP_op_cross_cov = new;
    m_SUBP_op_cross_cov = new;
    m_MULP_op_cross_cov = new;
    m_REDP_op_cross_cov = new;


  endfunction : new


  //***************************************************************************
  // Function: sample_instruction
  // This function will be called from Predictor every time an instruction is
  // executed. Type of the input parameter can be any appropriate
  // representation of an instruction (string, bit vector, structure, ...)

  // Parameters:
  // instr - instruction
  //***************************************************************************
  function void sample_instruction(dpi_instruction_t dpi_instruction);

    // Convert instruction encoding to vector format
    logic [1:0] l_type   = dpi_instruction.i_type;
    logic [3:0] l_opcode = dpi_instruction.opcode;
    logic [2:0] l_func   = dpi_instruction.func;

    logic [255:0] op1 = {32'(dpi_instruction.v.r.op1[7]), 32'(dpi_instruction.v.r.op1[6]),
                         32'(dpi_instruction.v.r.op1[5]), 32'(dpi_instruction.v.r.op1[4]),
                         32'(dpi_instruction.v.r.op1[3]), 32'(dpi_instruction.v.r.op1[2]),
                         32'(dpi_instruction.v.r.op1[1]), 32'(dpi_instruction.v.r.op1[0])};

    logic [255:0] op2 = {32'(dpi_instruction.v.r.op2[7]), 32'(dpi_instruction.v.r.op2[6]),
                         32'(dpi_instruction.v.r.op2[5]), 32'(dpi_instruction.v.r.op2[4]),
                         32'(dpi_instruction.v.r.op2[3]), 32'(dpi_instruction.v.r.op2[2]),
                         32'(dpi_instruction.v.r.op2[1]), 32'(dpi_instruction.v.r.op2[0])};

    logic [255:0] op3 = {32'(dpi_instruction.v.r.op3[7]), 32'(dpi_instruction.v.r.op3[6]),
                         32'(dpi_instruction.v.r.op3[5]), 32'(dpi_instruction.v.r.op3[4]),
                         32'(dpi_instruction.v.r.op3[3]), 32'(dpi_instruction.v.r.op3[2]),
                         32'(dpi_instruction.v.r.op3[1]), 32'(dpi_instruction.v.r.op3[0])};

    // Sample instruction
    m_instruction_cov.sample(instr_t'({l_type, l_opcode, l_func}));

    // Sample operands
    case ({l_type, l_opcode, l_func})

      
        I_ADD: begin
          m_ADD_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_ADD_reg_cov[1].sample(op1, i);
            m_ADD_reg_cov[2].sample(op2, i);
            m_ADD_reg_cov[3].sample(op3, i);
          end

        end
      
        I_SUB: begin
          m_SUB_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_SUB_reg_cov[1].sample(op1, i);
            m_SUB_reg_cov[2].sample(op2, i);
            m_SUB_reg_cov[3].sample(op3, i);
          end

        end
      
        I_CMP: begin
          m_CMP_instruction_ops_cov.sample(dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_CMP_reg_cov[2].sample(op2, i);
            m_CMP_reg_cov[3].sample(op3, i);
          end

        end
      
        I_AND: begin
          m_AND_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_AND_reg_cov[1].sample(op1, i);
            m_AND_reg_cov[2].sample(op2, i);
            m_AND_reg_cov[3].sample(op3, i);
          end

        end
      
        I_OR: begin
          m_OR_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_OR_reg_cov[1].sample(op1, i);
            m_OR_reg_cov[2].sample(op2, i);
            m_OR_reg_cov[3].sample(op3, i);
          end

        end
      
        I_XOR: begin
          m_XOR_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_XOR_reg_cov[1].sample(op1, i);
            m_XOR_reg_cov[2].sample(op2, i);
            m_XOR_reg_cov[3].sample(op3, i);
          end

        end
      
        I_NOT: begin
          m_NOT_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_NOT_reg_cov[1].sample(op1, i);
            m_NOT_reg_cov[2].sample(op2, i);
          end

        end
      
        I_LSL: begin
          m_LSL_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_LSL_reg_cov[1].sample(op1, i);
            m_LSL_reg_cov[2].sample(op2, i);
          end

        end
      
        I_LSR: begin
          m_LSR_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_LSR_reg_cov[1].sample(op1, i);
            m_LSR_reg_cov[2].sample(op2, i);
          end

        end
      
        I_ROL: begin
          m_ROL_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_ROL_reg_cov[1].sample(op1, i);
            m_ROL_reg_cov[2].sample(op2, i);
          end

        end
      
        I_ROR: begin
          m_ROR_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_ROR_reg_cov[1].sample(op1, i);
            m_ROR_reg_cov[2].sample(op2, i);
          end

        end
      
        I_ROL8: begin
          m_ROL8_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_ROL8_reg_cov[1].sample(op1, i);
            m_ROL8_reg_cov[2].sample(op2, i);
          end

        end
      
        I_ROR8: begin
          m_ROR8_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_ROR8_reg_cov[1].sample(op1, i);
            m_ROR8_reg_cov[2].sample(op2, i);
          end

        end
      
        I_SWE: begin
          m_SWE_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_SWE_reg_cov[1].sample(op1, i);
            m_SWE_reg_cov[2].sample(op2, i);
          end

        end
      
        I_MOV: begin
          m_MOV_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_MOV_reg_cov[1].sample(op1, i);
            m_MOV_reg_cov[2].sample(op2, i);
          end

        end
      
        I_CSWAP: begin
          m_CSWAP_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_CSWAP_reg_cov[1].sample(op1, i);
            m_CSWAP_reg_cov[2].sample(op2, i);
          end

        end
      
        I_HASH: begin
          m_HASH_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_HASH_reg_cov[1].sample(op1, i);
            m_HASH_reg_cov[2].sample(op2, i);
          end

        end
      
        I_GRV: begin
          m_GRV_instruction_ops_cov.sample(dpi_instruction.op1);
          for (int i=0; i<255; i++) begin
            m_GRV_reg_cov[1].sample(op1, i);
          end

        end
      
        I_SCB: begin
          m_SCB_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_SCB_reg_cov[1].sample(op1, i);
            m_SCB_reg_cov[2].sample(op2, i);
            m_SCB_reg_cov[3].sample(op3, i);
          end

        end
      
        I_MUL25519: begin
          m_MUL25519_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_MUL25519_reg_cov[1].sample(op1, i);
            m_MUL25519_reg_cov[2].sample(op2, i);
            m_MUL25519_reg_cov[3].sample(op3, i);
          end
          m_MUL25519_op_cross_cov.sample(op2, op3);

        end
      
        I_MUL256: begin
          m_MUL256_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_MUL256_reg_cov[1].sample(op1, i);
            m_MUL256_reg_cov[2].sample(op2, i);
            m_MUL256_reg_cov[3].sample(op3, i);
          end
          m_MUL256_op_cross_cov.sample(op2, op3);

        end
      
        I_ADDP: begin
          m_ADDP_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_ADDP_reg_cov[1].sample(op1, i);
            m_ADDP_reg_cov[2].sample(op2, i);
            m_ADDP_reg_cov[3].sample(op3, i);
          end
          m_ADDP_op_cross_cov.sample(op2, op3);

        end
      
        I_SUBP: begin
          m_SUBP_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_SUBP_reg_cov[1].sample(op1, i);
            m_SUBP_reg_cov[2].sample(op2, i);
            m_SUBP_reg_cov[3].sample(op3, i);
          end
          m_SUBP_op_cross_cov.sample(op2, op3);

        end
      
        I_MULP: begin
          m_MULP_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_MULP_reg_cov[1].sample(op1, i);
            m_MULP_reg_cov[2].sample(op2, i);
            m_MULP_reg_cov[3].sample(op3, i);
          end
          m_MULP_op_cross_cov.sample(op2, op3);

        end
      
        I_REDP: begin
          m_REDP_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2, dpi_instruction.op3);
          for (int i=0; i<255; i++) begin
            m_REDP_reg_cov[1].sample(op1, i);
            m_REDP_reg_cov[2].sample(op2, i);
            m_REDP_reg_cov[3].sample(op3, i);
          end
          m_REDP_op_cross_cov.sample(op2, op3);

        end
      
        I_ADDI: begin
          m_ADDI_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_ADDI_reg_cov[1].sample(op1, i);
            m_ADDI_reg_cov[2].sample(op2, i);
          end
          for (int i=0; i<12; i++) begin
            m_ADDI_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_SUBI: begin
          m_SUBI_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_SUBI_reg_cov[1].sample(op1, i);
            m_SUBI_reg_cov[2].sample(op2, i);
          end
          for (int i=0; i<12; i++) begin
            m_SUBI_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_CMPI: begin
          m_CMPI_instruction_ops_cov.sample(dpi_instruction.op1);
          for (int i=0; i<255; i++) begin
            m_CMPI_reg_cov[1].sample(op1, i);
          end
          for (int i=0; i<12; i++) begin
            m_CMPI_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_ANDI: begin
          m_ANDI_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_ANDI_reg_cov[1].sample(op1, i);
            m_ANDI_reg_cov[2].sample(op2, i);
          end
          for (int i=0; i<12; i++) begin
            m_ANDI_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_ORI: begin
          m_ORI_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_ORI_reg_cov[1].sample(op1, i);
            m_ORI_reg_cov[2].sample(op2, i);
          end
          for (int i=0; i<12; i++) begin
            m_ORI_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_XORI: begin
          m_XORI_instruction_ops_cov.sample(dpi_instruction.op1, dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_XORI_reg_cov[1].sample(op1, i);
            m_XORI_reg_cov[2].sample(op2, i);
          end
          for (int i=0; i<12; i++) begin
            m_XORI_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_CMPA: begin
          m_CMPA_instruction_ops_cov.sample(dpi_instruction.op1);
          for (int i=0; i<255; i++) begin
            m_CMPA_reg_cov[1].sample(op1, i);
          end
          for (int i=0; i<12; i++) begin
            m_CMPA_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_MOVI: begin
          m_MOVI_instruction_ops_cov.sample(dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_MOVI_reg_cov[2].sample(op2, i);
          end
          for (int i=0; i<12; i++) begin
            m_MOVI_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_HASH_IT: begin

        end
      
        I_GPK: begin
          m_GPK_instruction_ops_cov.sample(dpi_instruction.op2);
          for (int i=0; i<255; i++) begin
            m_GPK_reg_cov[2].sample(op2, i);
          end
          for (int i=0; i<12; i++) begin
            m_GPK_imm_cov.sample(dpi_instruction.v.i.immediate, i);
          end

        end
      
        I_LD: begin
          m_LD_instruction_ops_cov.sample(dpi_instruction.op1);
          for (int i=0; i<255; i++) begin
            m_LD_reg_cov[1].sample(op1, i);
          end
          for (int i=0; i<16; i++) begin
            m_LD_addr_cov.sample(dpi_instruction.v.m.addr, i);
          end

        end
      
        I_ST: begin
          m_ST_instruction_ops_cov.sample(dpi_instruction.op1);
          for (int i=0; i<255; i++) begin
            m_ST_reg_cov[1].sample(op1, i);
          end
          for (int i=0; i<16; i++) begin
            m_ST_addr_cov.sample(dpi_instruction.v.m.addr, i);
          end

        end
      
        I_CALL: begin
          for (int i=0; i<16; i++) begin
            m_CALL_newpc_cov.sample(dpi_instruction.v.j.new_pc, i);
          end

        end
      
        I_RET: begin

        end
      
        I_BRZ: begin
          for (int i=0; i<16; i++) begin
            m_BRZ_newpc_cov.sample(dpi_instruction.v.j.new_pc, i);
          end

        end
      
        I_BRNZ: begin
          for (int i=0; i<16; i++) begin
            m_BRNZ_newpc_cov.sample(dpi_instruction.v.j.new_pc, i);
          end

        end
      
        I_BRC: begin
          for (int i=0; i<16; i++) begin
            m_BRC_newpc_cov.sample(dpi_instruction.v.j.new_pc, i);
          end

        end
      
        I_BRNC: begin
          for (int i=0; i<16; i++) begin
            m_BRNC_newpc_cov.sample(dpi_instruction.v.j.new_pc, i);
          end

        end
      
        I_JMP: begin
          for (int i=0; i<16; i++) begin
            m_JMP_newpc_cov.sample(dpi_instruction.v.j.new_pc, i);
          end

        end
      
        I_END: begin

        end
      
        I_NOP: begin

        end
      endcase

  endfunction : sample_instruction

endclass : spect_coverage