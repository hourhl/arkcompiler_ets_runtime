/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ecmascript/compiler/assembler/riscv64/assembler_riscv64.h"

#include <ostream>
#include <sstream>

#include "ecmascript/compiler/assembler/assembler.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/mem/dyn_chunk.h"
#include "ecmascript/tests/test_helper.h"

#include "llvm-c/Analysis.h"
#include "llvm-c/Core.h"
#include "llvm-c/Disassembler.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm-c/Target.h"

namespace panda::test {
using namespace panda::ecmascript;
using namespace panda::ecmascript::riscv64;
class AssemblerRiscv64Test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        InitializeLLVM(TARGET_RISCV64);
        TestHelper::CreateEcmaVMWithScope(instance, thread, scope);
        chunk_ = thread->GetEcmaVM()->GetChunk();
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    static const char *SymbolLookupCallback([[maybe_unused]] void *disInfo, [[maybe_unused]] uint64_t referenceValue,
                                            uint64_t *referenceType, [[maybe_unused]] uint64_t referencePC,
                                            [[maybe_unused]] const char **referenceName)
    {
        *referenceType = LLVMDisassembler_ReferenceType_InOut_None;
        return nullptr;
    }

    void InitializeLLVM(std::string triple)
    {
        if (triple.compare(TARGET_RISCV64) == 0) {
            LLVMInitializeRISCVTargetInfo();
            LLVMInitializeRISCVTargetMC();
            LLVMInitializeRISCVDisassembler();
            LLVMInitializeRISCVAsmPrinter();
            LLVMInitializeRISCVAsmParser();
            LLVMInitializeRISCVTarget();
        } else {
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
        }
    }

    void DisassembleChunk(const char *triple, Assembler *assemlber, std::ostream &os)
    {
        LLVMDisasmContextRef dcr = LLVMCreateDisasm(triple, nullptr, 0, nullptr, SymbolLookupCallback);
        uint8_t *byteSp = assemlber->GetBegin();
        size_t numBytes = assemlber->GetCurrentPosition();
        unsigned pc = 0;
        const char outStringSize = 100;
        char outString[outStringSize];
        while (numBytes > 0) {
            size_t InstSize = LLVMDisasmInstruction(dcr, byteSp, numBytes, pc, outString, outStringSize);
            if (InstSize == 0) {
                // 8 : 8 means width of the pc offset and instruction code
                os << std::setw(8) << std::setfill('0') << std::hex << pc << ":" << std::setw(8)
                   << *reinterpret_cast<uint32_t *>(byteSp) << "maybe constant" << std::endl;
                pc += 4; // 4 pc length
                byteSp += 4; // 4 sp offset
                numBytes -= 4; // 4 num bytes
            }
            // 8 : 8 means width of the pc offset and instruction code
            os << std::setw(8) << std::setfill('0') << std::hex << pc << ":" << std::setw(8)
               << *reinterpret_cast<uint32_t *>(byteSp) << " " << outString << std::endl;
            pc += InstSize;
            byteSp += InstSize;
            numBytes -= InstSize;
        }
        LLVMDisasmDispose(dcr);
    }
    EcmaVM *instance {nullptr};
    JSThread *thread {nullptr};
    EcmaHandleScope *scope {nullptr};
    Chunk *chunk_ {nullptr};
};

#define __ masm.
HWTEST_F_L0(AssemblerRiscv64Test, AddSub)
{
    std::string expectResult(
            "00000000:002080b3 \tadd\tra, ra, sp\n"
            "00000004:003100b3 \tadd\tra, sp, gp\n"
            "00000008:41f887b3 \tsub\ta5, a7, t6\n"
            "0000000c:416409b3 \tsub\ts3, s0, s6\n" 
            "00000010:01f8a7b3 \tslt\ta5, a7, t6\n"
            "00000014:016429b3 \tslt\ts3, s0, s6\n"
            "00000018:01f8b7b3 \tsltu\ta5, a7, t6\n"
            "0000001c:016439b3 \tsltu\ts3, s0, s6\n"
            );
    AssemblerRiscv64 masm(chunk_);
    __ Add(Register(RA), Register(RA), Register(SP));
    __ Add(Register(RA), Register(SP), Register(GP));
    __ Sub(Register(A5), Register(A7), Register(T6));
    __ Sub(Register(S3), Register(FP), Register(S6));
    __ Slt(Register(A5), Register(A7), Register(T6));
    __ Slt(Register(S3), Register(FP), Register(S6));
    __ Sltu(Register(A5), Register(A7), Register(T6));
    __ Sltu(Register(S3), Register(FP), Register(S6));
    std::ostringstream oss;
    DisassembleChunk(TARGET_RISCV64, &masm, oss);
    ASSERT_EQ(oss.str(), expectResult);
}


HWTEST_F_L0(AssemblerRiscv64Test, Shift)
{
    std::string expectResult(
            "00000000:01f897b3 \tsll\ta5, a7, t6\n"
            "00000004:016419b3 \tsll\ts3, s0, s6\n"
            "00000008:01f8d7b3 \tsrl\ta5, a7, t6\n"
            "0000000c:016459b3 \tsrl\ts3, s0, s6\n"
            "00000010:41f8d7b3 \tsra\ta5, a7, t6\n"
            "00000014:416459b3 \tsra\ts3, s0, s6\n");
    AssemblerRiscv64 masm(chunk_);
    __ Sll(Register(A5), Register(A7), Register(T6));
    __ Sll(Register(S3), Register(FP), Register(S6));
    __ Srl(Register(A5), Register(A7), Register(T6));
    __ Srl(Register(S3), Register(FP), Register(S6));
    __ Sra(Register(A5), Register(A7), Register(T6));
    __ Sra(Register(S3), Register(FP), Register(S6));
    std::ostringstream oss;
    DisassembleChunk(TARGET_RISCV64, &masm, oss);
    ASSERT_EQ(oss.str(), expectResult);
}


HWTEST_F_L0(AssemblerRiscv64Test, BitwiseOp)
{
    std::string expectResult(
            "00000000:01f8c7b3 \txor\ta5, a7, t6\n"
            "00000004:016449b3 \txor\ts3, s0, s6\n"
            "00000008:01f8e7b3 \tor\ta5, a7, t6\n"
            "0000000c:016469b3 \tor\ts3, s0, s6\n"
            "00000010:01f8f7b3 \tand\ta5, a7, t6\n"
            "00000014:016479b3 \tand\ts3, s0, s6\n");
    AssemblerRiscv64 masm(chunk_);
    __ Xor(Register(A5), Register(A7), Register(T6));
    __ Xor(Register(S3), Register(FP), Register(S6));
    __ Or(Register(A5), Register(A7), Register(T6));
    __ Or(Register(S3), Register(FP), Register(S6));
    __ And(Register(A5), Register(A7), Register(T6));
    __ And(Register(S3), Register(FP), Register(S6));
    std::ostringstream oss;
    DisassembleChunk(TARGET_RISCV64, &masm, oss);
    ASSERT_EQ(oss.str(), expectResult);
}

#undef __
}  // namespace panda::test
