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
static uint256_t binary_logic_op_32_lsb(const uint256_t &lhs, const uint256_t &rhs,
                                        const uint256_t &res, T&&op)
{
    // Logic operation on 32 LSBs between lhs and rhs and storing result to res.
    // Keeps 256:32 of res intact.
    uint256_t mask_a = mask_32_lsb_bits(lhs);
    uint256_t mask_b = mask_32_lsb_bits(rhs);

    std::string mask_msb = std::string("0x") + std::string(56, 'F') + std::string(8, '0');
    uint256_t mask_res = res & uint256_t(mask_msb.c_str());

    uint256_t op_res = op(mask_a, mask_b);
    uint256_t rv = mask_res | op_res;
    return rv;
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
        uint256_t mask = mask_32_lsb_bits(add);                                                 \
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

IMPLEMENT_R_32_AIRTH_OP(InstructionADD,+,true)
IMPLEMENT_R_32_AIRTH_OP(InstructionSUB,-,true)
IMPLEMENT_R_32_AIRTH_OP(InstructionCMP,-,false)


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
            binary_logic_op_32_lsb(model_->GetGpr(TO_INT(op2_)),                                \
                                model_->GetGpr(TO_INT(op3_)),                                   \
                                model_->GetGpr(TO_INT(op1_)),                                   \
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

IMPLEMENT_R_32_LOGIC_OP(InstructionAND, &)
IMPLEMENT_R_32_LOGIC_OP(InstructionOR, |)
IMPLEMENT_R_32_LOGIC_OP(InstructionXOR, ^)


bool spect::InstructionNOT::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);

    PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr( TO_INT(op1_),
        binary_logic_op_32_lsb(model_->GetGpr(TO_INT(op2_)),
                            model_->GetGpr(TO_INT(op3_)),
                            model_->GetGpr(TO_INT(op1_)),
            [] (const uint256_t &lhs, const uint256_t &rhs) -> uint256_t {
                // Need copy, since ~ modifies passed reference
                uint256_t cpy = lhs;
                return mask_32_lsb_bits(~cpy);
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
        model_->SetGpr(TO_INT(op1_), tmp);                                                          \
        if (set_carry) {                                                                            \
            std::string mask_str = std::string("0x") + std::string("8") +                           \
                                   std::string(62, '0') +                                           \
                                   std::string("1");                                                \
            uint256_t mask = uint256_t(mask_str.c_str());                                           \
            mask = mask op_shift 255;                                                               \
            bool new_flag_val = ((op2 & mask) == uint256_t("0x0")) ? false : true;                  \
            model_->SetCpuFlag(CpuFlagType::CARRY, new_flag_val);                                   \
        }                                                                                           \
                                                                                                    \
        PUT_FLAG_TO_CHANGE(ch_cf, new_val, model_->GetCpuFlag(CpuFlagType::CARRY));                 \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                           \
        model_->ReportChange(ch_gpr);                                                               \
        if (set_carry)                                                                              \
            model_->ReportChange(ch_cf);                                                            \
                                                                                                    \
        return true;                                                                                \
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

bool spect::InstructionMOV::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr(TO_INT(op1_), model_->GetGpr(TO_INT(op2_)));

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::InstructionCSWAP::Execute()
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
    model_->ReportChange(ch_gpr_2);

    return true;
}

bool spect::InstructionHASH::Execute()
{
    DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, (TO_INT(op1_) + 1) % 32);

    PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->GetGpr(TO_INT(op1_)));
    PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->GetGpr((TO_INT(op1_) + 1) % 32));

    uint1024_t tmp = uint1024_t("0");
    for (int i = 3; i >=0; i--) {
        uint1024_t padded = model_->GetGpr((TO_INT(op2_) + i) % 32);
        tmp = tmp | padded;
        if (i > 0)
            tmp = tmp << 256;
    }
    // TODO: Clean-up
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
        model_->SetGpr((TO_INT(op1_) + i) % 32, reg);
    }

    // TODO: Check that HASH calculates correctly!!!
    //std::cout << "Hash result:" << std::endl;
    //std::cout << std::hex << model_->GetGpr((TO_INT(op1_) + 1) % 32) << std::endl;
    //std::cout << std::hex << model_->GetGpr(TO_INT(op1_)) << std::endl;

    PUT_GPR_TO_CHANGE(ch_gpr_1, new_val, model_->GetGpr(TO_INT(op1_)));
    PUT_GPR_TO_CHANGE(ch_gpr_2, new_val, model_->GetGpr((TO_INT(op1_) + 1) % 32));
    model_->ReportChange(ch_gpr_1);
    model_->ReportChange(ch_gpr_2);

    return true;
}

bool spect::InstructionGRV::Execute()
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

bool spect::InstructionSCB::Execute()
{
    DEFINE_CHANGE(ch_gpr_1, DPI_CHANGE_GPR, TO_INT(op1_));
    DEFINE_CHANGE(ch_gpr_2, DPI_CHANGE_GPR, (TO_INT(op1_) + 1) % 32);

    if (model_->change_reporting_){
        PUT_GPR_TO_CHANGE(ch_gpr_1, old_val, model_->GetGpr(TO_INT(op1_)));
        PUT_GPR_TO_CHANGE(ch_gpr_2, old_val, model_->GetGpr((TO_INT(op1_) + 1) % 32));
    }

    uint512_t tmp = model_->GetGpr(TO_INT(op3_));
    std::string mask = "0xA";
    mask += std::string(63, '0');
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

#define IMPLEMENT_MODULAR_OP(classname,operation, mod_num)                                      \
    bool spect::classname::Execute()                                                            \
    {                                                                                           \
        DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));                                    \
        PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));                       \
                                                                                                \
        uint512_t op2 = (uint512_t)model_->GetGpr(TO_INT(op2_));                                \
        uint512_t op3 = (uint512_t)model_->GetGpr(TO_INT(op3_));                                \
        uint512_t tmp = operation;                                                              \
        model_->SetGpr(TO_INT(op1_), tmp % (uint512_t)mod_num);                                 \
                                                                                                \
        PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));                       \
        model_->ReportChange(ch_gpr);                                                           \
                                                                                                \
        return true;                                                                            \
    }

IMPLEMENT_MODULAR_OP(InstructionMUL25519, (op2 * op3),          get_p_25519())
IMPLEMENT_MODULAR_OP(InstructionMUL256,   (op2 * op3),          get_p_256())
IMPLEMENT_MODULAR_OP(InstructionADDP,     (op2 + op3),          model_->GetGpr(TO_INT(CpuGpr::R31)))
IMPLEMENT_MODULAR_OP(InstructionSUBP,     (op2 - op3),          model_->GetGpr(TO_INT(CpuGpr::R31)))
IMPLEMENT_MODULAR_OP(InstructionMULP,     (op2 * op3),          model_->GetGpr(TO_INT(CpuGpr::R31)))
IMPLEMENT_MODULAR_OP(InstructionREDP,    ((op2 << 256) | op3),  model_->GetGpr(TO_INT(CpuGpr::R31)))



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
        uint256_t tmp = model_->GetGpr(TO_INT(op2_)) operand uint256_t(immediate_);             \
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

IMPLEMENT_I_32_AIRTH_OP(InstructionADDI,+,true)
IMPLEMENT_I_32_AIRTH_OP(InstructionSUBI,-,true)
IMPLEMENT_I_32_AIRTH_OP(InstructionCMPI,-,false)


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
            binary_logic_op_32_lsb(model_->GetGpr(TO_INT(op2_)),                                \
                                uint256_t(immediate_),                                          \
                                model_->GetGpr(TO_INT(op1_)),                                   \
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

IMPLEMENT_I_32_LOGIC_OP(InstructionANDI,&)
IMPLEMENT_I_32_LOGIC_OP(InstructionORI,|)
IMPLEMENT_I_32_LOGIC_OP(InstructionXORI,^)


bool spect::InstructionCMPA::Execute()
{
    DEFINE_CHANGE(ch_zf, DPI_CHANGE_FLAG, DPI_SPECT_FLAG_ZERO);
    PUT_FLAG_TO_CHANGE(ch_zf, old_val, model_->GetCpuFlag(CpuFlagType::ZERO));

    model_->SetCpuFlag(CpuFlagType::ZERO, model_->GetGpr(TO_INT(op2_)) == uint256_t(immediate_));

    PUT_FLAG_TO_CHANGE(ch_zf, new_val, model_->GetCpuFlag(CpuFlagType::ZERO));
    model_->ReportChange(ch_zf);

    return true;
}

bool spect::InstructionMOVI::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    model_->SetGpr(TO_INT(op1_), uint256_t(immediate_));

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

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
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    int index = immediate_ & 0x7;
    uint256_t tmp = 0;
    for (int i = 0; i < 8; i++) {
        uint256_t part = model_->GpkQueuePop(index);
        part = part << (32 * i);
        tmp = tmp | part;
    }
    model_->SetGpr(TO_INT(op1_), tmp);

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// M Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

bool spect::InstructionLD::Execute()
{
    DEFINE_CHANGE(ch_gpr, DPI_CHANGE_GPR, TO_INT(op1_));
    PUT_GPR_TO_CHANGE(ch_gpr, old_val, model_->GetGpr(TO_INT(op1_)));

    std::stringstream str;
    str << "0x" << std::hex << std::setfill('0') << std::setw(32);
    for (int i = 7; i >=0; i--)
        str << model_->ReadMemoryCoreData(addr_ + i);
    model_->SetGpr(TO_INT(op1_), uint256_t(str.str().c_str()));

    PUT_GPR_TO_CHANGE(ch_gpr, new_val, model_->GetGpr(TO_INT(op1_)));
    model_->ReportChange(ch_gpr);

    return true;
}

bool spect::InstructionST::Execute()
{
    uint256_t tmp =  model_->GetGpr(TO_INT(op1_));
    for (int i = 0; i < 8; i++) {
        model_->WriteMemoryCoreData(addr_ + i,
                    static_cast<uint32_t>(tmp & uint256_t("0xFFFFFFFF")));
        tmp = tmp >> 32;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// J Instructions
///////////////////////////////////////////////////////////////////////////////////////////////////

bool spect::InstructionCALL::Execute()
{
    DEFINE_CHANGE(ch_rar, DPI_CHANGE_RAR, DPI_RAR_PUSH);

    uint16_t ret_addr = model_->GetPc() + 0x4;
    model_->RarPush(ret_addr);
    model_->SetPc(new_pc_);

    ch_rar.new_val[0] = ret_addr;
    model_->ReportChange(ch_rar);

    return false;
}

bool spect::InstructionRET::Execute()
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

IMPLEMENT_COND_JUMP_OP(InstructionBRZ,zero,true)
IMPLEMENT_COND_JUMP_OP(InstructionBRNZ,zero,false)
IMPLEMENT_COND_JUMP_OP(InstructionBRC,carry,true)
IMPLEMENT_COND_JUMP_OP(InstructionBRNC,carry,false)

bool spect::InstructionJMP::Execute()
{
    model_->SetPc(new_pc_);
    return false;
}

bool spect::InstructionEND::Execute()
{
    model_->SetSrr(model_->GetGpr(31));
    model_->Finish(0);
    model_->UpdateInterrupts();

    return false;
}

bool spect::InstructionNOP::Execute()
{
    return true;
}