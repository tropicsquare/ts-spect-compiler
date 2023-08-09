#!/usr/bin/env python3
# PYTHON_ARGCOMPLETE_OK
# -*- coding: utf-8 -*-

####################################################################################################
# Generator of coverage class from instruction definitions.
#
# TODO: License
####################################################################################################

__author__ = "Ondrej Ille"
__copyright__ = "Tropic Square"
__license___ = "TODO:"
__maintainer__ = "Ondrej Ille"

import sys
import jinja2
from jinja2 import Template
from argparse import ArgumentParser


DPI_I_NAME = "dpi_instruction"


def form_val_code_sample(cov_point_name, has_op1, has_op2, has_op3, has_r31, has_special_cp, special_cov_point_name, special_op):
    rv = "          for (int i=0; i<256; i++) begin\n"
    if (has_op1):
        rv += f"            {cov_point_name}[1].sample(op1, i);\n"
    if (has_op2):
        rv += f"            {cov_point_name}[2].sample(op2, i);\n"
    if (has_op3):
        rv += f"            {cov_point_name}[3].sample(op3, i);\n"
    if (has_r31):
        rv += f"            {cov_point_name}[4].sample(r31, i);\n"
    if (has_special_cp):
        rv += f"            {special_cov_point_name}.sample({special_op}, i);\n"
    rv += "          end\n"
    return rv


def form_imm_sample(cov_point_name):
    rv = "          for (int i=0; i<12; i++) begin\n"
    rv += f"            {cov_point_name}.sample(dpi_instruction.immediate, i);\n"
    rv += "          end\n"
    return rv


def form_addr_sample(cov_point_name):
    rv = "          for (int i=0; i<16; i++) begin\n"
    rv += f"            {cov_point_name}.sample(dpi_instruction.addr, i);\n"
    rv += "          end\n"
    return rv


def form_newpc_sample(cov_point_name):
    rv = "          for (int i=0; i<16; i++) begin\n"
    rv += f"            {cov_point_name}.sample(dpi_instruction.new_pc, i);\n"
    rv += "          end\n"
    return rv


def remove_prefix(text, prefix):
    if text.startswith(prefix):
        return text[len(prefix):]
    return text  # or whatever


if __name__ == "__main__":

    parser = ArgumentParser(description="Coverage class generator for SPECT.")
    parser.add_argument("--input",          help="Input file with instruction definitions.")
    parser.add_argument("--output",         help="Output file where to generate SPECT coverage class.")
    parser.add_argument("--cov-template",   help="Path to coverage template file.")
    parser.add_argument("--instr-defs",     help="Path to instruction definitions.")
    args = parser.parse_args()

    # Open Jinja template
    with open(args.cov_template) as fd_tf_:
        template_ = Template(fd_tf_.read())

    i_ctx = {
        "instructions" : []
    }

    # Load Instruction definitions
    with open(args.instr_defs) as fd_instr_:
        lines = fd_instr_.readlines()
        i_types = []

        for line in lines:

            # On first line load available instruction types
            if (line.startswith("_TYPES:")):
                raw = line.strip("\n").split(":")[1].strip(' ').split(" ")
                #print(raw)
                for i_type in raw:
                    tmp = i_type.split("=")
                    i_types.append([tmp[0], tmp[1]])
                print("Loaded instruction types: ")
                print(i_types)

            # Check for valid line
            skip_line = True
            for i_type in i_types:
                if line[0] == i_type[0]:
                    skip_line = False
            if skip_line:
                continue

            # Parse out Instruction attribtues
            [i_type_str, mnemonic, opcode, func, op_mask, r31_dep, c_time, op2_op3_cross, cycles] = line.split()

            i_type_int = 0
            for i_type in i_types:
                if (i_type[0] == i_type_str):
                    i_type_int = i_type[1]

            func = remove_prefix(func, "0b")
            opcode = remove_prefix(opcode, "0b")
            i_type_int = remove_prefix(i_type_int, "0b")

            # Parse instruction type and generate names of cover groups
            op_cov_def = ""
            val_cov_def = ""
            res_val_cov_def = ""
            op3_val_cov_def = ""
            imm_cov_def = ""
            newpc_cov_def = ""
            cross_cov_def = ""
            op_cov_sample = ""
            val_cov_sample = ""
            imm_cov_sample = ""
            newpc_cov_sample = ""
            cross_cov_sample = ""
            addr_cov_def = ""
            addr_cov_sample = ""

            if (i_type_str == "R"):
                if op_mask == "0b111":
                    if mnemonic == "ADDP" or mnemonic == "SUBP":
                        op_cov_def = f"  operands_3_regs_modular_cov m_{mnemonic}_instruction_ops_cov;\n"
                    else:
                        op_cov_def = f"  operands_3_regs_cov         m_{mnemonic}_instruction_ops_cov;\n"
                    op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op1, {DPI_I_NAME}.op2, {DPI_I_NAME}.op3);\n"

                    if r31_dep == "true":
                        val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[1:4];\n"
                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, True, True, True, False, "", "")
                    else:
                        if mnemonic == "ADD" or mnemonic == "SUB":
                            val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[2:3];\n"
                            res_val_cov_def = f"  register_arith_cov  m_{mnemonic}_res_reg_cov;\n"
                            val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", False, True, True, False, True, f"m_{mnemonic}_res_reg_cov", "op1")
                        elif mnemonic == "SBIT" or mnemonic == "CBIT":
                            val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[1:2];\n"
                            op3_val_cov_def = f"  register_bit_cov    m_{mnemonic}_op3_reg_cov;\n"
                            val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, True, False, False, True, f"m_{mnemonic}_op3_reg_cov", "op3")
                        elif mnemonic == "MUL25519":
                            val_cov_def = f"  register_mul25519_cov m_{mnemonic}_reg_cov[1:3];\n"
                            val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, True, True, False, False, "", "")
                        else:
                            val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[1:3];\n"
                            val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, True, True, False, False, "", "")

                    if op2_op3_cross == "1":
                        if mnemonic == "MUL25519":
                            cross_cov_def = f"  op2_op3_cross_mul25519_cov m_{mnemonic}_op_cross_cov;\n"
                        else:
                            cross_cov_def = f"  op2_op3_cross_cov          m_{mnemonic}_op_cross_cov;\n"

                        cross_cov_sample = f"          m_{mnemonic}_op_cross_cov.sample(op2, op3);\n"

                elif (op_mask == "0b011" or op_mask == "0b110" or op_mask == "0b101"):
                    op_cov_def = f"  operands_2_regs_cov         m_{mnemonic}_instruction_ops_cov;\n"

                    if (op_mask == "0b011"):
                        val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[2:3];\n"
                        op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op2, {DPI_I_NAME}.op3);\n"
                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", False, True, True, False, False, "", "")

                    elif (op_mask == "0b110"):
                        op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op1, {DPI_I_NAME}.op2);\n"
                        if mnemonic == "LSR" or mnemonic == "LSL":
                            val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[2:2];\n"
                            res_val_cov_def = f"  register_{mnemonic.lower()}_cov    m_{mnemonic}_res_reg_cov;\n"
                            val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", False, True, False, False, True, f"m_{mnemonic}_res_reg_cov", "op1")
                        else:
                            val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[1:2];\n"
                            val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, True, False, False, False, "", "")

                    else:
                        print(f"Skipping operand coverpoint for: {mnemonic}")

                elif (op_mask == "0b100" or op_mask == "0b010" or op_mask == "0b001"):
                    op_cov_def = f"  operands_1_reg_cov          m_{mnemonic}_instruction_ops_cov;\n"
                    if (op_mask == "0b100"):
                        op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op1);\n"

                        val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[1:1];\n"
                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, False, False, False, False, "", "")

                    elif (op_mask == "0b010"):
                        op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op2);\n"

                        val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[2:2];\n"
                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", False, True, False, False, False, "", "")

                    elif (op_mask == "0b001"):
                        op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op3);\n"

                        val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[3:3];\n"
                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", False, False, True, False, False, "", "")

                #else:
                #    print(f"Invalid operand mask: {op_mask}, for instruction: {mnemonic}")

            elif (i_type_str == "I"):
                if op_mask == "0b111":
                    op_cov_def = f"  operands_2_regs_cov         m_{mnemonic}_instruction_ops_cov;\n"
                    op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op1, {DPI_I_NAME}.op2);\n"

                    imm_cov_def = f"  immediate_cov       m_{mnemonic}_imm_cov;\n"
                    imm_cov_sample = form_imm_sample(f"m_{mnemonic}_imm_cov")

                    if mnemonic == "ADDI" or mnemonic == "SUBI":
                        val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[2:2];\n"
                        res_val_cov_def = f"  register_arith_cov  m_{mnemonic}_res_reg_cov;\n"
                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", False, True, False, False, True, f"m_{mnemonic}_res_reg_cov", "op1")
                    else:
                        val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[1:2];\n"
                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, True, False, False, False, "", "")

                elif (op_mask == "0b011" or op_mask == "0b101"):
                    op_cov_def = f"  operands_1_reg_cov          m_{mnemonic}_instruction_ops_cov;\n"

                    if (op_mask == "0b101"):
                        op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op1);\n"

                        if mnemonic == "MOVI":
                            val_cov_def = f"  register_movi_cov     m_{mnemonic}_reg_cov[1:1];\n"
                        else:
                            val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[1:1];\n"

                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, False, False, False, False, "", "")

                    else:
                        op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op2);\n"

                        val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[2:2];\n"
                        val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", False, True, False, False, False, "", "")

                    imm_cov_def = f"  immediate_cov       m_{mnemonic}_imm_cov;\n"
                    imm_cov_sample = form_imm_sample(f"m_{mnemonic}_imm_cov")

                #else:
                #    print(f"Invalid operand mask: {op_mask}, for instruction: {mnemonic}")

            elif (i_type_str == "M"):
                op_cov_def = f"  operands_1_reg_cov          m_{mnemonic}_instruction_ops_cov;\n"
                op_cov_sample = f"          m_{mnemonic}_instruction_ops_cov.sample({DPI_I_NAME}.op1);\n"

                val_cov_def = f"  register_cov          m_{mnemonic}_reg_cov[1:1];\n"
                val_cov_sample = form_val_code_sample(f"m_{mnemonic}_reg_cov", True, False, False, False, False, "", "")

                addr_cov_def = f"  addr_newpc_cov      m_{mnemonic}_addr_cov;\n"
                addr_cov_sample = form_addr_sample(f"m_{mnemonic}_addr_cov")

            elif (i_type_str == "J"):
                if op_mask == "0b100":
                    newpc_cov_def = f"  addr_newpc_cov      m_{mnemonic}_newpc_cov;\n"
                    newpc_cov_sample = form_newpc_sample(f"m_{mnemonic}_newpc_cov")


            new_instr = {
                 "mnemonic"         : mnemonic,
                 "i_type"           : i_type_int,
                 "opcode"           : opcode,
                 "func"             : func,
                 "op_mask"          : op_mask,
                 "r31_dep"          : r31_dep,
                 "c_time "          : c_time,
            }

            if op_cov_def:
                new_instr["op_cov_def"] = op_cov_def
                new_instr["op_cov_constr"] = f"    m_{mnemonic}_instruction_ops_cov = new;\n"
                new_instr["op_cov_sample"] = op_cov_sample

            if val_cov_def:
                new_instr["val_cov_def"] = val_cov_def
                new_instr["val_cov_constr"] = f"    foreach (m_{mnemonic}_reg_cov[i]) m_{mnemonic}_reg_cov[i] = new;\n"
                new_instr["val_cov_sample"] = val_cov_sample

            if res_val_cov_def:
                new_instr["res_val_cov_def"] = res_val_cov_def
                new_instr["res_val_cov_constr"] = f"    m_{mnemonic}_res_reg_cov = new;\n"

            if op3_val_cov_def:
                new_instr["op3_val_cov_def"] = op3_val_cov_def
                new_instr["op3_val_cov_constr"] = f"    m_{mnemonic}_op3_reg_cov = new;\n"

            if imm_cov_def:
                new_instr["imm_cov_def"] = imm_cov_def
                new_instr["imm_cov_constr"] = f"    m_{mnemonic}_imm_cov = new;\n"
                new_instr["imm_cov_sample"] = imm_cov_sample

            # Accounts for both, NewPC and Address
            if newpc_cov_def:
                new_instr["newpc_cov_def"] = newpc_cov_def
                new_instr["newpc_cov_constr"] = f"    m_{mnemonic}_newpc_cov = new;\n"
                new_instr["newpc_cov_sample"] = newpc_cov_sample

            if addr_cov_def:
                new_instr["addr_cov_def"] = addr_cov_def
                new_instr["addr_cov_constr"] = f"    m_{mnemonic}_addr_cov = new;\n"
                new_instr["addr_cov_sample"] = addr_cov_sample

            if cross_cov_def:
                new_instr["cross_cov_def"] = cross_cov_def
                new_instr["cross_cov_constr"] = f"    m_{mnemonic}_op_cross_cov = new;\n"
                new_instr["cross_cov_sample"] = cross_cov_sample

            # Add to context to be passed to Jinja
            i_ctx["instructions"].append(new_instr)


    # Open output file
    with open(args.output, "w") as fd_of_:
        fd_of_.write(template_.render(i_ctx))

    sys.exit(0)
