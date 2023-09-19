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

#ifndef ECMASCRIPT_COMPILER_CIRCUIT_BUILDER_H
#define ECMASCRIPT_COMPILER_CIRCUIT_BUILDER_H

#include "ecmascript/base/number_helper.h"
#include "ecmascript/compiler/assembler/assembler.h"
#include "ecmascript/compiler/builtins/builtins_call_signature.h"
#include "ecmascript/compiler/circuit.h"
#include "ecmascript/compiler/call_signature.h"
#include "ecmascript/compiler/gate.h"
#include "ecmascript/compiler/gate_accessor.h"
#include "ecmascript/compiler/variable_type.h"
#include "ecmascript/global_env_constants.h"
#include "ecmascript/jspandafile/constpool_value.h"
#include "ecmascript/js_hclass.h"
#include "ecmascript/js_runtime_options.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/tagged_array.h"

namespace panda::ecmascript::kungfu {
using namespace panda::ecmascript;
#define DEFVAlUE(varname, cirBuilder, type, val) \
        Variable varname(cirBuilder, type, cirBuilder->NextVariableId(), val)

class Environment;
class Label;
class Variable;
class StubBuilder;
class TSHCRLowering;
class NTypeHCRLowering;
class SlowPathLowering;

#define BINARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH(V)                    \
    V(Int16Add, Add, MachineType::I16)                                    \
    V(Int32Add, Add, MachineType::I32)                                    \
    V(Int64Add, Add, MachineType::I64)                                    \
    V(DoubleAdd, Add, MachineType::F64)                                   \
    V(PtrAdd, Add, MachineType::ARCH)                                     \
    V(Int16Sub, Sub, MachineType::I16)                                    \
    V(Int32Sub, Sub, MachineType::I32)                                    \
    V(Int64Sub, Sub, MachineType::I64)                                    \
    V(DoubleSub, Sub, MachineType::F64)                                   \
    V(PtrSub, Sub, MachineType::ARCH)                                     \
    V(Int32Mul, Mul, MachineType::I32)                                    \
    V(Int64Mul, Mul, MachineType::I64)                                    \
    V(DoubleMul, Mul, MachineType::F64)                                   \
    V(PtrMul, Mul, MachineType::ARCH)                                     \
    V(Int32Div, Sdiv, MachineType::I32)                                   \
    V(Int64Div, Sdiv, MachineType::I64)                                   \
    V(DoubleDiv, Fdiv, MachineType::F64)                                  \
    V(Int32Mod, Smod, MachineType::I32)                                   \
    V(DoubleMod, Smod, MachineType::F64)                                  \
    V(BoolAnd, And, MachineType::I1)                                      \
    V(Int8And, And, MachineType::I8)                                      \
    V(Int32And, And, MachineType::I32)                                    \
    V(Int64And, And, MachineType::I64)                                    \
    V(BoolOr, Or, MachineType::I1)                                        \
    V(Int32Or, Or, MachineType::I32)                                      \
    V(Int64Or, Or, MachineType::I64)                                      \
    V(Int32Xor, Xor, MachineType::I32)                                    \
    V(Int64Xor, Xor, MachineType::I64)                                    \
    V(Int16LSL, Lsl, MachineType::I16)                                    \
    V(Int32LSL, Lsl, MachineType::I32)                                    \
    V(Int64LSL, Lsl, MachineType::I64)                                    \
    V(Int8LSR, Lsr, MachineType::I8)                                      \
    V(Int32LSR, Lsr, MachineType::I32)                                    \
    V(Int64LSR, Lsr, MachineType::I64)                                    \
    V(Int32ASR, Asr, MachineType::I32)

#define UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH(V)                  \
    V(BoolNot, Rev, MachineType::I1)                                   \
    V(Int32Not, Rev, MachineType::I32)                                 \
    V(Int64Not, Rev, MachineType::I64)                                 \
    V(CastDoubleToInt64, Bitcast, MachineType::I64)                    \
    V(CastInt64ToFloat64, Bitcast, MachineType::F64)                   \
    V(CastInt32ToFloat32, Bitcast, MachineType::F32)                   \
    V(SExtInt32ToInt64, Sext, MachineType::I64)                        \
    V(SExtInt1ToInt64, Sext, MachineType::I64)                         \
    V(SExtInt1ToInt32, Sext, MachineType::I32)                         \
    V(ZExtInt8ToInt16, Zext, MachineType::I16)                         \
    V(ZExtInt32ToInt64, Zext, MachineType::I64)                        \
    V(ZExtInt1ToInt64, Zext, MachineType::I64)                         \
    V(ZExtInt1ToInt32, Zext, MachineType::I32)                         \
    V(ZExtInt8ToInt32, Zext, MachineType::I32)                         \
    V(ZExtInt8ToInt64, Zext, MachineType::I64)                         \
    V(ZExtInt8ToPtr, Zext, MachineType::ARCH)                          \
    V(ZExtInt16ToPtr, Zext, MachineType::ARCH)                         \
    V(ZExtInt32ToPtr, Zext, MachineType::ARCH)                         \
    V(SExtInt32ToPtr, Sext, MachineType::ARCH)                         \
    V(ZExtInt16ToInt32, Zext, MachineType::I32)                        \
    V(ZExtInt16ToInt64, Zext, MachineType::I64)                        \
    V(TruncInt16ToInt8, Trunc, MachineType::I8)                        \
    V(TruncInt64ToInt32, Trunc, MachineType::I32)                      \
    V(TruncPtrToInt32, Trunc, MachineType::I32)                        \
    V(TruncInt64ToInt1, Trunc, MachineType::I1)                        \
    V(TruncInt64ToInt16, Trunc, MachineType::I16)                      \
    V(TruncInt32ToInt1, Trunc, MachineType::I1)                        \
    V(TruncInt32ToInt8, Trunc, MachineType::I8)                        \
    V(TruncInt32ToInt16, Trunc, MachineType::I16)                      \
    V(TruncFloatToInt64, TruncFloatToInt64, MachineType::I64)          \
    V(TruncFloatToInt32, TruncFloatToInt32, MachineType::I32)          \
    V(ExtFloat32ToDouble, Fext, MachineType::F64)                      \
    V(TruncDoubleToFloat32, Ftrunc, MachineType::F32)                  \
    V(ChangeInt32ToFloat64, SignedIntToFloat, MachineType::F64)        \
    V(ChangeInt32ToFloat32, SignedIntToFloat, MachineType::F32)        \
    V(ChangeUInt32ToFloat64, UnsignedIntToFloat, MachineType::F64)     \
    V(ChangeFloat64ToInt32, FloatToSignedInt, MachineType::I32)        \
    V(SExtInt16ToInt64, Sext, MachineType::I64)                        \
    V(SExtInt16ToInt32, Sext, MachineType::I32)                        \
    V(SExtInt8ToInt32, Sext, MachineType::I32)                         \
    V(SExtInt8ToInt64, Sext, MachineType::I64)

#define UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH_PRIVATE(V)          \
    V(ChangeTaggedPointerToInt64, TaggedToInt64, MachineType::I64)

#define BINARY_CMP_METHOD_LIST_WITHOUT_BITWIDTH(V)                                      \
    V(DoubleLessThan, Fcmp, static_cast<BitField>(FCmpCondition::OLT))                  \
    V(DoubleLessThanOrEqual, Fcmp, static_cast<BitField>(FCmpCondition::OLE))           \
    V(DoubleGreaterThan, Fcmp, static_cast<BitField>(FCmpCondition::OGT))               \
    V(DoubleGreaterThanOrEqual, Fcmp, static_cast<BitField>(FCmpCondition::OGE))        \
    V(DoubleEqual, Fcmp, static_cast<BitField>(FCmpCondition::OEQ))                     \
    V(DoubleNotEqual, Fcmp, static_cast<BitField>(FCmpCondition::ONE))                  \
    V(Int32LessThan, Icmp, static_cast<BitField>(ICmpCondition::SLT))                   \
    V(Int32LessThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::SLE))            \
    V(Int32GreaterThan, Icmp, static_cast<BitField>(ICmpCondition::SGT))                \
    V(Int32GreaterThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::SGE))         \
    V(Int32UnsignedLessThan, Icmp, static_cast<BitField>(ICmpCondition::ULT))           \
    V(Int32UnsignedLessThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::ULE))    \
    V(Int32UnsignedGreaterThan, Icmp, static_cast<BitField>(ICmpCondition::UGT))        \
    V(Int32UnsignedGreaterThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::UGE)) \
    V(Int64LessThan, Icmp, static_cast<BitField>(ICmpCondition::SLT))                   \
    V(Int64LessThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::SLE))            \
    V(Int64GreaterThan, Icmp, static_cast<BitField>(ICmpCondition::SGT))                \
    V(Int64GreaterThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::SGE))         \
    V(Int64UnsignedLessThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::ULE))

class CompilationConfig {
public:
    explicit CompilationConfig(const std::string &triple, const JSRuntimeOptions *options = nullptr)
        : tripleStr_(triple), triple_(GetTripleFromString(triple))
    {
        if (options != nullptr) {
            isTraceBc_ = options->IsTraceBC();
            profiling_ = options->GetOptCodeProfiler();
            stressDeopt_ = options->GetStressDeopt();
            verifyVTable_ = options->GetVerifyVTable();
        }
    }
    ~CompilationConfig() = default;

    inline bool Is32Bit() const
    {
        return triple_ == Triple::TRIPLE_ARM32;
    }

    inline bool IsAArch64() const
    {
        return triple_ == Triple::TRIPLE_AARCH64;
    }

    inline bool IsAmd64() const
    {
        return triple_ == Triple::TRIPLE_AMD64;
    }

    inline bool IsRiscv64() const
    {
        return triple_ == Triple::TRIPLE_RISCV64;
    }

    inline bool Is64Bit() const
    {
        return IsAArch64() || IsAmd64() || IsRiscv64();
    }

    Triple GetTriple() const
    {
        return triple_;
    }

    std::string GetTripleStr() const
    {
        return tripleStr_;
    }

    bool IsTraceBC() const
    {
        return isTraceBc_;
    }

    bool IsProfiling() const
    {
        return profiling_;
    }

    bool IsStressDeopt() const
    {
        return stressDeopt_;
    }

    bool IsVerifyVTbale() const
    {
        return verifyVTable_;
    }

private:
    inline Triple GetTripleFromString(const std::string &triple)
    {
        if (triple.compare(TARGET_RISCV64) == 0) {
            return Triple::TRIPLE_RISCV64;
        }

        if (triple.compare(TARGET_X64) == 0) {
            return Triple::TRIPLE_AMD64;
        }

        if (triple.compare(TARGET_AARCH64) == 0) {
            return Triple::TRIPLE_AARCH64;
        }

        if (triple.compare(TARGET_ARM32) == 0) {
            return Triple::TRIPLE_ARM32;
        }
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
    std::string tripleStr_;
    Triple triple_;
    bool isTraceBc_ {false};
    bool profiling_ {false};
    bool stressDeopt_ {false};
    bool verifyVTable_ {false};
};

class CircuitBuilder {
public:
    explicit CircuitBuilder(Circuit *circuit) : circuit_(circuit), acc_(circuit) {}
    CircuitBuilder(Circuit *circuit, CompilationConfig *cmpCfg)
        : circuit_(circuit), acc_(circuit), cmpCfg_(cmpCfg)
    {
    }
    ~CircuitBuilder() = default;
    NO_MOVE_SEMANTIC(CircuitBuilder);
    NO_COPY_SEMANTIC(CircuitBuilder);
    static constexpr uint32_t GATE_TWO_VALUESIN = 2;
    // low level interface
    GateRef HeapObjectCheck(GateRef gate, GateRef frameState);
    GateRef StableArrayCheck(GateRef gate, ElementsKind kind, ArrayMetaDataAccessor::Mode mode);
    GateRef COWArrayCheck(GateRef gate);
    GateRef EcmaStringCheck(GateRef gate);
    GateRef FlattenStringCheck(GateRef gate);
    GateRef HClassStableArrayCheck(GateRef gate, GateRef frameState, ArrayMetaDataAccessor accessor);
    GateRef ArrayGuardianCheck(GateRef frameState);
    GateRef TypedArrayCheck(GateType type, GateRef gate);
    GateRef LoadTypedArrayLength(GateType type, GateRef gate);
    GateRef RangeGuard(GateRef gate, uint32_t left, uint32_t right);
    GateRef IndexCheck(GateType type, GateRef gate, GateRef index);
    GateRef ObjectTypeCheck(GateType type, bool isHeapObject, GateRef gate, GateRef hclassIndex);
    GateRef ObjectTypeCompare(GateType type, bool isHeapObject, GateRef gate, GateRef hclassIndex);
    GateRef TryPrimitiveTypeCheck(GateType type, GateRef gate);
    GateRef CallTargetCheck(GateRef gate, GateRef function, GateRef id, GateRef param, const char* comment = nullptr);
    GateRef JSCallTargetFromDefineFuncCheck(GateType type, GateRef func, GateRef gate);
    template<TypedCallTargetCheckOp Op>
    GateRef JSCallTargetTypeCheck(GateType type, GateRef func, GateRef methodIndex, GateRef gate);
    template<TypedCallTargetCheckOp Op>
    GateRef JSCallThisTargetTypeCheck(GateType type, GateRef func, GateRef gate);
    template<TypedCallTargetCheckOp Op>
    inline GateRef JSNoGCCallThisTargetTypeCheck(GateType type, GateRef func, GateRef methodId, GateRef gate);
    GateRef DeoptCheck(GateRef condition, GateRef frameState, DeoptType type);
    GateRef TypedCallOperator(GateRef hirGate, MachineType type, const std::vector<GateRef>& inList);
    inline GateRef TypedCallBuiltin(GateRef hirGate, const std::vector<GateRef> &args, BuiltinsStubCSigns::ID id);
    GateRef TypeConvert(MachineType type, GateType typeFrom, GateType typeTo, const std::vector<GateRef>& inList);
    GateRef AddWithOverflow(GateRef left, GateRef right);
    GateRef SubWithOverflow(GateRef left, GateRef right);
    GateRef MulWithOverflow(GateRef left, GateRef right);
    GateRef ExtractValue(MachineType mt, GateRef pointer, GateRef index);
    GateRef Sqrt(GateRef param);
    GateRef Int32CheckRightIsZero(GateRef right);
    GateRef Float64CheckRightIsZero(GateRef right);
    GateRef ValueCheckNegOverflow(GateRef value);
    GateRef OverflowCheck(GateRef value);
    GateRef LexVarIsHoleCheck(GateRef value);
    GateRef Int32UnsignedUpperBoundCheck(GateRef value, GateRef upperBound);
    GateRef Int32DivWithCheck(GateRef left, GateRef right);
    MachineType GetMachineTypeOfValueType(ValueType type);
    GateType GetGateTypeOfValueType(ValueType type);
    GateRef IsJsCOWArray(GateRef obj);
    GateRef IsCOWArray(GateRef objectType);
    GateRef GetElementsArray(GateRef object);
    GateRef Convert(GateRef gate, ValueType src, ValueType dst);
    GateRef ConvertBoolToTaggedBoolean(GateRef gate);
    GateRef ConvertTaggedBooleanToBool(GateRef gate);
    GateRef ConvertInt32ToBool(GateRef gate);
    GateRef ConvertFloat64ToBool(GateRef gate);
    GateRef ConvertInt32ToTaggedInt(GateRef gate);
    GateRef ConvertFloat64ToTaggedDouble(GateRef gate);
    GateRef ConvertFloat64ToInt32(GateRef gate);
    GateRef ConvertInt32ToFloat64(GateRef gate);
    GateRef ConvertBoolToInt32(GateRef gate, ConvertSupport support);
    GateRef ConvertBoolToFloat64(GateRef gate, ConvertSupport support);
    GateRef ConvertUInt32ToBool(GateRef gate);
    GateRef ConvertUInt32ToTaggedNumber(GateRef gate);
    GateRef ConvertUInt32ToFloat64(GateRef gate);
    GateRef CheckAndConvert(
        GateRef gate, ValueType src, ValueType dst, ConvertSupport support = ConvertSupport::ENABLE);
    GateRef ConvertHoleAsUndefined(GateRef receiver);
    GateRef CheckUInt32AndConvertToInt32(GateRef gate);
    GateRef CheckTaggedIntAndConvertToInt32(GateRef gate);
    GateRef CheckTaggedDoubleAndConvertToInt32(GateRef gate);
    GateRef CheckTaggedNumberAndConvertToInt32(GateRef gate);
    GateRef CheckTaggedIntAndConvertToFloat64(GateRef gate);
    GateRef CheckTaggedDoubleAndConvertToFloat64(GateRef gate);
    GateRef CheckTaggedNumberAndConvertToFloat64(GateRef gate);
    GateRef CheckTaggedNumberAndConvertToBool(GateRef gate);
    GateRef CheckTaggedBooleanAndConvertToBool(GateRef gate);
    GateRef TypedConditionJump(MachineType type, TypedJumpOp jumpOp, BranchKind branchKind, GateType typeVal,
                               const std::vector<GateRef>& inList);
    GateRef TypedNewAllocateThis(GateRef ctor, GateRef hclassIndex, GateRef frameState);
    GateRef TypedSuperAllocateThis(GateRef superCtor, GateRef newTarget, GateRef frameState);
    GateRef GetSuperConstructor(GateRef ctor);
    GateRef Arguments(size_t index);
    GateRef Merge(const std::vector<GateRef> &inList);
    GateRef Selector(OpCode opcode, MachineType machineType, GateRef control, const std::vector<GateRef> &values,
        int valueCounts, VariableType type = VariableType::VOID());
    GateRef Selector(OpCode opcode, GateRef control, const std::vector<GateRef> &values,
        int valueCounts, VariableType type = VariableType::VOID());
    GateRef Int8(int8_t val);
    GateRef Int16(int16_t val);
    GateRef Int32(int32_t value);
    GateRef Int64(int64_t value);
    GateRef IntPtr(int64_t val);
    GateRef StringPtr(const std::string &str);
    GateRef Boolean(bool value);
    GateRef Double(double value);
    GateRef UndefineConstant();
    GateRef HoleConstant();
    GateRef NullPtrConstant();
    GateRef NullConstant();
    GateRef ExceptionConstant();
    GateRef RelocatableData(uint64_t val);
    GateRef Alloca(size_t size);
    GateRef Branch(GateRef state, GateRef condition,
        uint32_t leftWeight = 1, uint32_t rightWeight = 1);  // 1: default branch weight
    GateRef SwitchBranch(GateRef state, GateRef index, int caseCounts);
    GateRef Return(GateRef state, GateRef depend, GateRef value);
    GateRef ReturnVoid(GateRef state, GateRef depend);
    GateRef Goto(GateRef state);
    GateRef LoopBegin(GateRef state);
    GateRef LoopEnd(GateRef state);
    GateRef IfTrue(GateRef ifBranch);
    GateRef IfFalse(GateRef ifBranch);
    GateRef SwitchCase(GateRef switchBranch, int64_t value);
    GateRef DefaultCase(GateRef switchBranch);
    GateRef DependRelay(GateRef state, GateRef depend);
    GateRef ReadSp();
    GateRef BinaryArithmetic(const GateMetaData* meta, MachineType machineType,
        GateRef left, GateRef right, GateType gateType = GateType::Empty());
    GateRef BinaryCmp(const GateMetaData* meta, GateRef left, GateRef right);
    static MachineType GetMachineTypeFromVariableType(VariableType type);
    GateRef GetCallBuiltinId(GateRef method);
    Circuit *GetCircuit() const
    {
        return circuit_;
    }
    // constant
    inline GateRef True();
    inline GateRef False();
    inline GateRef Undefined();
    inline GateRef Hole();

    // call operation
    GateRef CallBCHandler(GateRef glue, GateRef target, const std::vector<GateRef> &args,
                          const char* comment = nullptr);
    GateRef CallBCDebugger(GateRef glue, GateRef target, const std::vector<GateRef> &args,
                           const char* comment = nullptr);
    GateRef CallBuiltin(GateRef glue, GateRef target, const std::vector<GateRef> &args,
                        const char* comment = nullptr);
    GateRef CallBuiltinWithArgv(GateRef glue, GateRef target, const std::vector<GateRef> &args,
                                const char* comment = nullptr);
    GateRef CallRuntimeVarargs(GateRef glue, int index, GateRef argc, GateRef argv, const char* comment = nullptr);
    GateRef CallRuntime(GateRef glue, int index, GateRef depend, const std::vector<GateRef> &args, GateRef hirGate,
                        const char* comment = nullptr);
    GateRef CallNGCRuntime(GateRef glue, GateRef gate, int index, const std::vector<GateRef> &args, bool useLabel);

    GateRef CallNGCRuntime(GateRef glue, int index, GateRef depend, const std::vector<GateRef> &args,
                           GateRef hirGate, const char* comment = nullptr);
    GateRef FastCallOptimized(GateRef glue, GateRef code, GateRef depend, const std::vector<GateRef> &args,
                              GateRef hirGate);
    GateRef CallOptimized(GateRef glue, GateRef code, GateRef depend, const std::vector<GateRef> &args,
                          GateRef hirGate);

    GateRef CallStub(GateRef glue, GateRef hirGate, int index, const std::vector<GateRef> &args,
                     const char* comment = nullptr);
    GateRef CallBuiltinRuntime(GateRef glue, GateRef depend, const std::vector<GateRef> &args,
                               bool isNew = false, const char* comment = nullptr);
    GateRef Call(const CallSignature* cs, GateRef glue, GateRef target, GateRef depend,
                 const std::vector<GateRef> &args, GateRef hirGate, const char* comment = nullptr);
    GateRef NoLabelCallRuntime(GateRef glue, GateRef depend, size_t index, std::vector<GateRef> &args, GateRef hirGate);

    void StartCallTimer(GateRef glue, GateRef gate, const std::vector<GateRef> &args, bool useLabel);
    void EndCallTimer(GateRef glue, GateRef gate, const std::vector<GateRef> &args, bool useLabel);

    // memory
    inline GateRef Load(VariableType type, GateRef base, GateRef offset);
    inline GateRef Load(VariableType type, GateRef base, GateRef offset, GateRef depend);
    void Store(VariableType type, GateRef glue, GateRef base, GateRef offset, GateRef value);
    void StoreWithNoBarrier(VariableType type, GateRef base, GateRef offset, GateRef value);

#define ARITHMETIC_BINARY_OP_WITH_BITWIDTH(NAME, OPCODEID, MACHINETYPEID)                 \
    inline GateRef NAME(GateRef x, GateRef y, GateType type = GateType::Empty())          \
    {                                                                                     \
        return BinaryArithmetic(circuit_->OPCODEID(), MACHINETYPEID, x, y, type);         \
    }

    BINARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH(ARITHMETIC_BINARY_OP_WITH_BITWIDTH)
#undef ARITHMETIC_BINARY_OP_WITH_BITWIDTH

#define ARITHMETIC_UNARY_OP_WITH_BITWIDTH(NAME, OPCODEID, MACHINETYPEID)                            \
    inline GateRef NAME(GateRef x)                                                                  \
    {                                                                                               \
        return circuit_->NewGate(circuit_->OPCODEID(), MACHINETYPEID, { x }, GateType::NJSValue()); \
    }

    UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH(ARITHMETIC_UNARY_OP_WITH_BITWIDTH)
#undef ARITHMETIC_UNARY_OP_WITH_BITWIDTH

#define CMP_BINARY_OP_WITHOUT_BITWIDTH(NAME, OPCODEID, CONDITION)                         \
    inline GateRef NAME(GateRef x, GateRef y)                                             \
    {                                                                                     \
        return BinaryCmp(circuit_->OPCODEID(static_cast<uint64_t>(CONDITION)), x, y);     \
    }

    BINARY_CMP_METHOD_LIST_WITHOUT_BITWIDTH(CMP_BINARY_OP_WITHOUT_BITWIDTH)
#undef CMP_BINARY_OP_WITHOUT_BITWIDTH

    inline GateRef Equal(GateRef x, GateRef y);
    inline GateRef NotEqual(GateRef x, GateRef y);

    // js world
    // cast operation
    inline GateRef GetInt64OfTInt(GateRef x);
    inline GateRef GetInt32OfTInt(GateRef x);
    inline GateRef TaggedCastToIntPtr(GateRef x);
    inline GateRef GetDoubleOfTDouble(GateRef x);
    inline GateRef GetBooleanOfTBoolean(GateRef x);
    inline GateRef GetDoubleOfTNumber(GateRef x);
    inline GateRef DoubleToInt(GateRef x, Label *exit);
    inline GateRef Int32ToTaggedPtr(GateRef x);
    inline GateRef Int64ToTaggedPtr(GateRef x);
    inline GateRef Int32ToTaggedInt(GateRef x);
    // bit operation
    inline GateRef IsSpecial(GateRef x, JSTaggedType type);
    inline GateRef TaggedIsInt(GateRef x);
    inline GateRef TaggedIsDouble(GateRef x);
    inline GateRef TaggedIsObject(GateRef x);
    inline GateRef TaggedIsNumber(GateRef x);
    inline GateRef TaggedIsNumeric(GateRef x);
    inline GateRef TaggedIsNotHole(GateRef x);
    inline GateRef TaggedIsHole(GateRef x);
    inline GateRef TaggedIsNullPtr(GateRef x);
    inline GateRef TaggedIsUndefined(GateRef x);
    inline GateRef TaggedIsException(GateRef x);
    inline GateRef TaggedIsSpecial(GateRef x);
    inline GateRef TaggedIsHeapObject(GateRef x);
    inline GateRef TaggedIsAsyncGeneratorObject(GateRef x);
    inline GateRef TaggedIsJSGlobalObject(GateRef x);
    inline GateRef TaggedIsGeneratorObject(GateRef x);
    inline GateRef TaggedIsJSArray(GateRef x);
    inline GateRef TaggedIsPropertyBox(GateRef x);
    inline GateRef TaggedIsWeak(GateRef x);
    inline GateRef TaggedIsPrototypeHandler(GateRef x);
    inline GateRef TaggedIsTransitionHandler(GateRef x);
    inline GateRef TaggedIsStoreTSHandler(GateRef x);
    inline GateRef TaggedIsTransWithProtoHandler(GateRef x);
    inline GateRef TaggedIsUndefinedOrNull(GateRef x);
    inline GateRef TaggedIsTrue(GateRef x);
    inline GateRef TaggedIsFalse(GateRef x);
    inline GateRef TaggedIsNull(GateRef x);
    inline GateRef TaggedIsBoolean(GateRef x);
    inline GateRef IsAOTLiteralInfo(GateRef x);
    inline GateRef TaggedGetInt(GateRef x);
    inline GateRef ToTaggedInt(GateRef x);
    inline GateRef ToTaggedIntPtr(GateRef x);
    inline GateRef DoubleToTaggedDoublePtr(GateRef x);
    inline GateRef BooleanToTaggedBooleanPtr(GateRef x);
    inline GateRef BooleanToInt32(GateRef x);
    inline GateRef BooleanToFloat64(GateRef x);
    inline GateRef Float32ToTaggedDoublePtr(GateRef x);
    inline GateRef TaggedDoublePtrToFloat32(GateRef x);
    inline GateRef TaggedIntPtrToFloat32(GateRef x);
    inline GateRef DoubleToTaggedDouble(GateRef x);
    inline GateRef DoubleToTagged(GateRef x);
    inline GateRef DoubleIsNAN(GateRef x);
    inline GateRef TaggedTrue();
    inline GateRef TaggedFalse();
    // Pointer/Arithmetic/Logic Operations
    inline GateRef IntPtrDiv(GateRef x, GateRef y);
    inline GateRef IntPtrOr(GateRef x, GateRef y);
    inline GateRef IntPtrLSL(GateRef x, GateRef y);
    inline GateRef IntPtrLSR(GateRef x, GateRef y);
    inline GateRef Int64NotEqual(GateRef x, GateRef y);
    inline GateRef Int32NotEqual(GateRef x, GateRef y);
    inline GateRef Int64Equal(GateRef x, GateRef y);
    inline GateRef Int8Equal(GateRef x, GateRef y);
    inline GateRef Int32Equal(GateRef x, GateRef y);
    inline GateRef IntPtrGreaterThan(GateRef x, GateRef y);
    template<OpCode Op, MachineType Type>
    inline GateRef BinaryOp(GateRef x, GateRef y);
    template<OpCode Op, MachineType Type>
    inline GateRef BinaryOpWithOverflow(GateRef x, GateRef y);
    inline GateRef GetLengthFromTaggedArray(GateRef array);
    inline GateRef GetValueFromTaggedArray(GateRef array, GateRef index);
    inline void SetValueToTaggedArray(VariableType valType, GateRef glue, GateRef array, GateRef index, GateRef val);
    GateRef TaggedIsString(GateRef obj);
    GateRef TaggedIsStringOrSymbol(GateRef obj);
    inline GateRef GetGlobalConstantString(ConstantIndex index);
    inline GateRef LoadObjectFromWeakRef(GateRef x);
    GateRef ComputeTaggedArraySize(GateRef length);

    GateRef IsJSHClass(GateRef obj);
    GateRef HasPendingException(GateRef glue);
    // middle ir: operations with any type
    template<TypedBinOp Op>
    inline GateRef TypedBinaryOp(GateRef x, GateRef y, GateType xType, GateType yType, GateType gateType,
        PGOSampleType sampleType);
    template<TypedUnOp Op>
    inline GateRef TypedUnaryOp(GateRef x, GateType xType, GateType gateType);
    template<TypedJumpOp Op>
    inline GateRef TypedConditionJump(GateRef x, GateType xType, BranchKind branchKind);
    inline GateRef PrimitiveToNumber(GateRef x, VariableType type);

    // middle ir: object operations
    GateRef ToLength(GateRef receiver);
    template<TypedLoadOp Op>
    GateRef LoadElement(GateRef receiver, GateRef index);
    template<TypedStoreOp Op>
    GateRef StoreElement(GateRef receiver, GateRef index, GateRef value);
    GateRef StoreMemory(MemoryType Op, VariableType type, GateRef receiver, GateRef index, GateRef value);
    GateRef LoadProperty(GateRef receiver, GateRef propertyLookupResult, bool isFunction);
    GateRef StoreProperty(GateRef receiver, GateRef propertyLookupResult, GateRef value);
    GateRef LoadArrayLength(GateRef array);
    GateRef LoadStringLength(GateRef string);
    GateRef Construct(GateRef hirGate, std::vector<GateRef> args);
    GateRef TypedCall(GateRef hirGate, std::vector<GateRef> args, bool isNoGC);
    GateRef TypedFastCall(GateRef hirGate, std::vector<GateRef> args, bool isNoGC);
    GateRef CallGetter(GateRef hirGate, GateRef receiver, GateRef propertyLookupResult, const char* comment = nullptr);
    GateRef CallSetter(GateRef hirGate, GateRef receiver, GateRef propertyLookupResult,
                       GateRef value, const char* comment = nullptr);
    GateRef GetConstPool(GateRef jsFunc);
    GateRef GetGlobalEnv();
    GateRef GetGlobalEnvObj(GateRef env, size_t index);
    GateRef GetGlobalEnvObjHClass(GateRef env, size_t index);
    GateRef GetGlobalConstantValue(ConstantIndex index);
    GateRef LoadConstOffset(VariableType type, GateRef receiver, size_t offset);
    GateRef StoreConstOffset(VariableType type, GateRef receiver, size_t offset, GateRef value);
    GateRef LoadObjectFromConstPool(GateRef jsFunc, GateRef index);
    // Object Operations
    inline GateRef LoadHClass(GateRef object);
    inline GateRef IsJSFunction(GateRef obj);
    inline GateRef IsJSFunctionWithBit(GateRef obj);
    inline GateRef IsOptimizedAndNotFastCall(GateRef obj);
    inline GateRef IsOptimized(GateRef obj);
    inline GateRef IsOptimizedWithBitField(GateRef bitfield);
    inline GateRef CanFastCall(GateRef obj);
    inline GateRef CanFastCallWithBitField(GateRef bitfield);
    inline GateRef IsDictionaryMode(GateRef object);
    inline void StoreHClass(GateRef glue, GateRef object, GateRef hClass);
    inline GateRef IsJsType(GateRef object, JSType type);
    inline GateRef GetObjectType(GateRef hClass);
    inline GateRef IsDictionaryModeByHClass(GateRef hClass);
    inline GateRef GetElementsKindByHClass(GateRef hClass);
    inline GateRef HasConstructorByHClass(GateRef hClass);
    inline GateRef IsStableElements(GateRef hClass);
    inline GateRef IsStableArguments(GateRef hClass);
    inline GateRef IsStableArray(GateRef hClass);
    inline GateRef DoubleIsINF(GateRef x);
    inline GateRef IsDictionaryElement(GateRef hClass);
    inline GateRef IsClassConstructor(GateRef object);
    inline GateRef IsClassConstructorWithBitField(GateRef bitfield);
    inline GateRef HasConstructor(GateRef object);
    inline GateRef IsConstructor(GateRef object);
    inline GateRef IsClassPrototype(GateRef object);
    inline GateRef IsClassPrototypeWithBitField(GateRef object);
    inline GateRef IsExtensible(GateRef object);
    inline GateRef GetExpectedNumOfArgs(GateRef method);
    inline GateRef TaggedObjectIsEcmaObject(GateRef obj);
    inline GateRef IsJSObject(GateRef obj);
    inline GateRef TaggedObjectIsString(GateRef obj);
    inline GateRef TaggedObjectBothAreString(GateRef x, GateRef y);
    inline GateRef IsCallable(GateRef obj);
    inline GateRef IsCallableFromBitField(GateRef bitfield);
    inline GateRef LogicAnd(GateRef x, GateRef y);
    inline GateRef LogicOr(GateRef x, GateRef y);
    inline GateRef BothAreString(GateRef x, GateRef y);
    inline GateRef GetObjectSizeFromHClass(GateRef hClass);
    inline GateRef IsTreeString(GateRef obj);
    inline GateRef TreeStringIsFlat(GateRef string);
    inline GateRef GetFirstFromTreeString(GateRef string);
    inline GateRef GetSecondFromTreeString(GateRef string);
    GateRef GetGlobalObject(GateRef glue);
    GateRef GetMethodFromFunction(GateRef function);
    GateRef GetModuleFromFunction(GateRef function);
    GateRef GetHomeObjectFromFunction(GateRef function);
    GateRef FunctionIsResolved(GateRef function);
    GateRef GetLengthFromString(GateRef value);
    GateRef GetHashcodeFromString(GateRef glue, GateRef value);
    GateRef TryGetHashcodeFromString(GateRef string);
    GateRef IsUtf16String(GateRef string);
    GateRef TaggedIsBigInt(GateRef obj);
    void SetLexicalEnvToFunction(GateRef glue, GateRef function, GateRef value);
    GateRef GetFunctionLexicalEnv(GateRef function);
    void SetModuleToFunction(GateRef glue, GateRef function, GateRef value);
    void SetPropertyInlinedProps(GateRef glue, GateRef obj, GateRef hClass,
        GateRef value, GateRef attrOffset, VariableType type);
    void SetHomeObjectToFunction(GateRef glue, GateRef function, GateRef value);
    GateRef GetConstPoolFromFunction(GateRef jsFunc);
    GateRef GetCodeAddr(GateRef method);
    GateRef GetObjectFromConstPool(GateRef glue, GateRef hirGate, GateRef jsFunc, GateRef index, ConstPoolType type);
    GateRef GetObjectFromConstPool(GateRef glue, GateRef hirGate, GateRef constPool, GateRef module, GateRef index,
                                   ConstPoolType type);
    void ClearConstantCache(GateRef gate);

    // middle ir: create new obj
    GateRef StartAllocate();
    GateRef FinishAllocate();
    GateRef HeapAlloc(GateRef size, GateType type, RegionSpaceFlag flag);
    GateRef CreateArray(ElementsKind kind, uint32_t arraySize);
    GateRef CreateArrayWithBuffer(ElementsKind kind, ArrayMetaDataAccessor::Mode mode,
                                  GateRef constPoolIndex, GateRef elementIndex);

    void SetEnvironment(Environment *env)
    {
        env_ = env;
    }
    Environment *GetCurrentEnvironment() const
    {
        return env_;
    }
    void SetCompilationConfig(CompilationConfig *cmpCfg)
    {
        cmpCfg_ = cmpCfg;
    }
    CompilationConfig *GetCompilationConfig() const
    {
        return cmpCfg_;
    }
    // label related
    void NewEnvironment(GateRef hir);
    void DeleteCurrentEnvironment();
    inline int NextVariableId();
    inline void HandleException(GateRef result, Label *success, Label *exception, Label *exit);
    inline void HandleException(GateRef result, Label *success, Label *fail, Label *exit, GateRef exceptionVal);
    inline void SubCfgEntry(Label *entry);
    inline void SubCfgExit();
    inline GateRef Return(GateRef value);
    inline GateRef Return();
    inline void Bind(Label *label);
    inline void Bind(Label *label, bool justSlowPath);
    void Jump(Label *label);
    void Branch(GateRef condition, Label *trueLabel, Label *falseLabel,
        uint32_t trueWeight = 1, uint32_t falseWeight = 1);   // 1: default branch weight
    void Switch(GateRef index, Label *defaultLabel, int64_t *keysValue, Label *keysLabel, int numberOfKeys);
    void LoopBegin(Label *loopHead);
    void LoopEnd(Label *loopHead);
    inline Label *GetCurrentLabel() const;
    inline GateRef GetState() const;
    inline GateRef GetDepend() const;
    inline StateDepend GetStateDepend() const;

    GateRef GetGlobalEnvValue(VariableType type, GateRef env, size_t index);
    GateRef IsBase(GateRef ctor);
    inline GateRef GetMethodId(GateRef func);
    inline GateRef LoadFromTaggedArray(GateRef array, size_t index);
    inline GateRef StoreToTaggedArray(GateRef array, size_t index, GateRef value);

private:
    inline void SetDepend(GateRef depend);
    inline void SetState(GateRef state);

#define ARITHMETIC_UNARY_OP_WITH_BITWIDTH(NAME, OPCODEID, MACHINETYPEID)                            \
    inline GateRef NAME(GateRef x)                                                                  \
    {                                                                                               \
        return circuit_->NewGate(circuit_->OPCODEID(), MACHINETYPEID, { x }, GateType::NJSValue()); \
    }

    UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH_PRIVATE(ARITHMETIC_UNARY_OP_WITH_BITWIDTH)
#undef ARITHMETIC_UNARY_OP_WITH_BITWIDTH

    Circuit *circuit_ {nullptr};
    GateAccessor acc_;
    Environment *env_ {nullptr};
    CompilationConfig *cmpCfg_ {nullptr};
    friend StubBuilder;
    friend TSHCRLowering;
    friend NTypeHCRLowering;
    friend SlowPathLowering;
};

class Label {
public:
    Label() = default;
    explicit Label(Environment *env);
    explicit Label(CircuitBuilder *cirBuilder);
    ~Label() = default;
    Label(Label const &label) = default;
    Label &operator=(Label const &label) = default;
    Label(Label &&label) = default;
    Label &operator=(Label &&label) = default;
    inline void Seal();
    inline void WriteVariable(Variable *var, GateRef value)
    {
        impl_->WriteVariable(var, value);
    }
    inline GateRef ReadVariable(Variable *var)
    {
        return impl_->ReadVariable(var);
    }
    inline void Bind();
    inline void MergeAllControl();
    inline void MergeAllDepend();
    inline void AppendPredecessor(const Label *predecessor);
    inline std::vector<Label> GetPredecessors() const;
    inline void SetControl(GateRef control);
    inline void SetPreControl(GateRef control);
    inline void MergeControl(GateRef control);
    inline GateRef GetControl() const;
    inline GateRef GetDepend() const;
    inline void SetDepend(GateRef depend);

private:
    class LabelImpl {
    public:
        LabelImpl(Environment *env, GateRef control)
            : env_(env), control_(control), predeControl_(-1), isSealed_(false)
        {
        }
        ~LabelImpl() = default;
        NO_MOVE_SEMANTIC(LabelImpl);
        NO_COPY_SEMANTIC(LabelImpl);
        void Seal();
        void WriteVariable(Variable *var, GateRef value);
        GateRef ReadVariable(Variable *var);
        void Bind();
        void MergeAllControl();
        void MergeAllDepend();
        void AppendPredecessor(LabelImpl *predecessor);
        std::vector<LabelImpl *> GetPredecessors() const
        {
            return predecessors_;
        }
        void SetControl(GateRef control)
        {
            control_ = control;
        }
        void SetPreControl(GateRef control)
        {
            predeControl_ = control;
        }
        void MergeControl(GateRef control)
        {
            if (predeControl_ == Circuit::NullGate()) {
                predeControl_ = control;
                control_ = predeControl_;
            } else {
                otherPredeControls_.push_back(control);
            }
        }
        GateRef GetControl() const
        {
            return control_;
        }
        void SetDepend(GateRef depend)
        {
            depend_ = depend;
        }
        GateRef GetDepend() const
        {
            return depend_;
        }

    private:
        bool IsNeedSeal() const;
        bool IsSealed() const
        {
            return isSealed_;
        }
        bool IsLoopHead() const;
        bool IsControlCase() const;
        GateRef ReadVariableRecursive(Variable *var);
        Environment *env_;
        GateRef control_;
        GateRef predeControl_ {Circuit::NullGate()};
        GateRef depend_ {Circuit::NullGate()};
        GateRef loopDepend_ {Circuit::NullGate()};
        std::vector<GateRef> otherPredeControls_;
        bool isSealed_ {false};
        std::map<Variable *, GateRef> valueMap_;
        std::vector<GateRef> phi;
        std::vector<LabelImpl *> predecessors_;
        std::map<Variable *, GateRef> incompletePhis_;
    };

    explicit Label(LabelImpl *impl) : impl_(impl) {}
    friend class Environment;
    LabelImpl *GetRawLabel() const
    {
        return impl_;
    }
    LabelImpl *impl_ {nullptr};
};

class Environment {
public:
    using LabelImpl = Label::LabelImpl;
    Environment(GateRef hir, Circuit *circuit, CircuitBuilder *builder);
    Environment(GateRef stateEntry, GateRef dependEntry, const std::initializer_list<GateRef>& args,
                Circuit *circuit, CircuitBuilder *builder);
    Environment(size_t arguments, CircuitBuilder *builder);
    ~Environment();
    Label *GetCurrentLabel() const
    {
        return currentLabel_;
    }
    void SetCurrentLabel(Label *label)
    {
        currentLabel_ = label;
    }
    CircuitBuilder *GetBuilder() const
    {
        return circuitBuilder_;
    }
    Circuit *GetCircuit() const
    {
        return circuit_;
    }
    int NextVariableId()
    {
        return nextVariableId_++;
    }
    void SetCompilationConfig(const CompilationConfig *cfg)
    {
        ccfg_ = cfg;
    }
    const CompilationConfig *GetCompilationConfig() const
    {
        return ccfg_;
    }
    inline bool Is32Bit() const
    {
        return ccfg_->Is32Bit();
    }
    inline bool IsAArch64() const
    {
        return ccfg_->IsAArch64();
    }
    inline bool IsAmd64() const
    {
        return ccfg_->IsAmd64();
    }
    inline bool IsArch64Bit() const
    {
        return ccfg_->IsAmd64() ||  ccfg_->IsAArch64();
    }
    inline bool IsAsmInterp() const
    {
        return circuit_->GetFrameType() == FrameType::ASM_INTERPRETER_FRAME;
    }
    inline bool IsArch32Bit() const
    {
        return ccfg_->Is32Bit();
    }
    inline GateRef GetArgument(size_t index) const
    {
        return arguments_.at(index);
    }

    inline Label GetLabelFromSelector(GateRef sel);
    inline void AddSelectorToLabel(GateRef sel, Label label);
    inline LabelImpl *NewLabel(Environment *env, GateRef control = -1);
    inline void SubCfgEntry(Label *entry);
    inline void SubCfgExit();
    inline GateRef GetInput(size_t index) const;

private:
    Label *currentLabel_ {nullptr};
    Circuit *circuit_ {nullptr};
    CircuitBuilder *circuitBuilder_ {nullptr};
    std::unordered_map<GateRef, LabelImpl *> phiToLabels_;
    std::vector<GateRef> inputList_;
    Label entry_;
    std::vector<LabelImpl *> rawLabels_;
    std::stack<Label *> stack_;
    int nextVariableId_ {0};
    std::vector<GateRef> arguments_;
    const CompilationConfig *ccfg_ {nullptr};
};

class Variable {
public:
    Variable(Environment *env, VariableType type, uint32_t id, GateRef value) : id_(id), type_(type), env_(env)
    {
        Bind(value);
        env_->GetCurrentLabel()->WriteVariable(this, value);
    }
    Variable(CircuitBuilder *cirbuilder, VariableType type, uint32_t id, GateRef value)
        : id_(id), type_(type), env_(cirbuilder->GetCurrentEnvironment())
    {
        Bind(value);
        env_->GetCurrentLabel()->WriteVariable(this, value);
    }
    ~Variable() = default;
    NO_MOVE_SEMANTIC(Variable);
    NO_COPY_SEMANTIC(Variable);
    void Bind(GateRef value)
    {
        currentValue_ = value;
    }
    GateRef Value() const
    {
        return currentValue_;
    }
    VariableType Type() const
    {
        return type_;
    }
    bool IsBound() const
    {
        return currentValue_ != 0;
    }
    Variable &operator=(const GateRef value)
    {
        env_->GetCurrentLabel()->WriteVariable(this, value);
        Bind(value);
        return *this;
    }
    GateRef operator*()
    {
        return env_->GetCurrentLabel()->ReadVariable(this);
    }
    GateRef ReadVariable()
    {
        return env_->GetCurrentLabel()->ReadVariable(this);
    }
    void WriteVariable(GateRef value)
    {
        env_->GetCurrentLabel()->WriteVariable(this, value);
        Bind(value);
    }
    GateRef AddPhiOperand(GateRef val);
    GateRef AddOperandToSelector(GateRef val, size_t idx, GateRef in);
    GateRef TryRemoveTrivialPhi(GateRef phi);
    uint32_t GetId() const
    {
        return id_;
    }

private:
    Circuit* GetCircuit() const
    {
        return env_->GetCircuit();
    }

    uint32_t id_;
    VariableType type_;
    GateRef currentValue_ {0};
    Environment *env_;
};
}  // namespace panda::ecmascript::kungfu

#endif  // ECMASCRIPT_COMPILER_CIRCUIT_BUILDER_H
