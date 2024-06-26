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

IMPLEMENT_R_32_AIRTH_OP(V2InstructionADD,+,true)
IMPLEMENT_R_32_AIRTH_OP(V2InstructionSUB,-,true)
IMPLEMENT_R_32_AIRTH_OP(V2InstructionCMP,-,false)


#define IMPLEMENT_R_LOGIC_OP(classname,operand)                                                 \
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
                                64,                                                             \
                [] (const uint256_t &lhs, const uint256_t &rhs) -> uint256_t {                  \
                    return lhs operand rhs;                                                     \
                }                                                                               \
            ));                                                                                 \
        bool new_flag_val = (model_->GetGpr(TO_INT(op1_)) == uint256_t("0x0"));                 \
        model_->SetCpuFlag(CpuFlagType::ZERO, new_flag_val);                                    \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                       \
        model_->ReportChange(ch_gpr);                                                           \
        model_->ReportChange(ch_zf);                                                            \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_R_LOGIC_OP(V2InstructionAND, &)
IMPLEMENT_R_LOGIC_OP(V2InstructionOR, |)
IMPLEMENT_R_LOGIC_OP(V2InstructionXOR, ^)


bool spect::V2InstructionNOT::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);

    PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr( TO_INT(op1_),
        binary_logic_op_lsb(model_->GetGpr(TO_INT(op2_)),
                            model_->GetGpr(TO_INT(op3_)),
                            64,
            [] (const uint256_t &lhs, [[maybe_unused]] const uint256_t &rhs) -> uint256_t {
                // Need copy, since ~ modifies passed reference
                uint256_t cpy = lhs;
                return mask_n_lsb_digits(~cpy, 64);
            }
        ));
    model_->SetCpuFlag(CpuFlagType::ZERO, model_->GetGpr(TO_INT(op1_)) == uint256_t("0x0"));

    PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));
    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);
    model_->ReportChange(ch_zf);

    return true;
}

bool spect::V2InstructionSBIT::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    uint256_t shift = model_->GetGpr(TO_INT(op3_)) & uint256_t("0xFF");
    uint256_t mask  = uint256_t("0x1") << static_cast<uint32_t>(shift);
    model_->SetGpr( TO_INT(op1_), model_->GetGpr(TO_INT(op2_)) | mask);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V2InstructionCBIT::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    uint256_t shift = model_->GetGpr(TO_INT(op3_)) & uint256_t("0xFF");
    uint256_t mask  = uint256_t("0x1") << static_cast<uint32_t>(shift);
    model_->SetGpr( TO_INT(op1_), model_->GetGpr(TO_INT(op2_)) & ~mask);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

#define IMPLEMENT_R_32_SHIFT_OP(classname,op_shift,op_opposite,n_bits,rotate,op3_in,set_carry)      \
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
        if (op3_in) {                                                                               \
            const uint256_t &op3 = model_->GetGpr(TO_INT(op3_));                                    \
            uint256_t rotated = op3 op_opposite (256 - n_bits);                                     \
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

IMPLEMENT_R_32_SHIFT_OP(V2InstructionLSL,     <<, >>, 1, false, false, true)
IMPLEMENT_R_32_SHIFT_OP(V2InstructionLSR,     >>, <<, 1, false, false, true)
IMPLEMENT_R_32_SHIFT_OP(V2InstructionROL,     <<, >>, 1, true,  false, true)
IMPLEMENT_R_32_SHIFT_OP(V2InstructionROR,     >>, <<, 1, true,  false, true)
IMPLEMENT_R_32_SHIFT_OP(V2InstructionROL8,    <<, >>, 8, true,  false, false)
IMPLEMENT_R_32_SHIFT_OP(V2InstructionROR8,    >>, <<, 8, true,  false, false)
IMPLEMENT_R_32_SHIFT_OP(V2InstructionROLIN,   <<, >>, 8, false, true,  false)
IMPLEMENT_R_32_SHIFT_OP(V2InstructionRORIN,   >>, <<, 8, false, true,  false)


bool spect::V2InstructionSWE::Execute()
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

bool spect::V2InstructionMOV::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr(TO_INT(op1_), model_->GetGpr(TO_INT(op2_)));

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V2InstructionLDR::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    uint16_t  addr = static_cast<uint16_t>(model_->GetGpr(TO_INT(op2_)));
    uint256_t tmp  = 0;
    for (int i = 0; i < 8; i++) {
        uint32_t buf = model_->ReadMemoryCoreData(addr + (4 * i));
        tmp = (uint256_t(buf) << (i * 32)) | tmp;
    }
    model_->SetGpr(TO_INT(op1_), tmp);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V2InstructionSTR::Execute()
{
    uint16_t  addr = static_cast<uint16_t>(model_->GetGpr(TO_INT(op2_)));
    uint256_t tmp  = model_->GetGpr(TO_INT(op1_));
    for (int i = 0; i < 8; i++) {
        model_->WriteMemoryCoreData(addr + (i * 4),
                    static_cast<uint32_t>(tmp & uint256_t("0xFFFFFFFF")));
        tmp = tmp >> 32;
    }
    return true;
}

#define IMPLEMENT_SWAP_OP(classname, flag_name)                                                 \
    bool spect::classname::Execute()                                                            \
    {                                                                                           \
        DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));                                  \
        DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, TO_INT(op2_));                                  \
                                                                                                \
        PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->GetGpr(TO_INT(op1_)));                     \
        PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->GetGpr(TO_INT(op2_)));                     \
                                                                                                \
        if (model_->GetCpuFlags().flag_name){                                                   \
            uint256_t tmp = model_->GetGpr(TO_INT(op2_));                                       \
            model_->SetGpr(TO_INT(op2_), model_->GetGpr(TO_INT(op1_)));                         \
            model_->SetGpr(TO_INT(op1_), tmp);                                                  \
        }                                                                                       \
                                                                                                \
        PUT_GPR_TO_CHANGE(ch_gpr_1, new_val, model_->GetGpr(TO_INT(op1_)));                     \
        PUT_GPR_TO_CHANGE(ch_gpr_2, new_val, model_->GetGpr(TO_INT(op2_)));                     \
        model_->ReportChange(ch_gpr_1);                                                         \
        model_->ReportChange(ch_gpr_2);                                                         \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_SWAP_OP(V2InstructionCSWAP, carry)
IMPLEMENT_SWAP_OP(V2InstructionZSWAP, zero)


bool spect::V2InstructionHASH::Execute()
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

bool spect::V2InstructionGRV::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    uint256_t tmp = 0;
    for (int i = 0; i < 8; i++) {
        DEFINE_CHANGE(ch_rbus, DPI_CHANGE_RBUS, (i == 0) ? DPI_RBUS_FRESH_ENT : DPI_RBUS_NO_FRESH_ENT);

        uint256_t part = model_->GrvQueuePop();
        part = part << (32 * i);
        tmp = tmp | part;

        model_->ReportChange(ch_rbus);
    }

    model_->SetGpr(TO_INT(op1_), tmp);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V2InstructionSCB::Execute()
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

IMPLEMENT_MODULAR_OP(V2InstructionMUL25519, (op2 * op3),          get_p_25519()                          ,true)
IMPLEMENT_MODULAR_OP(V2InstructionMUL256,   (op2 * op3),          get_p_256()                            ,true)
IMPLEMENT_MODULAR_OP(V2InstructionADDP,     (op2 + op3),          model_->GetGpr(TO_INT(CpuGpr::R31))    ,true)
IMPLEMENT_MODULAR_OP(V2InstructionMULP,     (op2 * op3),          model_->GetGpr(TO_INT(CpuGpr::R31))    ,false)
IMPLEMENT_MODULAR_OP(V2InstructionREDP,    ((op2 << 256) | op3),  model_->GetGpr(TO_INT(CpuGpr::R31))    ,false)


bool spect::V2InstructionSUBP::Execute()
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

bool spect::V2InstructionTMAC_IT::Execute()
{
    // Initialize Keccak
    if (KeccakWidth400_SpongeInitialize(&(model_->keccak_inst_), KECCAK_RATE, KECCAK_CAPACITY) != 0) {
        std::stringstream ss;
        ss << "Error: Calling KeccakWidth400_SpongeInitialize() failed.";
        model_->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
    }

    return true;
}

bool spect::V2InstructionTMAC_UP::Execute()
{
    unsigned char msg[KECCAK_RATE/8];
    std::stringstream ss;

    // Convert register op2_ to input message (must be character stream)
    uint256_t tmp = model_->GetGpr(TO_INT(op2_));
    for (int i = 0; i < KECCAK_RATE/8; i++) {
        msg[i] = (uint8_t)((tmp >> (136 - (i * 8))) & uint256_t("0xFF"));
    }

    // Print Message
    model_->DebugInfo(VERBOSITY_HIGH, "Keccak input message:");
    ss << std::hex << std::setw(2);
    for (int i = 0; i < KECCAK_RATE/8; i++)
        ss << (int)msg[i] << " ";
    model_->DebugInfo(VERBOSITY_HIGH, ss.str().c_str());
    model_->DebugInfo(VERBOSITY_HIGH, "");
    ss.str("");

    // Process by Keccak
    if (KeccakWidth400_SpongeAbsorb(&(model_->keccak_inst_), (unsigned char *)msg, KECCAK_RATE/8) != 0) {
        ss << "Error: Calling KeccakWidth400_SpongeAbsorb() failed.";
        model_->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
    }

    return true;
}

bool spect::V2InstructionTMAC_RD::Execute()
{
    unsigned char msg[KECCAK_CAPACITY/8];
    std::stringstream ss;

    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    // Move to squeezing phase
    model_->keccak_inst_.squeezing = 1;

    // Get Keccak output
    if (KeccakWidth400_SpongeSqueeze(&(model_->keccak_inst_), (unsigned char *)msg, KECCAK_CAPACITY/8) != 0) {
        ss << "Error: Calling KeccakWidth400_SpongeSqueeze() failed.";
        model_->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
    }

    // Print Message
    model_->DebugInfo(VERBOSITY_HIGH, "Keccak output message:");
    ss << std::hex << std::setw(2);
    for (int i = 0; i < KECCAK_CAPACITY/8; i++)
        ss << (int)msg[i] << " ";
    model_->DebugInfo(VERBOSITY_HIGH, ss.str().c_str());
    model_->DebugInfo(VERBOSITY_HIGH, "");

    // Convert output message to register op1_
    uint256_t reg = uint256_t(0);
    for (int i = 0; i < 32; i++) {
        uint256_t tmp = uint256_t(msg[i]);
        reg = reg | (tmp << (248 - (i * 8)));
    }

    model_->SetGpr(TO_INT(op1_), reg);

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

IMPLEMENT_I_32_AIRTH_OP(V2InstructionADDI,+,true)
IMPLEMENT_I_32_AIRTH_OP(V2InstructionSUBI,-,true)
IMPLEMENT_I_32_AIRTH_OP(V2InstructionCMPI,-,false)


#define IMPLEMENT_I_LOGIC_OP(classname,operand)                                              \
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
        bool new_flag_val = (model_->GetGpr(TO_INT(op1_)) == uint256_t("0x0"));                 \
        model_->SetCpuFlag(CpuFlagType::ZERO, new_flag_val);                                    \
                                                                                                \
        PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));              \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                       \
        model_->ReportChange(ch_gpr);                                                           \
        model_->ReportChange(ch_zf);                                                            \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_I_LOGIC_OP(V2InstructionANDI,&)
IMPLEMENT_I_LOGIC_OP(V2InstructionORI,|)
IMPLEMENT_I_LOGIC_OP(V2InstructionXORI,^)

bool spect::V2InstructionMOVI::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr(TO_INT(op1_), uint256_t(immediate_));

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::V2InstructionHASH_IT::Execute()
{
    model_->sha_512_.init();
    return true;
}

bool spect::V2InstructionTMAC_IS::Execute()
{
    // Init string in format {nonce, key length, key, 0x00, 0x00}
    unsigned char initstr[36];
    std::stringstream ss;

    // Nonce
    initstr[0] = uint8_t(immediate_ & 0xFF);
    // Key length (0x20)
    initstr[1] = 0x20;
    // Key
    uint256_t tmp = model_->GetGpr(TO_INT(op2_));
    for (int j = 0; j < 32; j++) {
        initstr[j+2] = (uint8_t)((tmp >> (248 - (j * 8))) & uint256_t("0xFF"));
    }
    // Zero bytes
    initstr[34] = 0x00;
    initstr[35] = 0x00;

    // Print Init string
    model_->DebugInfo(VERBOSITY_HIGH, "Keccak Init string:");
    ss << std::hex << std::setw(2);
    for (int i = 0; i < 36; i++)
        ss << (int)initstr[i] << " ";
    model_->DebugInfo(VERBOSITY_HIGH, ss.str().c_str());
    model_->DebugInfo(VERBOSITY_HIGH, "");
    ss.str("");

    // Process by Keccak
    for (int j = 0; j < 2; j++) {
        if (KeccakWidth400_SpongeAbsorb(&(model_->keccak_inst_), (unsigned char *)(&initstr[j*KECCAK_RATE/8]), KECCAK_RATE/8) != 0) {
            ss << "Error: Calling KeccakWidth400_SpongeAbsorb() failed.";
            model_->DebugInfo(VERBOSITY_NONE, ss.str().c_str());
        }
    }

    return true;
}

bool spect::V2InstructionLDK::Execute()
{
    uint32_t slot   = (uint32_t)(model_->GetGpr(TO_INT(op2_)) & uint256_t("0xFF"));
    uint32_t type   = (immediate_ >> 8) & 0xF;
    uint32_t offset = immediate_ & 0x1F;
    bool     error;

    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    // Read
    uint256_t tmp = 0;
    for (int i = 0; i < 8; i++) {
        DEFINE_CHANGE(ch_kbus, DPI_CHANGE_KBUS, KBUS_OBJ_ENCODE(DPI_KBUS_LDK_READ, type, slot, (offset*8+i)<<2));

        // If running with CPU Simulator, preload key from simulator Key Memory to queue
        if (model_->simulator_ != NULL) {
            uint32_t part;
            int      error_flag;

            error_flag = model_->simulator_->key_memory_->Read(type, slot, offset*8+i, part);
            model_->KbusErrorQueuePush(error_flag == 0 ? false : true);
            model_->LdkQueuePush(part);
        }

        uint256_t part = model_->LdkQueuePop();
        part = part << (32 * i);
        tmp = tmp | part;
        model_->ReportChange(ch_kbus);

        error = model_->KbusErrorQueuePop();
        DEFINE_CHANGE(ch_ef, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ERROR);
        PUT_FLAG_TO_CHANGE(ch_ef, old_val, model_->GetCpuFlag(CpuFlagType::ERROR));
        model_->SetCpuFlag(CpuFlagType::ERROR, error);
        PUT_FLAG_TO_CHANGE(ch_ef, new_val, model_->GetCpuFlag(CpuFlagType::ERROR));
        model_->ReportChange(ch_ef);

        if (error)
          return true;
    }
    model_->SetGpr(TO_INT(op1_), tmp);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}


bool spect::V2InstructionSTK::Execute()
{
    uint32_t slot   = (uint32_t)(model_->GetGpr(TO_INT(op2_)) & uint256_t("0xFF"));
    uint32_t type   = (immediate_ >> 8) & 0xF;
    uint32_t offset = immediate_ & 0x1F;
    bool     error;

    // Write
    uint256_t tmp = model_->GetGpr(TO_INT(op1_));
    for (int i = 0; i < 8; i++) {
        DEFINE_CHANGE(ch_kbus, DPI_CHANGE_KBUS, KBUS_OBJ_ENCODE(DPI_KBUS_STK_WRITE, type, slot, (offset*8+i)<<2));
        ch_kbus.new_val[0] = uint32_t(tmp >> (32 * i));
        model_->ReportChange(ch_kbus);

        // If running with CPU Simulator, store key to simulator Key Memory
        if (model_->simulator_ != NULL) {
            int error_flag = model_->simulator_->key_memory_->Write(offset*8+i, uint32_t(tmp >> (32 * i)));
            model_->KbusErrorQueuePush(error_flag == 0 ? false : true);
        }

        error = model_->KbusErrorQueuePop();
        DEFINE_CHANGE(ch_ef, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ERROR);
        PUT_FLAG_TO_CHANGE(ch_ef, old_val, model_->GetCpuFlag(CpuFlagType::ERROR));
        model_->SetCpuFlag(CpuFlagType::ERROR, error);
        PUT_FLAG_TO_CHANGE(ch_ef, new_val, model_->GetCpuFlag(CpuFlagType::ERROR));
        model_->ReportChange(ch_ef);

        if (error)
          return true;
    }

    return true;
}

bool spect::V2InstructionKBO::Execute()
{
    uint32_t slot   = (uint32_t)(model_->GetGpr(TO_INT(op2_)) & uint256_t("0xFF"));
    uint32_t type   = (immediate_ >> 8) & 0xF;
    uint32_t opcode = static_cast<dpi_kbus_change_kind_t>(immediate_ & 0xF);
    bool     error;

    DEFINE_CHANGE(ch_kbus, DPI_CHANGE_KBUS, KBUS_OBJ_ENCODE(opcode, type, slot, 0));
    model_->ReportChange(ch_kbus);

    // If running with CPU Simulator, update simulator Key Memory
    if (model_->simulator_ != NULL) {
        uint32_t data;
        int      error_flag;

        if (opcode == DPI_KBUS_WRITE)
            error_flag = model_->simulator_->key_memory_->Write(0, 0x0BAD1DEA);
        else if (opcode == DPI_KBUS_READ)
            error_flag = model_->simulator_->key_memory_->Read(type, slot, 0, data);
        else if (opcode == DPI_KBUS_PROGRAM)
            error_flag = model_->simulator_->key_memory_->Program(type, slot);
        else if (opcode == DPI_KBUS_ERASE)
            error_flag = model_->simulator_->key_memory_->Erase(type, slot);
        else if (opcode == DPI_KBUS_VERIFY)
            error_flag = model_->simulator_->key_memory_->VerifyErase(type, slot);
        else if (opcode == DPI_KBUS_FLUSH)
            error_flag = model_->simulator_->key_memory_->Flush();

        model_->KbusErrorQueuePush(error_flag == 0 ? false : true);
    }

    error = model_->KbusErrorQueuePop();

    DEFINE_CHANGE(ch_ef, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ERROR);
    PUT_FLAG_TO_CHANGE(ch_ef, old_val, model_->GetCpuFlag(CpuFlagType::ERROR));
    model_->SetCpuFlag(CpuFlagType::ERROR, error);
    PUT_FLAG_TO_CHANGE(ch_ef, new_val, model_->GetCpuFlag(CpuFlagType::ERROR));
    model_->ReportChange(ch_ef);

    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// M Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

bool spect::V2InstructionLD::Execute()
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

bool spect::V2InstructionST::Execute()
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

bool spect::V2InstructionCALL::Execute()
{
    DEFINE_CHANGE(ch_rar, DPI_CHANGE_RAR, DPI_RAR_PUSH);

    uint16_t ret_addr = model_->GetPc() + 0x4;
    model_->RarPush(ret_addr);
    model_->SetPc(new_pc_);

    ch_rar.new_val[0] = ret_addr;
    model_->ReportChange(ch_rar);

    return false;
}

bool spect::V2InstructionRET::Execute()
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

IMPLEMENT_COND_JUMP_OP(V2InstructionBRZ,zero,true)
IMPLEMENT_COND_JUMP_OP(V2InstructionBRNZ,zero,false)
IMPLEMENT_COND_JUMP_OP(V2InstructionBRC,carry,true)
IMPLEMENT_COND_JUMP_OP(V2InstructionBRNC,carry,false)
IMPLEMENT_COND_JUMP_OP(V2InstructionBRE,error,true)
IMPLEMENT_COND_JUMP_OP(V2InstructionBRNE,error,false)

bool spect::V2InstructionJMP::Execute()
{
    model_->SetPc(new_pc_);
    return false;
}

bool spect::V2InstructionEND::Execute()
{
    model_->Finish(0);
    model_->UpdateInterrupts();

    return false;
}

bool spect::V2InstructionNOP::Execute()
{
    return true;
}
