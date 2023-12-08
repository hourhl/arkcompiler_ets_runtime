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
    FP = X8,
    INVALID_REG = 0xFF,
};

enum RegisterType {
    W = 0,  /* a word for 32 bits */
    D = 1,  /* a double-word for 64 bits */
};

static const int RegDSize = 64;
static const int RegWSize = 32;

enum AddSubOpFunct {
    Add     = 0x00000033,
    Addw    = 0x0000003b,
    Sub     = 0x40000033,
    Subw    = 0x4000003b,
    Slt     = 0x00002033,
    Sltu    = 0x00003033,
};

enum ShiftOpFunct {
    Sll     = 0x00001033,
    Sllw    = 0x0000103b,
    Srl     = 0x00005033,
    Srlw    = 0x0000503b,
    Sra     = 0x40005033,
    Sraw    = 0x40005033,
};

enum BitwiseOpFunct {
    Xor = 0x00004033,
    Or  = 0x00006033,
    And = 0x00007033,
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

#define EMIT_R_TYPE_INSTS(V)    \
    V(Add )                     \
    V(Addw)                     \
    V(Sub )                     \
    V(Subw)                     \
    V(Slt )                     \
    V(Sltu)                     \
    V(Sll )                     \
    V(Sllw)                     \
    V(Srl )                     \
    V(Srlw)                     \
    V(Sra )                     \
    V(Sraw)                     \
    V(Xor )                     \
    V(Or  )                     \
    V(And )                     \

#define EMIT_R_TYPE_INST(INSTNAME) \
void AssemblerRiscv64::##INSTNAME(const Register &rd, const Register &rs1, const Register &rs2) \
{ \
    uint32_t rd = Rd(rd.GetId()); \
    uint32_t rs1 = Rs1(rs1.GetId()); \
    uint32_t rs2 = Rs2(rs2.GetId()); \
    uint32_t code = rd | rs1 | rs2 | opc | INSTNAME; \
    Emit(code); \
}

};  // namespace panda::ecmascript::riscv64
#endif
