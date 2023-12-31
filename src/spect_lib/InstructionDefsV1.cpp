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

#include <iostream>

#include "InstructionDefs.h"
#include "InstructionFactory.h"

#include "ordt_pio_common.hpp"
#include "ordt_pio.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Mask LSB part of number
/// @param val Number to mask
/// @param digits Number of digits (in hexadecimal) to mask (e.g. 8 = 32 bits, 6 = 24 bits)
/// @returns Masked value
///////////////////////////////////////////////////////////////////////////////////////////////////
static uint256_t mask_n_lsb_digits(const uint256_t &val, int digits)
{
    std::string mask = std::string("0x") + std::string(digits, 'F');
    return val & uint256_t(mask.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @returns True if 32 LSBs of number are zero, False otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
static bool is_32_lsb_bits_zero(const uint256_t &val)
{
    return ((val & (uint256_t("0xFFFFFFFF"))) == uint256_t("0x000000000"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Binary logic operation on least significant bits of operands.
/// @param lhs - LHS operand
/// @param rhs - RHS operand
/// @param active_digits - Number of LSBS / 4 (number of hex digits) to execute operation on.
/// @param op - Operand to executed
/// @returns lhs op rhs
/// @note Bits 255:active_digits*4 are passed to result from lhs
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
static uint256_t binary_logic_op_lsb(const uint256_t &lhs, const uint256_t &rhs,
                                     int active_digits, T&&op)
{
    // Mask higher digits than 'active_digits' from both operands
    uint256_t mask_a = mask_n_lsb_digits(lhs, active_digits);
    uint256_t mask_b = mask_n_lsb_digits(rhs, active_digits);

    // LHS corresponds to op2_, mask its 'active_digits' LSB bits, keep only upper bits
    std::string mask_msb = std::string("0x") +
                           std::string(64 - active_digits, 'F') +
                           std::string(active_digits, '0');
    uint256_t mask_res = lhs & uint256_t(mask_msb.c_str());

    uint256_t op_res = op(mask_a, mask_b);
    uint256_t rv = mask_res | op_res;
    return rv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @returns P25519 prime (2^255 - 19)
///////////////////////////////////////////////////////////////////////////////////////////////////
static uint512_t get_p_25519()
{
    uint512_t tmp = uint512_t("0x1");
    tmp = tmp << 255;
    return tmp - uint512_t("19");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @returns P256 prime (2^256 - 2^224 + 2^192 + 2^96 - 1)
///////////////////////////////////////////////////////////////////////////////////////////////////
static uint512_t get_p_256()
{
    uint512_t msk = uint512_t("0x1");
    uint512_t tmp = (msk << 256);
    tmp -= (msk << 224);
    tmp += (msk << 192);
    tmp += (msk << 96);
    tmp -= msk;
    return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @returns Checks if inputs to modular instruction inputs are less than prime modulus.
///          This is pre-condition of HW, and if not met, its behavior is undefined.
///////////////////////////////////////////////////////////////////////////////////////////////////
static void check_modulo_conds(spect::CpuModel *model, spect::InstructionR *instr, uint256_t prime)
{
    uint256_t op2 = model->GetGpr(TO_INT(instr->op2_));
    uint256_t op3 = model->GetGpr(TO_INT(instr->op3_));
    if (op2 >= prime || op3 >= prime || prime == uint256_t("0x0") || prime == uint256_t("0x1"))
    {
        std::stringstream ss;
        ss << "Error: Input operands are not valid -> Behavior of HW is undefined. ";
        ss << "Following conditions are not met:";
        model->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
        ss.str("");

        if (op2 >= prime) {
            ss << "    op2(" << instr->op2_ << ") < 0x" << std::hex << prime;
            model->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
        }
        ss.str("");

        if (op3 >= prime) {
            ss << "    op3(" << instr->op3_ << ") < 0x" << std::hex << prime;
            model->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
        }
        ss.str("");

        if (prime == uint256_t("0x0")) {
            ss << "    R31(" << prime << ") != 0";
            model->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
        }

        if (prime == uint256_t("0x1")) {
            ss << "    R31(" << prime << ") != 1";
            model->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Semantics of instruction execution
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// R Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_R_32_AIRTH_OP(classname,operand,store_res)                                    \
    bool spect::classname::Execute()                                                            \
    {                                                                                           \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                    \
        DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);                             \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));                       \
                                                                                                \
        uint256_t add = model_->GetGpr(TO_INT(op2_)) operand model_->GetGpr(TO_INT(op3_));      \
        uint256_t mask = mask_n_lsb_digits(add, 8);                                             \
        if (store_res)                                                                          \
            model_->SetGpr(TO_INT(op1_), mask);                                                 \
        model_->SetCpuFlag(CpuFlagType::ZERO, is_32_lsb_bits_zero(mask));                       \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                       \
        if (store_res)                                                                          \
            model_->ReportChange(ch_gpr);                                                       \
        model_->ReportChange(ch_zf);                                                            \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_R_32_AIRTH_OP(V1InstructionADD,+,true)
IMPLEMENT_R_32_AIRTH_OP(V1InstructionSUB,-,true)
IMPLEMENT_R_32_AIRTH_OP(V1InstructionCMP,-,false)


#define IMPLEMENT_R_32_LOGIC_OP(classname,operand)                                              \
    bool spect::classname::Execute()                                                            \
    {                                                                                           \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                    \
        DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);                             \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));                       \
                                                                                                \
        model_->SetGpr(TO_INT(op1_),                                                            \
            binary_logic_op_lsb(model_->GetGpr(TO_INT(op2_)),                                   \
                                model_->GetGpr(TO_INT(op3_)),                                   \
                                8,                                                              \
                [] (const uint256_t &lhs, const uint256_t &rhs) -> uint256_t {                  \
                    return lhs operand rhs;                                                     \
                }                                                                               \
            ));                                                                                 \
        bool new_flag_val = is_32_lsb_bits_zero(model_->GetGpr(TO_INT(op1_)));                  \
        model_->SetCpuFlag(CpuFlagType::ZERO, new_flag_val);                                    \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                       \
        model_->ReportChange(ch_gpr);                                                           \
        model_->ReportChange(ch_zf);                                                            \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_R_32_LOGIC_OP(V1InstructionAND, &)
IMPLEMENT_R_32_LOGIC_OP(V1InstructionOR, |)
IMPLEMENT_R_32_LOGIC_OP(V1InstructionXOR, ^)


bool spect::V1InstructionNOT::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);

    PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr( TO_INT(op1_),
        binary_logic_op_lsb(model_->GetGpr(TO_INT(op2_)),
                            model_->GetGpr(TO_INT(op3_)),
                            8,
            [] (const uint256_t &lhs, [[maybe_unused]] const uint256_t &rhs) -> uint256_t {
                // Need copy, since ~ modifies passed reference
                uint256_t cpy = lhs;
                return mask_n_lsb_digits(~cpy, 8);
            }
        ));
    model_->SetCpuFlag(CpuFlagType::ZERO, is_32_lsb_bits_zero(model_->GetGpr(TO_INT(op1_))));

    PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));
    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);
    model_->ReportChange(ch_zf);

    return true;
}

#define IMPLEMENT_R_32_SHIFT_OP(classname,op_shift,op_opposite,n_bits,rotate,set_carry)             \
    bool spect::classname::Execute()                                                                \
    {                                                                                               \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                        \
        DEFINE_CHANGE(ch_cf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_CARRY);                                \
                                                                                                    \
        PUT_FLAG_TO_CHANGE(ch_cf, old_val, model_->GetCpuFlag(CpuFlagType::CARRY));                 \
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));                           \
                                                                                                    \
        const uint256_t &op2 = model_->GetGpr(TO_INT(op2_));                                        \
        uint256_t tmp = op2 op_shift n_bits;                                                        \
                                                                                                    \
        if (rotate) {                                                                               \
            uint256_t rotated = op2 op_opposite (256 - n_bits);                                     \
            tmp = tmp | rotated;                                                                    \
        }                                                                                           \
        if (set_carry) {                                                                            \
            std::string mask_str = std::string("0x") + std::string("8") +                           \
                                   std::string(62, '0') +                                           \
                                   std::string("1");                                                \
            uint256_t mask = uint256_t(mask_str.c_str());                                           \
            mask = mask op_shift 255;                                                               \
            bool new_flag_val = ((op2 & mask) == uint256_t("0x0")) ? false : true;                  \
            model_->SetCpuFlag(CpuFlagType::CARRY, new_flag_val);                                   \
        }                                                                                           \
        model_->SetGpr(TO_INT(op1_), tmp);                                                          \
                                                                                                    \
        PUT_FLAG_TO_CHANGE(ch_cf, new_val, model_->GetCpuFlag(CpuFlagType::CARRY));                 \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                           \
        model_->ReportChange(ch_gpr);                                                               \
        if (set_carry)                                                                              \
            model_->ReportChange(ch_cf);                                                            \
                                                                                                    \
        return true;                                                                                \
    }

IMPLEMENT_R_32_SHIFT_OP(V1InstructionLSL,     <<, >>, 1, false, true)
IMPLEMENT_R_32_SHIFT_OP(V1InstructionLSR,     >>, <<, 1, false, true)
IMPLEMENT_R_32_SHIFT_OP(V1InstructionROL,     <<, >>, 1, true,  true)
IMPLEMENT_R_32_SHIFT_OP(V1InstructionROR,     >>, <<, 1, true,  true)
IMPLEMENT_R_32_SHIFT_OP(V1InstructionROL8,    <<, >>, 8, true,  false)
IMPLEMENT_R_32_SHIFT_OP(V1InstructionROR8,    >>, <<, 8, true,  false)


bool spect::V1InstructionSWE::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    uint256_t result = uint256_t("0x0");

    for (int i = 0; i < 32; i++) {
        uint256_t mask = uint256_t("0xFF");
        mask = (mask << (8 * i));
        uint256_t tmp = model_->GetGpr(TO_INT(op2_)) & mask;
        if (i < 16)
            tmp = tmp << ((31 - 2 * i) * 8);
        else
            tmp = tmp >> ((i - 15) * 16 - 8);
        result = result | tmp;
    }

    model_->SetGpr(TO_INT(op1_), result);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V1InstructionMOV::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr(TO_INT(op1_), model_->GetGpr(TO_INT(op2_)));

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V1InstructionCSWAP::Execute()
{
    DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, TO_INT(op2_));

    PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->GetGpr(TO_INT(op1_)));
    PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->GetGpr(TO_INT(op2_)));

    if (model_->GetCpuFlag(CpuFlagType::CARRY)) {
        uint256_t tmp = model_->GetGpr(TO_INT(op2_));
        model_->SetGpr(TO_INT(op2_), model_->GetGpr(TO_INT(op1_)));
        model_->SetGpr(TO_INT(op1_), tmp);
    }

    PUT_GPR_TO_CHANGE(ch_gpr_1, new_val, model_->GetGpr(TO_INT(op1_)));
    PUT_GPR_TO_CHANGE(ch_gpr_2, new_val, model_->GetGpr(TO_INT(op2_)));
    model_->ReportChange(ch_gpr_1);

    // Match DUT behavior, report only single change if swapping between
    // the same registers.
    if (op1_ != op2_)
        model_->ReportChange(ch_gpr_2);

    return true;
}

bool spect::V1InstructionHASH::Execute()
{
    DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, (TO_INT(op1_) + 1) % 32);

    PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->GetGpr(TO_INT(op1_)));
    PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->GetGpr((TO_INT(op1_) + 1) % 32));

    // Convert registers op2_ .. op2_+3 to input message (must be character stream)
    unsigned char msg[128];
    for (int i = 3; i >= 0; i--) {
        uint256_t tmp = model_->GetGpr((TO_INT(op2_) + i) % 32);
        for (int j = 0; j < 32; j++) {
            uint8_t byte = (uint8_t)((tmp >> (248 - (j * 8))) & uint256_t("0xFF"));
            msg[((3 - i) * 32) + j] = byte;
        }
    }

    // Print Message
    model_->DebugInfo(VERBOSITY_HIGH, "Hash input message:");
    std::stringstream ss;
    ss << std::hex << std::setw(2);
    for (int i = 0; i < 128; i++)
        ss << (int)msg[i] << " ";
    model_->DebugInfo(VERBOSITY_HIGH, ss.str().c_str());
    model_->DebugInfo(VERBOSITY_HIGH, "");

    // Print context before, calculate, Print context after
    model_->DebugInfo(VERBOSITY_HIGH, "Hash context (before):");
    model_->PrintHashContext(VERBOSITY_HIGH);
    model_->DebugInfo(VERBOSITY_HIGH, "");

    model_->sha_512_.update((unsigned char*)msg, 128);

    model_->DebugInfo(VERBOSITY_HIGH, "Hash context (after):");
    model_->PrintHashContext(VERBOSITY_HIGH);
    model_->DebugInfo(VERBOSITY_HIGH, "");

    // Put current HASH context to op1_, op1_+1
    for (int i = 0; i < 2; i++) {
        uint256_t reg = uint256_t(0);

        for (int j = 0; j < 4; j++) {
            unsigned long long ctx = model_->sha_512_.getContext((i * 4) + j);
            reg = reg | uint256_t(ctx);
            if (j < 3)
                reg = reg << 64;
        }

        model_->SetGpr((TO_INT(op1_) + (1 - i)) % 32, reg);
    }

    PUT_GPR_TO_CHANGE(ch_gpr_1, new_val, model_->GetGpr(TO_INT(op1_)));
    PUT_GPR_TO_CHANGE(ch_gpr_2, new_val, model_->GetGpr((TO_INT(op1_) + 1) % 32));
    model_->ReportChange(ch_gpr_1);
    model_->ReportChange(ch_gpr_2);

    return true;
}

bool spect::V1InstructionGRV::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    uint256_t tmp = 0;
    for (int i = 0; i < 8; i++) {
        uint256_t part = model_->GrvQueuePop();
        part = part << (32 * i);
        tmp = tmp | part;
    }
    model_->SetGpr(TO_INT(op1_), tmp);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V1InstructionSCB::Execute()
{
    DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, (TO_INT(op1_) + 1) % 32);

    if (model_->change_reporting_){
        PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->GetGpr(TO_INT(op1_)));
        PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->GetGpr((TO_INT(op1_) + 1) % 32));
    }

    uint512_t tmp = model_->GetGpr(TO_INT(op3_));
    std::string mask = "0x80000000"; // 2 ^ 255
    mask += "80000000";              // 2 ^ 223
    mask += std::string(48, '0');    // Remaining 48 * 4 zeroes.
    tmp = tmp | uint512_t(mask.c_str());
    tmp = tmp * uint512_t(model_->GetGpr(TO_INT(CpuGpr::R31)));
    tmp = tmp + uint512_t(model_->GetGpr(TO_INT(op2_)));

    // TODO: Check implicit conversion takes LSBS!
    model_->SetGpr(TO_INT(op1_), tmp);
    model_->SetGpr((TO_INT(op1_) + 1) % 32, (tmp >> 256));

    PUT_GPR_TO_CHANGE(ch_gpr_1, new_val, model_->GetGpr(TO_INT(op1_)));
    PUT_GPR_TO_CHANGE(ch_gpr_2, new_val, model_->GetGpr((TO_INT(op1_) + 1) % 32));
    model_->ReportChange(ch_gpr_1);
    model_->ReportChange(ch_gpr_2);

    return true;
}

#define IMPLEMENT_MODULAR_OP(classname,operation, mod_num, check_ops)                           \
    bool spect::classname::Execute()                                                            \
    {                                                                                           \
        InstructionR::Execute();                                                                \
                                                                                                \
        if (check_ops)                                                                          \
            check_modulo_conds(model_, this, mod_num);                                          \
                                                                                                \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                    \
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));                       \
                                                                                                \
        uint512_t op2 = (int512_t)model_->GetGpr(TO_INT(op2_));                                 \
        uint512_t op3 = (int512_t)model_->GetGpr(TO_INT(op3_));                                 \
        uint512_t tmp = operation;                                                              \
        model_->SetGpr(TO_INT(op1_), (uint256_t)(tmp % ((uint512_t)mod_num)));                  \
                                                                                                \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                       \
        model_->ReportChange(ch_gpr);                                                           \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_MODULAR_OP(V1InstructionMUL25519, (op2 * op3),          get_p_25519()                          ,true)
IMPLEMENT_MODULAR_OP(V1InstructionMUL256,   (op2 * op3),          get_p_256()                            ,true)
IMPLEMENT_MODULAR_OP(V1InstructionADDP,     (op2 + op3),          model_->GetGpr(TO_INT(CpuGpr::R31))    ,true)
IMPLEMENT_MODULAR_OP(V1InstructionMULP,     (op2 * op3),          model_->GetGpr(TO_INT(CpuGpr::R31))    ,false)
IMPLEMENT_MODULAR_OP(V1InstructionREDP,    ((op2 << 256) | op3),  model_->GetGpr(TO_INT(CpuGpr::R31))    ,false)


bool spect::V1InstructionSUBP::Execute()
{
    InstructionR::Execute();

    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    uint512_t op2 = (uint512_t)model_->GetGpr(TO_INT(op2_));
    uint512_t op3 = (uint512_t)model_->GetGpr(TO_INT(op3_));
    uint256_t prime = model_->GetGpr(TO_INT(CpuGpr::R31));
    check_modulo_conds(model_, this, prime);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // uint_wide_t screws up modulo negative number, we do dirty trick where we add the
    // modulus to make sure that lhs operand is bigger than rhs. Since we put the restriction
    // that op2 < r31 and op3 < r31, it is enough to add single R31 to lhs to make it bigger than
    // rhs.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    uint512_t lhs = op2;
    if (op3 > op2)
        lhs += (uint512_t)prime;
    uint512_t tmp = lhs - op3;
    model_->SetGpr(TO_INT(op1_), (uint256_t)(tmp % ((uint512_t)prime)));

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// I Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_I_32_AIRTH_OP(classname,operand,store_res)                                    \
    bool spect::classname::Execute()                                                            \
    {                                                                                           \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                    \
        DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);                             \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));                       \
                                                                                                \
        const uint256_t& tmp = model_->GetGpr(TO_INT(op2_)) operand uint256_t(immediate_);      \
        uint256_t mask = tmp & uint256_t("0xFFFFFFFF");                                         \
        if (store_res)                                                                          \
            model_->SetGpr(TO_INT(op1_), mask);                                                 \
        model_->SetCpuFlag(CpuFlagType::ZERO, is_32_lsb_bits_zero(mask));                       \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                       \
        if (store_res)                                                                          \
            model_->ReportChange(ch_gpr);                                                       \
        model_->ReportChange(ch_zf);                                                            \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_I_32_AIRTH_OP(V1InstructionADDI,+,true)
IMPLEMENT_I_32_AIRTH_OP(V1InstructionSUBI,-,true)
IMPLEMENT_I_32_AIRTH_OP(V1InstructionCMPI,-,false)


#define IMPLEMENT_I_32_LOGIC_OP(classname,operand)                                              \
    bool spect::classname::Execute()                                                            \
    {                                                                                           \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                    \
        DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);                             \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));                       \
                                                                                                \
        model_->SetGpr( TO_INT(op1_),                                                           \
            binary_logic_op_lsb(model_->GetGpr(TO_INT(op2_)),                                   \
                                uint256_t(immediate_),                                          \
                                3,                                                              \
                [] (const uint256_t &lhs, const uint256_t &rhs) -> uint256_t {                  \
                    return lhs operand rhs;                                                     \
                }                                                                               \
            ));                                                                                 \
        bool new_flag_val = is_32_lsb_bits_zero(model_->GetGpr(TO_INT(op1_)));                  \
        model_->SetCpuFlag(CpuFlagType::ZERO, new_flag_val);                                    \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                       \
        model_->ReportChange(ch_gpr);                                                           \
        model_->ReportChange(ch_zf);                                                            \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_I_32_LOGIC_OP(V1InstructionANDI,&)
IMPLEMENT_I_32_LOGIC_OP(V1InstructionORI,|)
IMPLEMENT_I_32_LOGIC_OP(V1InstructionXORI,^)


bool spect::V1InstructionCMPA::Execute()
{
    DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);
    PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));

    model_->SetCpuFlag(CpuFlagType::ZERO, model_->GetGpr(TO_INT(op2_)) == uint256_t(immediate_));

    PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));
    model_->ReportChange(ch_zf);

    return true;
}

bool spect::V1InstructionMOVI::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr(TO_INT(op1_), uint256_t(immediate_));

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V1InstructionHASH_IT::Execute()
{
    model_->sha_512_.init();
    return true;
}

bool spect::V1InstructionGPK::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    int index = immediate_ & 0x7;
    uint256_t tmp = 0;
    for (int i = 0; i < 8; i++) {

        // If running with CPU Simulator, preload key from simulator memory to queue
        // GPK is V1 instruction corresponding to LDK from later ISA versions
        //
        // Also V1, allowed only to select address following keys:
        //      spect_prk_type[5:0]
        //          Bits 5:3 - 8 Different keys         -> Map to 8 slots of Key Memory
        //          Bits 2:0 - 8 words within each key  -> Map to offset within a Slot
        //
        // This-way using GPK in ISS with ISA V1 will give you data from Key Memory.
        // Locate each Key at offset 0x0 within a Slot.
        if (model_->simulator_ != NULL) {
            uint32_t part;
            model_->simulator_->key_memory_->Read(0, index, i, part);
            model_->LdkQueuePush(part);
        }

        uint256_t part = model_->LdkQueuePop();
        part = part << (32 * i);
        tmp = tmp | part;

        // ISA V1 did not have KBUS, nor E flag. Dont report KBUS transfers to TB, nor setting
        // of Error flag
    }
    model_->SetGpr(TO_INT(op1_), tmp);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// M Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

bool spect::V1InstructionLD::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    uint256_t tmp = 0;
    for (int i = 0; i < 8; i++) {
        uint32_t buf = model_->ReadMemoryCoreData(addr_ + (4 * i));
        tmp = (uint256_t(buf) << (i * 32)) | tmp;
    }
    model_->SetGpr(TO_INT(op1_), tmp);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V1InstructionST::Execute()
{
    uint256_t tmp = model_->GetGpr(TO_INT(op1_));
    for (int i = 0; i < 8; i++) {
        model_->WriteMemoryCoreData(addr_ + (i * 4),
                    static_cast<uint32_t>(tmp & uint256_t("0xFFFFFFFF")));
        tmp = tmp >> 32;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// J Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

bool spect::V1InstructionCALL::Execute()
{
    DEFINE_CHANGE(ch_rar, DPI_CHANGE_RAR, DPI_RAR_PUSH);

    uint16_t ret_addr = model_->GetPc() + 0x4;
    model_->RarPush(ret_addr);
    model_->SetPc(new_pc_);

    ch_rar.new_val[0] = ret_addr;
    model_->ReportChange(ch_rar);

    return false;
}

bool spect::V1InstructionRET::Execute()
{
    DEFINE_CHANGE(ch_rar, DPI_CHANGE_RAR, DPI_RAR_POP);

    uint16_t ret_addr = model_->RarPop();
    model_->SetPc(ret_addr);

    ch_rar.new_val[0] = ret_addr;
    model_->ReportChange(ch_rar);

    return false;
}

#define IMPLEMENT_COND_JUMP_OP(classname,flag_name,value)                                   \
    bool spect::classname::Execute()                                                        \
    {                                                                                       \
        if (model_->GetCpuFlags().flag_name == value){                                      \
            model_->SetPc(new_pc_);                                                         \
            return false;                                                                   \
        }                                                                                   \
        return true;                                                                        \
    }

IMPLEMENT_COND_JUMP_OP(V1InstructionBRZ,zero,true)
IMPLEMENT_COND_JUMP_OP(V1InstructionBRNZ,zero,false)
IMPLEMENT_COND_JUMP_OP(V1InstructionBRC,carry,true)
IMPLEMENT_COND_JUMP_OP(V1InstructionBRNC,carry,false)

bool spect::V1InstructionJMP::Execute()
{
    model_->SetPc(new_pc_);
    return false;
}

bool spect::V1InstructionEND::Execute()
{
    model_->DebugInfo(VERBOSITY_LOW, "Instruction END: SRR register from ISA version 1 is not modeled anymore !!!");

    model_->Finish(0);
    model_->UpdateInterrupts();

    return false;
}

bool spect::V1InstructionNOP::Execute()
{
    return true;
}
