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

static uint256_t mask_32_lsb_bits(const uint256_t &val)
{
    return val & uint256_t("0xFFFFFFFF");
}

static bool is_32_lsb_bits_zero(const uint256_t &val)
{
    return ((val & (uint256_t("0xFFFFFFFF"))) == uint256_t("0x000000000"));
}

template<class T>
static void binary_logic_op_32_lsb(const uint256_t &lhs, const uint256_t &rhs, uint256_t &res, T&&op)
{
    // Logic operation on 32 LSBs between lhs and rhs and storing result to res.
    // Keeps 256:32 of res intact.
    uint256_t mask_a = mask_32_lsb_bits(lhs);
    uint256_t mask_b = mask_32_lsb_bits(rhs);

    std::string mask_msb = std::string("0x") + std::string(56, 'F') + std::string(8, '0');
    uint256_t mask_res = res & uint256_t(mask_msb.c_str());

    uint256_t op_res = op(mask_a, mask_b);
    res = mask_res | op_res;
}

static uint512_t get_p_25519()
{
    uint512_t tmp = uint512_t("0x1");
    tmp = tmp << 255;
    return tmp - uint512_t("19");
}

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
///////////////////////////////////////////////////////////////////////////////////////////////////
// Semantics of instruction execution
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// R Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_R_32_AIRTH_OP(classname,operand,store_res)                                \
    bool spect::classname::Execute()                                                        \
    {                                                                                       \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                \
        DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);                         \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->flags_.zero);                        \
            PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);                 \
        }                                                                                   \
                                                                                            \
        uint256_t add = model_->gpr_[TO_INT(op2_)] operand model_->gpr_[TO_INT(op3_)];      \
        uint256_t mask = mask_32_lsb_bits(add);                                             \
        if (store_res)                                                                      \
            model_->gpr_[TO_INT(op1_)] = mask;                                              \
        model_->flags_.zero = is_32_lsb_bits_zero(mask);                                    \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->flags_.zero);                        \
            PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);                 \
            if (store_res)                                                                  \
                model_->change_q_.push(ch_gpr);                                             \
            model_->change_q_.push(ch_zf);                                                  \
        }                                                                                   \
        return true;                                                                        \
    }

IMPLEMENT_R_32_AIRTH_OP(InstructionADD,+,true)
IMPLEMENT_R_32_AIRTH_OP(InstructionSUB,-,true)
IMPLEMENT_R_32_AIRTH_OP(InstructionCMP,-,false)


#define IMPLEMENT_R_32_LOGIC_OP(classname,operand)                                          \
    bool spect::classname::Execute()                                                        \
    {                                                                                       \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                \
        DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);                         \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->flags_.zero);                        \
            PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);                 \
        }                                                                                   \
                                                                                            \
        binary_logic_op_32_lsb(model_->gpr_[TO_INT(op2_)],                                  \
                            model_->gpr_[TO_INT(op3_)],                                     \
                            model_->gpr_[TO_INT(op1_)],                                     \
            [] (const uint256_t &lhs, const uint256_t &rhs) -> uint256_t {                  \
                return lhs operand rhs;                                                     \
            }                                                                               \
        );                                                                                  \
        model_->flags_.zero = is_32_lsb_bits_zero(model_->gpr_[TO_INT(op1_)]);              \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->flags_.zero);                        \
            PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);                 \
            model_->change_q_.push(ch_gpr);                                                 \
            model_->change_q_.push(ch_zf);                                                  \
        }                                                                                   \
        return true;                                                                        \
    }

IMPLEMENT_R_32_LOGIC_OP(InstructionAND, &)
IMPLEMENT_R_32_LOGIC_OP(InstructionOR, |)
IMPLEMENT_R_32_LOGIC_OP(InstructionXOR, ^)


bool spect::InstructionNOT::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);

    if (model_->change_reporting_) {
        PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->flags_.zero);
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);
    }

    binary_logic_op_32_lsb(model_->gpr_[TO_INT(op2_)],
                           model_->gpr_[TO_INT(op3_)],
                           model_->gpr_[TO_INT(op1_)],
        [] (const uint256_t &lhs, const uint256_t &rhs) -> uint256_t {
            // Need copy, since ~ modifies passed reference
            uint256_t cpy = lhs;
            return mask_32_lsb_bits(~cpy);
        }
    );
    model_->flags_.zero = is_32_lsb_bits_zero(model_->gpr_[TO_INT(op1_)]);

    if (model_->change_reporting_) {
        PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->flags_.zero);
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);
        model_->change_q_.push(ch_gpr);
        model_->change_q_.push(ch_zf);
    }

    return true;
}

#define IMPLEMENT_R_32_SHIFT_OP(classname,op_shift,op_opposite,n_bits,rotate,set_carry)     \
    bool spect::classname::Execute()                                                        \
    {                                                                                       \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                \
        DEFINE_CHANGE(ch_cf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_CARRY);                        \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_cf, old_val, model_->flags_.carry);                       \
            PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);                 \
        }                                                                                   \
                                                                                            \
        const uint256_t &op2 = model_->gpr_[TO_INT(op2_)];                                  \
        uint256_t tmp = op2 op_shift n_bits;                                                \
                                                                                            \
        if (rotate) {                                                                       \
            uint256_t rotated = op2 op_opposite (256 - n_bits);                             \
            tmp = tmp | rotated;                                                            \
        }                                                                                   \
        model_->gpr_[TO_INT(op1_)] = tmp;                                                   \
        if (set_carry) {                                                                    \
            std::string mask_str = std::string("0x") + std::string("8") +                   \
                                   std::string(62, '0') +                                   \
                                   std::string("1");                                        \
            uint256_t mask = uint256_t(mask_str.c_str());                                   \
            mask = mask op_shift 255;                                                       \
            model_->flags_.carry = (op2 & mask) == uint256_t("0x0") ? false : true;         \
        }                                                                                   \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_cf, new_val, model_->flags_.carry);                       \
            PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);                 \
            model_->change_q_.push(ch_gpr);                                                 \
            if (set_carry)                                                                  \
                model_->change_q_.push(ch_cf);                                              \
        }                                                                                   \
                                                                                            \
        return true;                                                                        \
    }

IMPLEMENT_R_32_SHIFT_OP(InstructionLSL,     <<, >>, 1, false, true)
IMPLEMENT_R_32_SHIFT_OP(InstructionLSR,     >>, <<, 1, false, true)
IMPLEMENT_R_32_SHIFT_OP(InstructionROL,     <<, >>, 1, true,  true)
IMPLEMENT_R_32_SHIFT_OP(InstructionROR,     >>, <<, 1, true,  true)
IMPLEMENT_R_32_SHIFT_OP(InstructionROL8,    <<, >>, 8, true,  false)
IMPLEMENT_R_32_SHIFT_OP(InstructionROR8,    >>, <<, 8, true,  false)


bool spect::InstructionSWE::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    if (model_->change_reporting_)
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);

    uint256_t result = uint256_t("0x0");

    for (int i = 0; i < 32; i++) {
        uint256_t mask = uint256_t("0xFF");
        mask = (mask << (8 * i));
        uint256_t tmp = model_->gpr_[TO_INT(op2_)] & mask;
        if (i < 16)
            tmp = tmp << ((31 - 2 * i) * 8);
        else
            tmp = tmp >> ((i - 15) * 16 - 8);
        result = result | tmp;
    }

    model_->gpr_[TO_INT(op1_)] = result;

    if (model_->change_reporting_) {
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);
        model_->change_q_.push(ch_gpr);
    }

    return true;
}

bool spect::InstructionMOV::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    if (model_->change_reporting_)
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);

    model_->gpr_[TO_INT(op1_)] = model_->gpr_[TO_INT(op2_)];

    if (model_->change_reporting_) {
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);
        model_->change_q_.push(ch_gpr);
    }

    return true;
}

bool spect::InstructionCSWAP::Execute()
{
    DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, TO_INT(op2_));

    if (model_->change_reporting_){
        PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->gpr_[TO_INT(op1_)]);
        PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->gpr_[TO_INT(op2_)]);
    }

    if (model_->flags_.carry) {
        uint256_t tmp = model_->gpr_[TO_INT(op2_)];
        model_->gpr_[TO_INT(op2_)] = model_->gpr_[TO_INT(op1_)];
        model_->gpr_[TO_INT(op1_)] = tmp;
    }

    if (model_->change_reporting_){
        PUT_GPR_TO_CHANGE(ch_gpr_1, new_val, model_->gpr_[TO_INT(op1_)]);
        PUT_GPR_TO_CHANGE(ch_gpr_2, new_val, model_->gpr_[TO_INT(op2_)]);
        model_->change_q_.push(ch_gpr_1);
        model_->change_q_.push(ch_gpr_2);
    }

    return true;
}

bool spect::InstructionHASH::Execute()
{
    DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, (TO_INT(op1_) + 1) % 32);

    if (model_->change_reporting_){
        PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->gpr_[TO_INT(op1_)]);
        PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->gpr_[(TO_INT(op1_) + 1) % 32]);
    }

    uint1024_t tmp = uint1024_t("0");
    for (int i = 3; i >=0; i--) {
        uint1024_t padded = model_->gpr_[(TO_INT(op2_) + i) % 32];
        tmp = tmp | padded;
        if (i > 0)
            tmp = tmp << 256;
    }
    // TODO: Clena-up
    //std::cout << "Hash input:" << std::endl;
    //std::cout << tmp << std::endl;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(256) << std::hex << tmp;
    //std::cout << "Hash input stream:" << std::endl;
    //std::cout << ss.str() << std::endl;

    model_->sha_512_.update((unsigned char*)ss.str().c_str(), 1024);

    // Get current state of HASH context
    for (int i = 0; i < 2; i++) {
        uint256_t reg = uint256_t(0);
        for (int j = 0; j < 4; j++) {
            uint256_t state = uint256_t(model_->sha_512_.getContext(i * 4 + j));
            reg = reg | (state << (j * 64));
        }
        model_->gpr_[(TO_INT(op1_) + i) % 32] = reg;
    }

    // TODO: Check that HASH calculates correctly!!!
    //std::cout << "Hash result:" << std::endl;
    //std::cout << std::hex << model_->gpr_[(TO_INT(op1_) + 1) % 32] << std::endl;
    //std::cout << std::hex << model_->gpr_[TO_INT(op1_)] << std::endl;

    if (model_->change_reporting_){
        PUT_GPR_TO_CHANGE(ch_gpr_1, new_val, model_->gpr_[TO_INT(op1_)]);
        PUT_GPR_TO_CHANGE(ch_gpr_2, new_val, model_->gpr_[(TO_INT(op1_) + 1) % 32]);
        model_->change_q_.push(ch_gpr_1);
        model_->change_q_.push(ch_gpr_2);
    }

    return true;
}

bool spect::InstructionGRV::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    if (model_->change_reporting_)
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);

    model_->gpr_[TO_INT(op1_)] = model_->grv_q_.front();
    model_->grv_q_.pop();

    if (model_->change_reporting_) {
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);
        model_->change_q_.push(ch_gpr);
    }

    return true;
}

bool spect::InstructionSCB::Execute()
{
    DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, (TO_INT(op1_) + 1) % 32);

    if (model_->change_reporting_){
        PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->gpr_[TO_INT(op1_)]);
        PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->gpr_[(TO_INT(op1_) + 1) % 32]);
    }

    uint512_t tmp = model_->gpr_[TO_INT(op3_)];
    std::string mask = "0xA";
    mask += std::string(63, '0');
    tmp = tmp | uint512_t(mask.c_str());
    tmp = tmp * uint512_t(model_->gpr_[TO_INT(CpuGpr::R31)]);
    tmp = tmp + uint512_t(model_->gpr_[TO_INT(op2_)]);

    // TODO: Check implicit conversion takes LSBS!
    model_->gpr_[TO_INT(op1_)] = tmp;
    model_->gpr_[(TO_INT(op1_) + 1) % 32] = (tmp >> 256);

    if (model_->change_reporting_){
        PUT_GPR_TO_CHANGE(ch_gpr_1, new_val, model_->gpr_[TO_INT(op1_)]);
        PUT_GPR_TO_CHANGE(ch_gpr_2, new_val, model_->gpr_[(TO_INT(op1_) + 1) % 32]);
        model_->change_q_.push(ch_gpr_1);
        model_->change_q_.push(ch_gpr_2);
    }

    return true;
}

#define IMPLEMENT_MODULAR_OP(classname,operation, mod_num)                                  \
    bool spect::classname::Execute()                                                        \
    {                                                                                       \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                \
        if (model_->change_reporting_)                                                      \
            PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);                 \
                                                                                            \
        uint512_t op2 = (uint512_t)model_->gpr_[TO_INT(op2_)];                              \
        uint512_t op3 = (uint512_t)model_->gpr_[TO_INT(op3_)];                              \
        uint512_t tmp = operation;                                                          \
        model_->gpr_[TO_INT(op1_)] = tmp % (uint512_t)mod_num;                              \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);                 \
            model_->change_q_.push(ch_gpr);                                                 \
        }                                                                                   \
                                                                                            \
        return true;                                                                        \
    }

IMPLEMENT_MODULAR_OP(InstructionMUL25519, (op2 * op3),          get_p_25519())
IMPLEMENT_MODULAR_OP(InstructionMUL256,   (op2 * op3),          get_p_256())
IMPLEMENT_MODULAR_OP(InstructionADDP,     (op2 + op3),          model_->gpr_[TO_INT(CpuGpr::R31)])
IMPLEMENT_MODULAR_OP(InstructionSUBP,     (op2 + op3),          model_->gpr_[TO_INT(CpuGpr::R31)])
IMPLEMENT_MODULAR_OP(InstructionMULP,     (op2 + op3),          model_->gpr_[TO_INT(CpuGpr::R31)])
IMPLEMENT_MODULAR_OP(InstructionREDP,    ((op2 << 256) | op3),  model_->gpr_[TO_INT(CpuGpr::R31)])



///////////////////////////////////////////////////////////////////////////////////////////////////
// I Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_I_32_AIRTH_OP(classname,operand,store_res)                                \
    bool spect::classname::Execute()                                                        \
    {                                                                                       \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                \
        DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);                         \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->flags_.zero);                        \
            PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);                 \
        }                                                                                   \
                                                                                            \
        uint256_t tmp = model_->gpr_[TO_INT(op2_)] operand uint256_t(immediate_);           \
        uint256_t mask = tmp & uint256_t("0xFFFFFFFF");                                     \
        if (store_res)                                                                      \
            model_->gpr_[TO_INT(op1_)] = mask;                                              \
        model_->flags_.zero = is_32_lsb_bits_zero(mask);                                    \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->flags_.zero);                        \
            PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);                 \
            if (store_res)                                                                  \
                model_->change_q_.push(ch_gpr);                                             \
            model_->change_q_.push(ch_zf);                                                  \
        }                                                                                   \
                                                                                            \
        return true;                                                                        \
    }

IMPLEMENT_I_32_AIRTH_OP(InstructionADDI,+,true)
IMPLEMENT_I_32_AIRTH_OP(InstructionSUBI,-,true)
IMPLEMENT_I_32_AIRTH_OP(InstructionCMPI,-,false)


#define IMPLEMENT_I_32_LOGIC_OP(classname,operand)                                          \
    bool spect::classname::Execute()                                                        \
    {                                                                                       \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                \
        DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);                         \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->flags_.zero);                        \
            PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);                 \
        }                                                                                   \
                                                                                            \
        binary_logic_op_32_lsb(model_->gpr_[TO_INT(op2_)],                                  \
                            uint256_t(immediate_),                                          \
                            model_->gpr_[TO_INT(op1_)],                                     \
            [] (const uint256_t &lhs, const uint256_t &rhs) -> uint256_t {                  \
                return lhs operand rhs;                                                     \
            }                                                                               \
        );                                                                                  \
        model_->flags_.zero = is_32_lsb_bits_zero(model_->gpr_[TO_INT(op1_)]);              \
                                                                                            \
        if (model_->change_reporting_) {                                                    \
            PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->flags_.zero);                        \
            PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);                 \
            model_->change_q_.push(ch_gpr);                                                 \
            model_->change_q_.push(ch_zf);                                                  \
        }                                                                                   \
                                                                                            \
        return true;                                                                        \
    }

IMPLEMENT_I_32_LOGIC_OP(InstructionANDI,&)
IMPLEMENT_I_32_LOGIC_OP(InstructionORI,|)
IMPLEMENT_I_32_LOGIC_OP(InstructionXORI,^)


bool spect::InstructionCMPA::Execute()
{
    DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);

    if (model_->change_reporting_)
        PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->flags_.zero);

    model_->flags_.zero = (model_->gpr_[TO_INT(op2_)] == uint256_t(immediate_));

    if (model_->change_reporting_) {
        PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->flags_.zero);
        model_->change_q_.push(ch_zf);
    }

    return true;
}

bool spect::InstructionMOVI::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    if (model_->change_reporting_)
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);

    model_->gpr_[TO_INT(op1_)] = uint256_t(immediate_);

    if (model_->change_reporting_) {
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);
        model_->change_q_.push(ch_gpr);
    }

    return true;
}

bool spect::InstructionHASH_IT::Execute()
{
    model_->sha_512_.init();
    return true;
}

bool spect::InstructionGPK::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    if (model_->change_reporting_)
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);

    int index = immediate_ & 0x7;
    model_->gpr_[TO_INT(op1_)] = model_->gpk_q_[index].front();
    model_->gpk_q_[index].pop();

    if (model_->change_reporting_) {
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);
        model_->change_q_.push(ch_gpr);
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// M Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

bool spect::InstructionLD::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));

    if (model_->change_reporting_)
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->gpr_[TO_INT(op1_)]);

    std::stringstream str;
    str << "0x" << std::hex << std::setfill('0') << std::setw(32);
    for (int i = 7; i >=0; i--)
        str << model_->ReadMemoryCoreData(addr_ + i);
    model_->gpr_[TO_INT(op1_)] = uint256_t(str.str().c_str());

    if (model_->change_reporting_) {
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->gpr_[TO_INT(op1_)]);
        model_->change_q_.push(ch_gpr);
    }

    return true;
}

bool spect::InstructionST::Execute()
{
    uint256_t tmp =  model_->gpr_[TO_INT(op1_)];
    for (int i = 0; i < 8; i++) {

        DEFINE_CHANGE(ch_mem, DPI_CHANGE_MEM, addr_ + i);
        if (model_->change_reporting_)
            ch_mem.old_val[0] = model_->GetMemory(addr_ + i);

        uint32_t written = model_->WriteMemoryCoreData(addr_ + i,
                            static_cast<uint32_t>(tmp & uint256_t("0xFFFFFFFF")));

        if (model_->change_reporting_) {
            ch_mem.new_val[0] = written;
            model_->change_q_.push(ch_mem);
        }

        tmp = tmp >> 32;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// J Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

bool spect::InstructionCALL::Execute()
{
    DEFINE_CHANGE(ch_rar, DPI_CHANGE_RAR, new_pc_);
    DEFINE_CHANGE(ch_rar_sp, DPI_CHANGE_RAR_SP, 0);
    if (model_->change_reporting_) {
        ch_rar.old_val[0] = model_->rar_stack_[model_->rar_sp_];
        ch_rar_sp.old_val[0] = model_->rar_sp_;
    }

    model_->rar_stack_[model_->rar_sp_] = model_->pc_ + 0x4;
    model_->rar_sp_ += 1;
    model_->pc_ = new_pc_;

    if (model_->change_reporting_) {
        ch_rar.new_val[0] = model_->rar_stack_[model_->rar_sp_ - 1];
        ch_rar_sp.new_val[0] = model_->rar_sp_;
        model_->change_q_.push(ch_rar);
        model_->change_q_.push(ch_rar_sp);
    }

    return false;

    // TODO: Assert on RAR stack overflow?
}

bool spect::InstructionRET::Execute()
{
    DEFINE_CHANGE(ch_rar_sp, DPI_CHANGE_RAR_SP, new_pc_);
    if (model_->change_reporting_)
        ch_rar_sp.old_val[0] = model_->rar_sp_;

    model_->pc_ = model_->rar_stack_[model_->rar_sp_];
    model_->rar_sp_ -= 1;

    if (model_->change_reporting_) {
        ch_rar_sp.new_val[0] = model_->rar_sp_;
        model_->change_q_.push(ch_rar_sp);
    }

    return false;
}

#define IMPLEMENT_COND_JUMP_OP(classname,flag_name,value)                                   \
    bool spect::classname::Execute()                                                        \
    {                                                                                       \
        if (model_->flags_.flag_name == value){                                             \
            model_->pc_ = new_pc_;                                                          \
            return false;                                                                   \
        }                                                                                   \
        return true;                                                                        \
    }

IMPLEMENT_COND_JUMP_OP(InstructionBRZ,zero,true)
IMPLEMENT_COND_JUMP_OP(InstructionBRNZ,zero,false)
IMPLEMENT_COND_JUMP_OP(InstructionBRC,carry,true)
IMPLEMENT_COND_JUMP_OP(InstructionBRNC,carry,false)

bool spect::InstructionJMP::Execute()
{
    model_->pc_ = new_pc_;
    return false;
}

bool spect::InstructionEND::Execute()
{
    DEFINE_CHANGE(ch_srr, DPI_CHANGE_SRR, 0);
    if (model_->change_reporting_) {
        PUT_GPR_TO_CHANGE(ch_srr, old_val, model_->srr_);
    }

    model_->srr_ = model_->gpr_[31];
    model_->end_executed_ = true;

    ordt_data wdata(1,0);
    wdata[0] = 1;
    model_->regs_->r_status.f_idle.write(wdata);
    model_->regs_->r_status.f_done.write(wdata);
    model_->UpdateInterrupts();

    if (model_->change_reporting_) {
        PUT_GPR_TO_CHANGE(ch_srr, new_val, model_->srr_);
        model_->change_q_.push(ch_srr);
    }

    return false;
}

bool spect::InstructionNOP::Execute()
{
    return true;
}