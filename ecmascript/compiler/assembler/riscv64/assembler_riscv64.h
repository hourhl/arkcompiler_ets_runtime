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

#ifndef ECMASCRIPT_COMPILER_ASSEMBLER_RISCV64_H
#define ECMASCRIPT_COMPILER_ASSEMBLER_RISCV64_H

#include "ecmascript/compiler/assembler/assembler.h"
#include "ecmascript/compiler/assembler/riscv64/assembler_riscv64_constants.h"

namespace panda::ecmascript::riscv64 {
class Register {
public:
    Register(RegisterId reg) : reg_(reg) {};

    inline RegisterId GetId() const
    {
        return reg_;
    }
    inline bool IsValid() const
    {
        return reg_ != RegisterId::INVALID_REG;
    }
    inline bool operator !=(const Register &other)
    {
        return reg_ != other.GetId();
    }
    inline bool operator ==(const Register &other)
    {
        return reg_ == other.GetId();
    }
private:
    RegisterId reg_;
};

class Immediate {
public:
    Immediate(int32_t value) : value_(value) {}
    ~Immediate() = default;

    int32_t Value() const
    {
        return value_;
    }
private:
    int32_t value_;
};

class AssemblerRiscv64 : public Assembler {
public:
    explicit AssemblerRiscv64(Chunk *chunk)
        : Assembler(chunk)
    {
    }
    // Integer Register-Register Operations (R-types)
    void Add(const Register &rd, const Register &rs1, const Register &rs2);
    void Addw(const Register &rd, const Register &rs1, const Register &rs2);
    void Sub(const Register &rd, const Register &rs1, const Register &rs2);
    void Subw(const Register &rd, const Register &rs1, const Register &rs2);
    void Slt(const Register &rd, const Register &rs1, const Register &rs2);
    void Sltu(const Register &rd, const Register &rs1, const Register &rs2);
    void Sll(const Register &rd, const Register &rs1, const Register &rs2);
    void Sllw(const Register &rd, const Register &rs1, const Register &rs2);
    void Srl(const Register &rd, const Register &rs1, const Register &rs2);
    void Srlw(const Register &rd, const Register &rs1, const Register &rs2);
    void Sra(const Register &rd, const Register &rs1, const Register &rs2);
    void Sraw(const Register &rd, const Register &rs1, const Register &rs2);
    void Xor(const Register &rd, const Register &rs1, const Register &rs2);
    void Or(const Register &rd, const Register &rs1, const Register &rs2);
    void And(const Register &rd, const Register &rs1, const Register &rs2);
    
    // Integer Register-Register Operations (B-types)
    void Beq(const Register &rs1, const Register &rs2,const Immediate& imm12);
    void Bne(const Register &rs1, const Register &rs2,const Immediate& imm12);
    void Blt(const Register &rs1, const Register &rs2,const Immediate& imm12);
private:
    // R_TYPE field defines
    inline uint32_t Rd(uint32_t id){
        return (id << R_TYPE_rd_LOWBITS) & R_TYPE_rd_MASK;
    }
    inline uint32_t Rs1(uint32_t id){
        return (id << R_TYPE_rs1_LOWBITS) & R_TYPE_rs1_MASK;
    }
    inline uint32_t Rs2(uint32_t id){
        return (id << R_TYPE_rs2_LOWBITS) & R_TYPE_rs2_MASK;
    }
    
    // B_TYPE field defines
    /*inline uint32_t BImm1(uint32_t imm1){
        return (imm1 << B_TYPE_imm1_LOWBITS) & B_TYPE_imm1_MASK;
    }
    inline uint32_t BImm2(uint32_t imm2){
        return (imm2 << B_TYPE_imm2_LOWBITS) & B_TYPE_imm2_MASK;
    }

    inline uint32_t BRs1(uint32_t id){
        return (id << B_TYPE_rs1_LOWBITS) & B_TYPE_rs1_MASK;
    }
    inline uint32_t BRs2(uint32_t id){
        return (id << B_TYPE_rs2_LOWBITS) & B_TYPE_rs2_MASK;
   }

    inline uint32_t Bfunct3(uint32_t funct3){
        return (funct3 << B_TYPE_funct3_LOWBITS) & B_TYPE_funct3_MASK;
    }*/
};
} // namespace panda::ecmascript::riscv64
#endif
