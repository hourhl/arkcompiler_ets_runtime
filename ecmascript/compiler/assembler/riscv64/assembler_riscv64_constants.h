/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ECMASCRIPT_COMPILER_ASSEMBLER_RISCV64_CONSTANTS_H
#define ECMASCRIPT_COMPILER_ASSEMBLER_RISCV64_CONSTANTS_H
namespace panda::ecmascript::riscv64 {
enum RegisterId : uint8_t {
    X0, X1, SP, X3, X4, X5, X6, X7,
    X8, X9, X10, X11, X12, X13, X14, X15,
    X16, X17, X18, X19, X20, X21, X22, X23,
    X24, X25, X26, X27, X28, X29, X30, X31,
    ZERO = X0,
    RA = X1,
    GP = X3,
    TP = X4,
    T0 = X5,
    T1 = X6,
    T2 = X7,
    S0 = X8,
    FP = X8,
    S1 = X9,
    A0 = X10,
    A1 = X11,
    A2 = X12,
    A3 = X13,
    A4 = X14,
    A5 = X15,
    A6 = X16,
    A7 = X17,
    S2 = X18,
    S3 = X19,
    S4 = X20,
    S5 = X21,
    S6 = X22,
    S7 = X23,
    S8 = X24,
    S9 = X25,
    S10 = X26,
    S11 = X27,
    T3 = X28,
    T4 = X29,
    T5 = X30,
    T6 = X31,
    INVALID_REG = 0xFF,
};

enum RegisterType {
    W = 0,  /* a word for 32 bits */
    D = 1,  /* a double-word for 64 bits */
};

static const int RegDSize = 64;
static const int RegWSize = 32;

enum AddSubOpFunct {
    ADD     = 0x00000033,
    ADDW    = 0x0000003b,
    SUB     = 0x40000033,
    SUBW    = 0x4000003b,
    SLT     = 0x00002033,
    SLTU    = 0x00003033,
};

enum ShiftOpFunct {
    SLL     = 0x00001033,
    SLLW    = 0x0000103b,
    SRL     = 0x00005033,
    SRLW    = 0x0000503b,
    SRA     = 0x40005033,
    SRAW    = 0x4000503b,
};

enum BitwiseOpFunct {
    XOR = 0x00004033,
    OR  = 0x00006033,
    AND = 0x00007033,
};

#define R_TYPE_FIELD_LIST(V)    \
    V(R_TYPE, opcode,  6,  0)   \
    V(R_TYPE,     rd, 11,  7)   \
    V(R_TYPE, funct3, 14, 12)   \
    V(R_TYPE,    rs1, 19, 15)   \
    V(R_TYPE,    rs2, 24, 20)   \
    V(R_TYPE, funct7, 31, 25)

#define DECL_FIELDS_IN_INSTRUCTION(INSTNAME, FIELD_NAME, HIGHBITS, LOWBITS) \
static const uint32_t INSTNAME##_##FIELD_NAME##_HIGHBITS = HIGHBITS;  \
static const uint32_t INSTNAME##_##FIELD_NAME##_LOWBITS = LOWBITS;    \
static const uint32_t INSTNAME##_##FIELD_NAME##_WIDTH = ((HIGHBITS - LOWBITS) + 1); \
static const uint32_t INSTNAME##_##FIELD_NAME##_MASK = (((1 << INSTNAME##_##FIELD_NAME##_WIDTH) - 1) << LOWBITS);

#define DECL_INSTRUCTION_FIELDS(V)  \
    R_TYPE_FIELD_LIST(V)

DECL_INSTRUCTION_FIELDS(DECL_FIELDS_IN_INSTRUCTION)
#undef DECL_INSTRUCTION_FIELDS

#define EMIT_INSTS \
    EMIT_R_TYPE_INSTS(EMIT_R_TYPE_INST) \

#define EMIT_R_TYPE_INSTS(V) \
    V( Add,  ADD)            \
    V(Addw, ADDW)            \
    V( Sub,  SUB)            \
    V(Subw, SUBW)            \
    V( Slt,  SLT)            \
    V(Sltu, SLTU)            \
    V( Sll,  SLL)            \
    V(Sllw, SLLW)            \
    V( Srl,  SRL)            \
    V(Srlw, SRLW)            \
    V( Sra,  SRA)            \
    V(Sraw, SRAW)            \
    V( Xor,  XOR)            \
    V(  Or,   OR)            \
    V( And,  AND)            \

#define EMIT_R_TYPE_INST(INSTNAME, INSTID) \
void AssemblerRiscv64::INSTNAME(const Register &rd, const Register &rs1, const Register &rs2) \
{ \
    uint32_t rd_id = Rd(rd.GetId()); \
    uint32_t rs1_id = Rs1(rs1.GetId()); \
    uint32_t rs2_id = Rs2(rs2.GetId()); \
    uint32_t code = rd_id | rs1_id | rs2_id | INSTID; \
    EmitU32(code); \
}

};  // namespace panda::ecmascript::riscv64
#endif
