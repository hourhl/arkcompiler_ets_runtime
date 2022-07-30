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

#include "ecmascript/compiler/trampoline/aarch64/common_call.h"

#include "ecmascript/compiler/assembler/assembler.h"
#include "ecmascript/compiler/argument_accessor.h"
#include "ecmascript/compiler/common_stubs.h"
#include "ecmascript/compiler/rt_call_signature.h"
#include "ecmascript/ecma_runtime_call_info.h"
#include "ecmascript/frames.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_method.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/js_generator_object.h"
#include "ecmascript/message_string.h"
#include "ecmascript/runtime_call_id.h"

#include "libpandafile/bytecode_instruction-inl.h"

namespace panda::ecmascript::aarch64 {
using Label = panda::ecmascript::Label;
#define __ assembler->

// Generate code for entering asm interpreter
// c++ calling convention
// Input: glue           - %X0
//        callTarget     - %X1
//        method         - %X2
//        callField      - %X3
//        argc           - %X4
//        argv           - %X5(<callTarget, newTarget, this> are at the beginning of argv)
void AsmInterpreterCall::AsmInterpreterEntry(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(AsmInterpreterEntry));
    Label target;
    PushAsmInterpEntryFrame(assembler);
    __ Bl(&target);
    PopAsmInterpEntryFrame(assembler);
    __ Ret();

    __ Bind(&target);
    {
        JSCallDispatch(assembler);
    }
}

// Input: glue           - %X0
//        callTarget     - %X1
//        method         - %X2
//        callField      - %X3
//        argc           - %X4
//        argv           - %X5(<callTarget, newTarget, this> are at the beginning of argv)
void AsmInterpreterCall::JSCallDispatch(ExtendedAssembler *assembler)
{
    Label notJSFunction;
    Label callNativeEntry;
    Label callJSFunctionEntry;
    Label notCallable;
    Register glueRegister(X0);
    Register argcRegister(X4, W);
    Register argvRegister(X5);
    Register callTargetRegister(X1);
    Register callFieldRegister(X3);
    Register bitFieldRegister(X16);
    Register tempRegister(X17); // can not be used to store any variable
    Register functionTypeRegister(X18, W);
    __ Ldr(tempRegister, MemoryOperand(callTargetRegister, 0)); // hclass
    __ Ldr(bitFieldRegister, MemoryOperand(tempRegister, JSHClass::BIT_FIELD_OFFSET));
    __ And(functionTypeRegister, bitFieldRegister.W(), LogicalImmediate::Create(0xFF, RegWSize));
    __ Mov(tempRegister.W(), Immediate(static_cast<int64_t>(JSType::JS_FUNCTION_BEGIN)));
    __ Cmp(functionTypeRegister, tempRegister.W());
    __ B(Condition::LO, &notJSFunction);
    __ Mov(tempRegister.W(), Immediate(static_cast<int64_t>(JSType::JS_FUNCTION_END)));
    __ Cmp(functionTypeRegister, tempRegister.W());
    __ B(Condition::LS, &callJSFunctionEntry);
    __ Bind(&notJSFunction);
    {
        __ Tst(bitFieldRegister,
            LogicalImmediate::Create(static_cast<int64_t>(1ULL << JSHClass::CallableBit::START_BIT), RegXSize));
        __ B(Condition::EQ, &notCallable);
        // fall through
    }
    __ Bind(&callNativeEntry);
    CallNativeEntry(assembler);
    __ Bind(&callJSFunctionEntry);
    {
        __ Tbnz(callFieldRegister, JSMethod::IsNativeBit::START_BIT, &callNativeEntry);
        // fast path
        __ Add(argvRegister, argvRegister, Immediate(NUM_MANDATORY_JSFUNC_ARGS * JSTaggedValue::TaggedTypeSize()));
        JSCallCommonEntry(assembler, JSCallMode::CALL_ENTRY);
    }
    __ Bind(&notCallable);
    {
        Register runtimeId(X11);
        Register trampoline(X12);
        __ Mov(runtimeId, Immediate(kungfu::RuntimeStubCSigns::ID_ThrowNotCallableException));
        // 3 : 3 means *8
        __ Add(trampoline, glueRegister, Operand(runtimeId, LSL, 3));
        __ Ldr(trampoline, MemoryOperand(trampoline, JSThread::GlueData::GetRTStubEntriesOffset(false)));
        __ Blr(trampoline);
        __ Ret();
    }
}

void AsmInterpreterCall::JSCallCommonEntry(ExtendedAssembler *assembler, JSCallMode mode)
{
    Label stackOverflow;
    Register glueRegister = __ GlueRegister();
    Register fpRegister = __ AvailableRegister1();
    Register currentSlotRegister = __ AvailableRegister3();
    Register callFieldRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_FIELD);
    Register argcRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARGC);
    if (!kungfu::AssemblerModule::IsJumpToCallCommonEntry(mode)) {
        __ PushFpAndLr();
    }
    // save fp
    __ Mov(fpRegister, Register(SP));
    __ Mov(currentSlotRegister, Register(SP));

    {
        // Reserve enough sp space to prevent stack parameters from being covered by cpu profiler.
        [[maybe_unused]] TempRegister1Scope scope(assembler);
        Register tempRegister = __ TempRegister1();
        __ Ldr(tempRegister, MemoryOperand(glueRegister, JSThread::GlueData::GetStackLimitOffset(false)));
        __ Mov(Register(SP), tempRegister);
    }

    auto jumpSize = kungfu::AssemblerModule::GetJumpSizeFromJSCallMode(mode);
    if (mode == JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV) {
        Register thisRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG2);
        [[maybe_unused]] TempRegister1Scope scope(assembler);
        Register tempArgcRegister = __ TempRegister1();
        __ PushArgc(argcRegister, tempArgcRegister, currentSlotRegister);
        __ Str(thisRegister, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
        jumpSize = 0;
    }

    if (jumpSize >= 0) {
        [[maybe_unused]] TempRegister1Scope scope(assembler);
        Register temp = __ TempRegister1();
        __ Mov(temp, Immediate(static_cast<int>(jumpSize)));
        int64_t offset = static_cast<int64_t>(AsmInterpretedFrame::GetCallSizeOffset(false))
            - static_cast<int64_t>(AsmInterpretedFrame::GetSize(false));
        ASSERT(offset < 0);
        __ Stur(temp, MemoryOperand(Register(FP), offset));
    }

    Register declaredNumArgsRegister = __ AvailableRegister2();
    GetDeclaredNumArgsFromCallField(assembler, callFieldRegister, declaredNumArgsRegister);

    Label slowPathEntry;
    Label fastPathEntry;
    Label pushCallThis;
    auto argc = kungfu::AssemblerModule::GetArgcFromJSCallMode(mode);
    if (argc >= 0) {
        __ Cmp(declaredNumArgsRegister, Immediate(argc));
    } else {
        __ Cmp(declaredNumArgsRegister, argcRegister);
    }
    __ B(Condition::NE, &slowPathEntry);
    __ Bind(&fastPathEntry);
    JSCallCommonFastPath(assembler, mode, &pushCallThis, &stackOverflow);
    __ Bind(&pushCallThis);
    PushCallThis(assembler, mode, &stackOverflow);
    __ Bind(&slowPathEntry);
    JSCallCommonSlowPath(assembler, mode, &fastPathEntry, &pushCallThis, &stackOverflow);

    __ Bind(&stackOverflow);
    if (kungfu::AssemblerModule::IsJumpToCallCommonEntry(mode)) {
        __ Mov(Register(SP), fpRegister);
        [[maybe_unused]] TempRegister1Scope scope(assembler);
        Register temp = __ TempRegister1();
        // only glue and acc are useful in exception handler
        if (glueRegister.GetId() != X19) {
            __ Mov(Register(X19), glueRegister);
        }
        Register acc(X23);
        __ Mov(acc, Immediate(JSTaggedValue::VALUE_EXCEPTION));
        Register methodRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::METHOD);
        Register callTargetRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_TARGET);
        // Reload pc to make sure stack trace is right
        __ Mov(temp, callTargetRegister);
        __ Ldr(Register(X20), MemoryOperand(methodRegister, JSMethod::GetBytecodeArrayOffset(false)));
        // Reload constpool and profileInfo to make sure gc map work normally
        __ Ldr(Register(X22), MemoryOperand(temp, JSFunction::PROFILE_TYPE_INFO_OFFSET));
        __ Ldr(Register(X21), MemoryOperand(temp, JSFunction::CONSTANT_POOL_OFFSET));

        __ Mov(temp, kungfu::BytecodeStubCSigns::ID_ThrowStackOverflowException);
        __ Add(temp, glueRegister, Operand(temp, UXTW, 3));  // 3： bc * 8
        __ Ldr(temp, MemoryOperand(temp, JSThread::GlueData::GetBCStubEntriesOffset(false)));
        __ Br(temp);
    } else {
        [[maybe_unused]] TempRegister1Scope scope(assembler);
        Register temp = __ TempRegister1();
        ThrowStackOverflowExceptionAndReturn(assembler, glueRegister, fpRegister, temp);
    }
}

void AsmInterpreterCall::JSCallCommonFastPath(ExtendedAssembler *assembler, JSCallMode mode, Label *pushCallThis,
    Label *stackOverflow)
{
    Register glueRegister = __ GlueRegister();
    auto argc = kungfu::AssemblerModule::GetArgcFromJSCallMode(mode);
    Register currentSlotRegister = __ AvailableRegister3();
    // call range
    if (argc < 0) {
        Register numRegister = __ AvailableRegister2();
        Register argcRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARGC);
        Register argvRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARGV);
        __ Mov(numRegister, argcRegister);
        [[maybe_unused]] TempRegister1Scope scope(assembler);
        Register opRegister = __ TempRegister1();
        PushArgsWithArgv(assembler, glueRegister, numRegister, argvRegister, opRegister, currentSlotRegister,
            pushCallThis, stackOverflow);
    } else if (argc > 0) {
        Register arg0 = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG0);
        Register arg1 = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG1);
        Register arg2 = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG2);
        if (argc > 2) { // 2: call arg2
            __ Str(arg2, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
        }
        if (argc > 1) {
            __ Str(arg1, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
        }
        if (argc > 0) {
            __ Str(arg0, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
        }
    }
}

void AsmInterpreterCall::JSCallCommonSlowPath(ExtendedAssembler *assembler, JSCallMode mode,
                                              Label *fastPathEntry, Label *pushCallThis, Label *stackOverflow)
{
    Register glueRegister = __ GlueRegister();
    Register callFieldRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_FIELD);
    Register argcRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARGC);
    Register argvRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARGV);
    Register currentSlotRegister = __ AvailableRegister3();
    Register arg0 = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG0);
    Register arg1 = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG1);
    Label noExtraEntry;
    Label pushArgsEntry;

    auto argc = kungfu::AssemblerModule::GetArgcFromJSCallMode(mode);
    Register declaredNumArgsRegister = __ AvailableRegister2();
    __ Tbz(callFieldRegister, JSMethod::HaveExtraBit::START_BIT, &noExtraEntry);
    // extra entry
    {
        [[maybe_unused]] TempRegister1Scope scope1(assembler);
        Register tempArgcRegister = __ TempRegister1();
        if (argc >= 0) {
            __ PushArgc(argc, tempArgcRegister, currentSlotRegister);
        } else {
            __ PushArgc(argcRegister, tempArgcRegister, currentSlotRegister);
        }
        // fall through
    }
    __ Bind(&noExtraEntry);
    {
        if (argc == 0) {
            {
                [[maybe_unused]] TempRegister1Scope scope(assembler);
                Register tempRegister = __ TempRegister1();
                PushUndefinedWithArgc(assembler, glueRegister, declaredNumArgsRegister, tempRegister,
                    currentSlotRegister, nullptr, stackOverflow);
            }
            __ B(fastPathEntry);
            return;
        }
        [[maybe_unused]] TempRegister1Scope scope1(assembler);
        Register diffRegister = __ TempRegister1();
        if (argc >= 0) {
            __ Sub(diffRegister.W(), declaredNumArgsRegister.W(), Immediate(argc));
        } else {
            __ Sub(diffRegister.W(), declaredNumArgsRegister.W(), argcRegister.W());
        }
        [[maybe_unused]] TempRegister2Scope scope2(assembler);
        Register tempRegister = __ TempRegister2();
        PushUndefinedWithArgc(assembler, glueRegister, diffRegister, tempRegister, currentSlotRegister, &pushArgsEntry,
            stackOverflow);
        __ B(fastPathEntry);
    }
    // declare < actual
    __ Bind(&pushArgsEntry);
    {
        __ Tbnz(callFieldRegister, JSMethod::HaveExtraBit::START_BIT, fastPathEntry);
        // no extra branch
        // arg1, declare must be 0
        if (argc == 1) {
            __ B(pushCallThis);
            return;
        }
        __ Cmp(declaredNumArgsRegister, Immediate(0));
        __ B(Condition::EQ, pushCallThis);
        // call range
        if (argc < 0) {
            [[maybe_unused]] TempRegister1Scope scope(assembler);
            Register opRegister = __ TempRegister1();
            PushArgsWithArgv(assembler, glueRegister, declaredNumArgsRegister, argvRegister, opRegister,
                currentSlotRegister, nullptr, stackOverflow);
        } else if (argc > 0) {
            Label pushArgs0;
            if (argc > 2) {  // 2: call arg2
                // decalare is 2 or 1 now
                __ Cmp(declaredNumArgsRegister, Immediate(1));
                __ B(Condition::EQ, &pushArgs0);
                __ Str(arg1, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
            }
            if (argc > 1) {
                __ Bind(&pushArgs0);
                // decalare is is 1 now
                __ Str(arg0, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
            }
        }
        __ B(pushCallThis);
    }
}

Register AsmInterpreterCall::GetThisRegsiter(ExtendedAssembler *assembler, JSCallMode mode)
{
    switch (mode) {
        case JSCallMode::CALL_GETTER:
            return __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG0);
        case JSCallMode::CALL_SETTER:
            return __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG1);
        case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::CALL_THIS_WITH_ARGV:
            return __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG2);
        case JSCallMode::CALL_FROM_AOT:
        case JSCallMode::CALL_ENTRY: {
            Register argvRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG1);
            Register thisRegister = __ AvailableRegister2();
            __ Ldur(thisRegister, MemoryOperand(argvRegister, -FRAME_SLOT_SIZE));
            return thisRegister;
        }
        default:
            UNREACHABLE();
    }
    return INVALID_REG;
}

Register AsmInterpreterCall::GetNewTargetRegsiter(ExtendedAssembler *assembler, JSCallMode mode)
{
    switch (mode) {
        case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::CALL_THIS_WITH_ARGV:
            return __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_TARGET);
        case JSCallMode::CALL_FROM_AOT:
        case JSCallMode::CALL_ENTRY: {
            Register argvRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG1);
            Register newTargetRegister = __ AvailableRegister2();
            // 2: new Target index
            __ Ldur(newTargetRegister, MemoryOperand(argvRegister, -2 * FRAME_SLOT_SIZE));
            return newTargetRegister;
        }
        default:
            UNREACHABLE();
    }
    return INVALID_REG;
}

// void PushCallArgsxAndDispatch(uintptr_t glue, uintptr_t sp, uint64_t callTarget, uintptr_t method,
//     uint64_t callField, ...)
// GHC calling convention
// Input1: for callarg0/1/2/3        Input2: for callrange
// X19 - glue                        // X19 - glue
// FP  - sp                          // FP  - sp
// X20 - callTarget                  // X20 - callTarget
// X21 - method                      // X21 - method
// X22 - callField                   // X22 - callField
// X23 - arg0                        // X23 - actualArgc
// X24 - arg1                        // X24 - argv
// X25 - arg2
void AsmInterpreterCall::PushCallIThisRangeAndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallIThisRangeAndDispatch));
    JSCallCommonEntry(assembler, JSCallMode::CALL_THIS_WITH_ARGV);
}

void AsmInterpreterCall::PushCallIRangeAndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallIRangeAndDispatch));
    JSCallCommonEntry(assembler, JSCallMode::CALL_WITH_ARGV);
}

void AsmInterpreterCall::PushCallNewAndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallNewAndDispatch));
    JSCallCommonEntry(assembler, JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV);
}

void AsmInterpreterCall::PushCallArgs3AndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallArgs3AndDispatch));
    JSCallCommonEntry(assembler, JSCallMode::CALL_ARG3);
}

void AsmInterpreterCall::PushCallArgs2AndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallArgs2AndDispatch));
    JSCallCommonEntry(assembler, JSCallMode::CALL_ARG2);
}

void AsmInterpreterCall::PushCallArgs1AndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallArgs1AndDispatch));
    JSCallCommonEntry(assembler, JSCallMode::CALL_ARG1);
}

void AsmInterpreterCall::PushCallArgs0AndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallArgs0AndDispatch));
    JSCallCommonEntry(assembler, JSCallMode::CALL_ARG0);
}

// uint64_t PushCallIRangeAndDispatchNative(uintptr_t glue, uint32_t argc, JSTaggedType calltarget, uintptr_t argv[])
// c++ calling convention call js function
// Input: X0 - glue
//        X1 - nativeCode
//        X2 - callTarget
//        X3 - thisValue
//        X4  - argc
//        X5  - argV (...)
void AsmInterpreterCall::PushCallIRangeAndDispatchNative(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallIRangeAndDispatchNative));
    CallNativeWithArgv(assembler, false);
}

void AsmInterpreterCall::PushCallNewAndDispatchNative(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallNewAndDispatchNative));
    CallNativeWithArgv(assembler, true);
}

void AsmInterpreterCall::CallNativeWithArgv(ExtendedAssembler *assembler, bool callNew)
{
    Register glue(X0);
    Register nativeCode(X1);
    Register callTarget(X2);
    Register thisObj(X3);
    Register argc(X4);
    Register argv(X5);
    Register opArgc(X8);
    Register opArgv(X9);
    Register temp(X10);
    Register currentSlotRegister(X11);
    Register spRegister(SP);

    Label pushThis;
    Label stackOverflow;
    PushBuiltinFrame(assembler, glue, FrameType::BUILTIN_FRAME_WITH_ARGV, temp, argc);

    __ Mov(currentSlotRegister, spRegister);
    // Reserve enough sp space to prevent stack parameters from being covered by cpu profiler.
    __ Ldr(temp, MemoryOperand(glue, JSThread::GlueData::GetStackLimitOffset(false)));
    __ Mov(Register(SP), temp);

    __ Mov(opArgc, argc);
    __ Mov(opArgv, argv);
    PushArgsWithArgv(assembler, glue, opArgc, opArgv, temp, currentSlotRegister, &pushThis, &stackOverflow);

    __ Bind(&pushThis);
    // newTarget
    if (callNew) {
        // 16: this & newTarget
        __ Stp(callTarget, thisObj, MemoryOperand(currentSlotRegister, -16, AddrMode::PREINDEX));
    } else {
        __ Mov(temp, Immediate(JSTaggedValue::VALUE_UNDEFINED));
        // 16: this & newTarget
        __ Stp(temp, thisObj, MemoryOperand(currentSlotRegister, -16, AddrMode::PREINDEX));
    }
    // callTarget
    __ Str(callTarget, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    __ Add(temp, currentSlotRegister, Immediate(40));  // 40: skip frame type, numArgs, func, newTarget and this
    __ Add(Register(FP), temp, Operand(argc, LSL, 3));  // 3: argc * 8

    __ Add(temp, argc, Immediate(NUM_MANDATORY_JSFUNC_ARGS));
    // 2: thread & argc
    __ Stp(glue, temp, MemoryOperand(currentSlotRegister, -2 * FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    __ Add(Register(X0), currentSlotRegister, Immediate(0));

    __ Align16(currentSlotRegister);
    __ Mov(spRegister, currentSlotRegister);

    CallNativeInternal(assembler, nativeCode);
    __ Ret();

    __ Bind(&stackOverflow);
    {
        // use builtin_with_argv_frame to mark gc map
        Register frameType(X11);
        __ Ldr(temp, MemoryOperand(glue, JSThread::GlueData::GetLeaveFrameOffset(false)));
        __ Mov(spRegister, temp);
        __ Mov(frameType, Immediate(static_cast<int32_t>(FrameType::BUILTIN_FRAME_WITH_ARGV)));
        // 2: frame type and argc
        __ Stp(Register(Zero), frameType, MemoryOperand(Register(SP), -FRAME_SLOT_SIZE * 2, AddrMode::PREINDEX));
        __ Mov(temp, Immediate(JSTaggedValue::VALUE_UNDEFINED));
        // 2: fill this&newtgt slots
        __ Stp(temp, temp, MemoryOperand(spRegister, -FRAME_SLOT_SIZE * 2, AddrMode::PREINDEX));
        // 2: fill func&align slots
        __ Stp(Register(Zero), temp, MemoryOperand(spRegister, -FRAME_SLOT_SIZE * 2, AddrMode::PREINDEX));
        __ Mov(temp, spRegister);
        __ Add(Register(FP), temp, Immediate(48));  // 48: skip frame type, numArgs, func, newTarget, this and align

        Register runtimeId(X11);
        Register trampoline(X12);
        __ Mov(runtimeId, Immediate(kungfu::RuntimeStubCSigns::ID_ThrowStackOverflowException));
        // 3 : 3 means *8
        __ Add(trampoline, glue, Operand(runtimeId, LSL, 3));
        __ Ldr(trampoline, MemoryOperand(trampoline, JSThread::GlueData::GetRTStubEntriesOffset(false)));
        __ Blr(trampoline);

        // resume rsp
        __ Mov(Register(SP), Register(FP));
        __ RestoreFpAndLr();
        __ Ret();
    }
}

// uint64_t PushCallArgsAndDispatchNative(uintptr_t codeAddress, uintptr_t glue, uint32_t argc, ...)
// webkit_jscc calling convention call runtime_id's runtion function(c-abi)
// Input: X0 - codeAddress
// stack layout: sp + N*8 argvN
//               ........
//               sp + 24: argv1
//               sp + 16: argv0
//               sp + 8:  actualArgc
//               sp:      thread
// construct Native Leave Frame
//               +--------------------------+
//               |       argv0              | calltarget , newTarget, this, ....
//               +--------------------------+ ---
//               |       argc               |   ^
//               |--------------------------|  Fixed
//               |       thread             | BuiltinFrame
//               |--------------------------|   |
//               |       returnAddr         |   |
//               |--------------------------|   |
//               |       callsiteFp         |   |
//               |--------------------------|   |
//               |       frameType          |   v
//               +--------------------------+ ---
void AsmInterpreterCall::PushCallArgsAndDispatchNative(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(PushCallArgsAndDispatchNative));

    Register nativeCode(X0);
    Register glue(X1);
    Register argv(X5);
    Register temp(X6);
    Register sp(SP);
    Register nativeCodeTemp(X2);

    __ Mov(nativeCodeTemp, nativeCode);

    __ Ldr(glue, MemoryOperand(sp, 0));
    __ Add(Register(X0), sp, Immediate(0));
    PushBuiltinFrame(assembler, glue, FrameType::BUILTIN_FRAME, temp, argv);

    CallNativeInternal(assembler, nativeCodeTemp);
    __ Ret();
}

void AsmInterpreterCall::PushBuiltinFrame(ExtendedAssembler *assembler, Register glue,
    FrameType type, Register op, Register next)
{
    Register sp(SP);
    __ PushFpAndLr();
    __ Mov(op, sp);
    __ Str(op, MemoryOperand(glue, JSThread::GlueData::GetLeaveFrameOffset(false)));
    __ Mov(op, Immediate(static_cast<int32_t>(type)));
    if (type == FrameType::BUILTIN_FRAME) {
        // push stack args
        __ Add(next, sp, Immediate(BuiltinFrame::GetStackArgsToFpDelta(false)));
        // 16: type & next
        __ Stp(next, op, MemoryOperand(sp, -16, AddrMode::PREINDEX));
        __ Add(Register(FP), sp, Immediate(16));  // 16: skip next and frame type
    } else if (type == FrameType::BUILTIN_ENTRY_FRAME) {
        // 16: type & next
        __ Stp(next, op, MemoryOperand(sp, -16, AddrMode::PREINDEX));
        __ Add(Register(FP), sp, Immediate(16));  // 16: skip next and frame type
    } else {
        // 16: type & next
        __ Stp(next, op, MemoryOperand(sp, -16, AddrMode::PREINDEX));
    }
}

void AsmInterpreterCall::CallNativeInternal(ExtendedAssembler *assembler, Register nativeCode)
{
    __ Blr(nativeCode);
    // resume rsp
    __ Mov(Register(SP), Register(FP));
    __ RestoreFpAndLr();
}

// ResumeRspAndDispatch(uintptr_t glue, uintptr_t sp, uintptr_t pc, uintptr_t constantPool,
//     uint64_t profileTypeInfo, uint64_t acc, uint32_t hotnessCounter, size_t jumpSize)
// GHC calling convention
// X19 - glue
// FP  - sp
// X20 - pc
// X21 - constantPool
// X22 - profileTypeInfo
// X23 - acc
// X24 - hotnessCounter
// X25 - jumpSizeAfterCall
void AsmInterpreterCall::ResumeRspAndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(ResumeRspAndDispatch));

    Register glueRegister = __ GlueRegister();
    Register sp(FP);
    Register rsp(SP);
    Register pc(X20);
    Register jumpSizeRegister(X25);

    Register ret(X23);
    Register opcode(X6, W);
    Register temp(X7);
    Register bcStub(X7);
    Register fp(X8);

    int64_t fpOffset = static_cast<int64_t>(AsmInterpretedFrame::GetFpOffset(false))
        - static_cast<int64_t>(AsmInterpretedFrame::GetSize(false));
    int64_t spOffset = static_cast<int64_t>(AsmInterpretedFrame::GetBaseOffset(false))
        - static_cast<int64_t>(AsmInterpretedFrame::GetSize(false));
    ASSERT(fpOffset < 0);
    ASSERT(spOffset < 0);

    Label newObjectDynRangeReturn;
    Label dispatch;
    __ Ldur(fp, MemoryOperand(sp, fpOffset));  // store fp for temporary
    __ Cmp(jumpSizeRegister, Immediate(0));
    __ B(Condition::EQ, &newObjectDynRangeReturn);
    __ Ldur(sp, MemoryOperand(sp, spOffset));  // update sp

    __ Add(pc, pc, Operand(jumpSizeRegister, LSL, 0));
    __ Ldrb(opcode, MemoryOperand(pc, 0));
    __ Bind(&dispatch);
    {
        __ Mov(rsp, fp);  // resume rsp
        __ Add(bcStub, glueRegister, Operand(opcode, UXTW, SHIFT_OF_FRAMESLOT));
        __ Ldr(bcStub, MemoryOperand(bcStub, JSThread::GlueData::GetBCStubEntriesOffset(false)));
        __ Br(bcStub);
    }

    auto jumpSize = kungfu::AssemblerModule::GetJumpSizeFromJSCallMode(JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV);
    Label getHiddenThis;
    Label notUndefined;
    __ Bind(&newObjectDynRangeReturn);
    {
        __ Cmp(ret, Immediate(JSTaggedValue::VALUE_UNDEFINED));
        __ B(Condition::NE, &notUndefined);
        auto index = AsmInterpretedFrame::ReverseIndex::THIS_OBJECT_REVERSE_INDEX;
        auto thisOffset = index * 8;  // 8: byte size
        ASSERT(thisOffset < 0);
        __ Bind(&getHiddenThis);
        __ Ldur(sp, MemoryOperand(sp, spOffset));  // update sp
        __ Mov(rsp, fp);  // resume rsp
        __ Ldur(ret, MemoryOperand(rsp, thisOffset));  // update acc
        __ Add(pc, pc, Immediate(jumpSize));
        __ Ldrb(opcode, MemoryOperand(pc, 0));
        __ Add(bcStub, glueRegister, Operand(opcode, UXTW, SHIFT_OF_FRAMESLOT));
        __ Ldr(bcStub, MemoryOperand(bcStub, JSThread::GlueData::GetBCStubEntriesOffset(false)));
        __ Br(bcStub);
    }
    __ Bind(&notUndefined);
    {
        Label notEcmaObject;
        __ Mov(temp, Immediate(JSTaggedValue::TAG_HEAPOBJECT_MARK));
        __ And(temp, temp, ret);
        __ Cmp(temp, Immediate(0));
        __ B(Condition::NE, &notEcmaObject);
        // acc is heap object
        __ Ldr(temp, MemoryOperand(ret, 0)); // hclass
        __ Ldr(temp, MemoryOperand(temp, JSHClass::BIT_FIELD_OFFSET));
        __ And(temp.W(), temp.W(), LogicalImmediate::Create(0xFF, RegWSize));
        __ Cmp(temp.W(), Immediate(static_cast<int64_t>(JSType::ECMA_OBJECT_END)));
        __ B(Condition::HI, &notEcmaObject);
        __ Cmp(temp.W(), Immediate(static_cast<int64_t>(JSType::ECMA_OBJECT_BEGIN)));
        __ B(Condition::LO, &notEcmaObject);
        // acc is ecma object
        __ Ldur(sp, MemoryOperand(sp, spOffset));  // update sp
        __ Add(pc, pc, Immediate(jumpSize));
        __ Ldrb(opcode, MemoryOperand(pc, 0));
        __ B(&dispatch);

        __ Bind(&notEcmaObject);
        {
            int64_t constructorOffset = static_cast<int64_t>(AsmInterpretedFrame::GetFunctionOffset(false))
                - static_cast<int64_t>(AsmInterpretedFrame::GetSize(false));
            ASSERT(constructorOffset < 0);
            __ Ldur(temp, MemoryOperand(sp, constructorOffset));  // load constructor
            __ Ldr(temp, MemoryOperand(temp, JSFunction::BIT_FIELD_OFFSET));
            __ Lsr(temp.W(), temp.W(), JSFunction::FunctionKindBits::START_BIT);
            __ And(temp.W(), temp.W(),
                LogicalImmediate::Create((1LU << JSFunction::FunctionKindBits::SIZE) - 1, RegWSize));
            __ Cmp(temp.W(), Immediate(static_cast<int64_t>(FunctionKind::CLASS_CONSTRUCTOR)));
            __ B(Condition::LS, &getHiddenThis);  // constructor is base
            // exception branch
            {
                __ Mov(opcode, kungfu::BytecodeStubCSigns::ID_NewObjectDynRangeThrowException);
                __ Ldur(sp, MemoryOperand(sp, spOffset));  // update sp
                __ B(&dispatch);
            }
        }
    }
}

// ResumeRspAndReturn(uintptr_t acc)
// GHC calling convention
// X19 - acc
// FP - prevSp
// X20 - sp
void AsmInterpreterCall::ResumeRspAndReturn(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(ResumeRspAndReturn));
    Register rsp(SP);
    Register currentSp(X20);

    [[maybe_unused]] TempRegister1Scope scope1(assembler);
    Register fpRegister = __ TempRegister1();
    int64_t offset = static_cast<int64_t>(AsmInterpretedFrame::GetFpOffset(false))
        - static_cast<int64_t>(AsmInterpretedFrame::GetSize(false));
    ASSERT(offset < 0);
    __ Ldur(fpRegister, MemoryOperand(currentSp, offset));
    __ Mov(rsp, fpRegister);

    // return
    {
        __ RestoreFpAndLr();
        __ Mov(Register(X0), Register(X19));
        __ Ret();
    }
}

// ResumeCaughtFrameAndDispatch(uintptr_t glue, uintptr_t sp, uintptr_t pc, uintptr_t constantPool,
//     uint64_t profileTypeInfo, uint64_t acc, uint32_t hotnessCounter)
// GHC calling convention
// X19 - glue
// FP  - sp
// X20 - pc
// X21 - constantPool
// X22 - profileTypeInfo
// X23 - acc
// X24 - hotnessCounter
void AsmInterpreterCall::ResumeCaughtFrameAndDispatch(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(ResumeCaughtFrameAndDispatch));

    Register glue(X19);
    Register pc(X20);
    Register fp(X5);
    Register opcode(X6, W);
    Register bcStub(X7);

    Label dispatch;
    __ Ldr(fp, MemoryOperand(glue, JSThread::GlueData::GetLastFpOffset(false)));
    __ Cmp(fp, Immediate(0));
    __ B(Condition::EQ, &dispatch);
    // up frame
    __ Mov(Register(SP), fp);
    // fall through
    __ Bind(&dispatch);
    {
        __ Ldrb(opcode, MemoryOperand(pc, 0));
        __ Add(bcStub, glue, Operand(opcode, UXTW, SHIFT_OF_FRAMESLOT));
        __ Ldr(bcStub, MemoryOperand(bcStub, JSThread::GlueData::GetBCStubEntriesOffset(false)));
        __ Br(bcStub);
    }
}

// ResumeUncaughtFrameAndReturn(uintptr_t glue)
// GHC calling convention
// X19 - glue
// FP - sp
// X20 - acc
void AsmInterpreterCall::ResumeUncaughtFrameAndReturn(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(ResumeUncaughtFrameAndReturn));

    Register glue(X19);
    Register fp(X5);
    Register acc(X20);
    Register cppRet(X0);

    __ Ldr(fp, MemoryOperand(glue, JSThread::GlueData::GetLastFpOffset(false)));
    __ Mov(Register(SP), fp);
    // this method will return to Execute(cpp calling convention), and the return value should be put into X0.
    __ Mov(cppRet, acc);
    __ RestoreFpAndLr();
    __ Ret();
}

// c++ calling convention
// X0 - glue
// X1 - callTarget
// X2 - method
// X3 - callField
// X4 - receiver
// X5 - value
void AsmInterpreterCall::CallGetter(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(CallGetter));
    Label target;

    PushAsmInterpBridgeFrame(assembler);
    __ Bl(&target);
    PopAsmInterpBridgeFrame(assembler);
    __ Ret();
    __ Bind(&target);
    {
        JSCallCommonEntry(assembler, JSCallMode::CALL_GETTER);
    }
}

void AsmInterpreterCall::CallSetter(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(CallSetter));
    Label target;
    PushAsmInterpBridgeFrame(assembler);
    __ Bl(&target);
    PopAsmInterpBridgeFrame(assembler);
    __ Ret();
    __ Bind(&target);
    {
        JSCallCommonEntry(assembler, JSCallMode::CALL_SETTER);
    }
}

// Generate code for generator re-entering asm interpreter
// c++ calling convention
// Input: %X0 - glue
//        %X1 - context(GeneratorContext)
void AsmInterpreterCall::GeneratorReEnterAsmInterp(ExtendedAssembler *assembler)
{
    __ BindAssemblerStub(RTSTUB_ID(GeneratorReEnterAsmInterp));
    Label target;
    PushAsmInterpEntryFrame(assembler);
    __ Bl(&target);
    PopAsmInterpEntryFrame(assembler);
    __ Ret();
    __ Bind(&target);
    {
        GeneratorReEnterAsmInterpDispatch(assembler);
    }
}

void AsmInterpreterCall::GeneratorReEnterAsmInterpDispatch(ExtendedAssembler *assembler)
{
    Label pushFrameState;
    Label stackOverflow;
    Register glue = __ GlueRegister();
    Register contextRegister(X1);
    Register spRegister(SP);
    Register pc(X8);
    Register prevSpRegister(FP);
    Register callTarget(X4);
    Register method(X5);
    Register temp(X6); // can not be used to store any variable
    Register currentSlotRegister(X7);
    Register fpRegister(X9);
    Register nRegsRegister(X26, W);
    Register regsArrayRegister(X27);
    Register newSp(X28);
    __ Ldr(callTarget, MemoryOperand(contextRegister, GeneratorContext::GENERATOR_METHOD_OFFSET));
    __ Ldr(method, MemoryOperand(callTarget, JSFunctionBase::METHOD_OFFSET));
    __ PushFpAndLr();
    // save fp
    __ Mov(fpRegister, spRegister);
    __ Mov(currentSlotRegister, spRegister);
    // Reserve enough sp space to prevent stack parameters from being covered by cpu profiler.
    __ Ldr(temp, MemoryOperand(glue, JSThread::GlueData::GetStackLimitOffset(false)));
    __ Mov(Register(SP), temp);
    // push context regs
    __ Ldr(nRegsRegister, MemoryOperand(contextRegister, GeneratorContext::GENERATOR_NREGS_OFFSET));
    __ Ldr(regsArrayRegister, MemoryOperand(contextRegister, GeneratorContext::GENERATOR_REGS_ARRAY_OFFSET));
    __ Add(regsArrayRegister, regsArrayRegister, Immediate(TaggedArray::DATA_OFFSET));
    PushArgsWithArgv(assembler, glue, nRegsRegister, regsArrayRegister, temp, currentSlotRegister, &pushFrameState,
        &stackOverflow);

    __ Bind(&pushFrameState);
    __ Mov(newSp, currentSlotRegister);
    // push frame state
    PushGeneratorFrameState(
        assembler, prevSpRegister, fpRegister, currentSlotRegister, callTarget, method, contextRegister, pc, temp);
    __ Align16(currentSlotRegister);
    __ Mov(Register(SP), currentSlotRegister);
    // call bc stub
    CallBCStub(assembler, newSp, glue, callTarget, method, pc, temp);

    __ Bind(&stackOverflow);
    {
        ThrowStackOverflowExceptionAndReturn(assembler, glue, fpRegister, temp);
    }
}

void AsmInterpreterCall::PushCallThis(ExtendedAssembler *assembler, JSCallMode mode, Label *stackOverflow)
{
    Register callFieldRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_FIELD);
    Register callTargetRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_TARGET);
    Register currentSlotRegister = __ AvailableRegister3();

    Label pushVregs;
    Label pushNewTarget;
    Label pushCallTarget;
    bool haveThis = kungfu::AssemblerModule::JSModeHaveThisArg(mode);
    bool haveNewTarget = kungfu::AssemblerModule::JSModeHaveNewTargetArg(mode);
    if (!haveThis) {
        __ Tst(callFieldRegister, LogicalImmediate::Create(CALL_TYPE_MASK, RegXSize));
        __ B(Condition::EQ, &pushVregs);
    }
    __ Tbz(callFieldRegister, JSMethod::HaveThisBit::START_BIT, &pushNewTarget);
    if (!haveThis) {
        [[maybe_unused]] TempRegister1Scope scope1(assembler);
        Register tempRegister = __ TempRegister1();
        __ Mov(tempRegister, Immediate(JSTaggedValue::VALUE_UNDEFINED));
        __ Str(tempRegister, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    } else {
        Register thisRegister = GetThisRegsiter(assembler, mode);
        __ Str(thisRegister, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    }
    __ Bind(&pushNewTarget);
    {
        __ Tbz(callFieldRegister, JSMethod::HaveNewTargetBit::START_BIT, &pushCallTarget);
        if (!haveNewTarget) {
            [[maybe_unused]] TempRegister1Scope scope1(assembler);
            Register newTarget = __ TempRegister1();
            __ Mov(newTarget, Immediate(JSTaggedValue::VALUE_UNDEFINED));
            __ Str(newTarget, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
        } else {
            Register newTargetRegister = GetNewTargetRegsiter(assembler, mode);
            __ Str(newTargetRegister, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
        }
    }
    __ Bind(&pushCallTarget);
    {
        __ Tbz(callFieldRegister, JSMethod::HaveFuncBit::START_BIT, &pushVregs);
        __ Str(callTargetRegister, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    }
    __ Bind(&pushVregs);
    {
        PushVregs(assembler, stackOverflow);
    }
}

void AsmInterpreterCall::PushVregs(ExtendedAssembler *assembler, Label *stackOverflow)
{
    Register glue = __ GlueRegister();
    Register prevSpRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::SP);
    Register callTargetRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_TARGET);
    Register methodRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::METHOD);
    Register callFieldRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_FIELD);
    Register fpRegister = __ AvailableRegister1();
    Register currentSlotRegister = __ AvailableRegister3();

    Label pushFrameStateAndCall;
    [[maybe_unused]] TempRegister1Scope scope1(assembler);
    Register tempRegister = __ TempRegister1();
    // args register can be reused now.
    Register numVregsRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::ARG0);
    GetNumVregsFromCallField(assembler, callFieldRegister, numVregsRegister);
    PushUndefinedWithArgc(assembler, glue, numVregsRegister, tempRegister, currentSlotRegister, &pushFrameStateAndCall,
        stackOverflow);
    // fall through
    __ Bind(&pushFrameStateAndCall);
    {
        Register newSpRegister = __ AvailableRegister2();
        __ Mov(newSpRegister, currentSlotRegister);

        [[maybe_unused]] TempRegister2Scope scope2(assembler);
        Register pcRegister = __ TempRegister2();
        PushFrameState(assembler, prevSpRegister, fpRegister, currentSlotRegister, callTargetRegister, methodRegister,
            pcRegister, tempRegister);

        __ Align16(currentSlotRegister);
        __ Mov(Register(SP), currentSlotRegister);
        DispatchCall(assembler, pcRegister, newSpRegister);
    }
}

// Input: X19 - glue
//        FP - sp
//        X20 - callTarget
//        X21 - method
void AsmInterpreterCall::DispatchCall(ExtendedAssembler *assembler, Register pcRegister, Register newSpRegister)
{
    Register glueRegister = __ GlueRegister();
    Register callTargetRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::CALL_TARGET);
    Register methodRegister = __ CallDispatcherArgument(kungfu::CallDispatchInputs::METHOD);

    if (glueRegister.GetId() != X19) {
        __ Mov(Register(X19), glueRegister);
    }
    __ Ldrh(Register(X24, W), MemoryOperand(methodRegister, JSMethod::GetHotnessCounterOffset(false)));
    __ Mov(Register(X23), Immediate(JSTaggedValue::VALUE_HOLE));
    __ Ldr(Register(X22), MemoryOperand(callTargetRegister, JSFunction::PROFILE_TYPE_INFO_OFFSET));
    __ Ldr(Register(X21), MemoryOperand(callTargetRegister, JSFunction::CONSTANT_POOL_OFFSET));
    __ Mov(Register(X20), pcRegister);
    __ Mov(Register(FP), newSpRegister);

    Register bcIndexRegister = __ AvailableRegister1();
    Register tempRegister = __ AvailableRegister2();
    __ Ldrb(bcIndexRegister.W(), MemoryOperand(pcRegister, 0));
    __ Add(tempRegister, glueRegister, Operand(bcIndexRegister.W(), UXTW, SHIFT_OF_FRAMESLOT));
    __ Ldr(tempRegister, MemoryOperand(tempRegister, JSThread::GlueData::GetBCStubEntriesOffset(false)));
    __ Br(tempRegister);
}

void AsmInterpreterCall::PushFrameState(ExtendedAssembler *assembler, Register prevSp, Register fp, Register currentSlot,
    Register callTarget, Register method, Register pc, Register op)
{
    __ Mov(op, Immediate(static_cast<int32_t>(FrameType::ASM_INTERPRETER_FRAME)));
    __ Stp(prevSp, op, MemoryOperand(currentSlot, -16, AddrMode::PREINDEX));            // -16: frame type & prevSp
    __ Ldr(pc, MemoryOperand(method, JSMethod::GetBytecodeArrayOffset(false)));
    __ Stp(fp, pc, MemoryOperand(currentSlot, -16, AddrMode::PREINDEX));                // -16: pc & fp
    __ Ldr(op, MemoryOperand(callTarget, JSFunction::LEXICAL_ENV_OFFSET));
    __ Stp(op, Register(Zero), MemoryOperand(currentSlot, -16, AddrMode::PREINDEX));    // -16: jumpSizeAfterCall & env
    __ Mov(op, Immediate(JSTaggedValue::VALUE_HOLE));
    __ Stp(callTarget, op, MemoryOperand(currentSlot, -16, AddrMode::PREINDEX));        // -16: acc & callTarget
}

void AsmInterpreterCall::GetNumVregsFromCallField(ExtendedAssembler *assembler, Register callField, Register numVregs)
{
    __ Mov(numVregs, callField);
    __ Lsr(numVregs, numVregs, JSMethod::NumVregsBits::START_BIT);
    __ And(numVregs.W(), numVregs.W(),
        LogicalImmediate::Create(JSMethod::NumVregsBits::Mask() >> JSMethod::NumVregsBits::START_BIT, RegWSize));
}

void AsmInterpreterCall::GetDeclaredNumArgsFromCallField(ExtendedAssembler *assembler, Register callField,
    Register declaredNumArgs)
{
    __ Mov(declaredNumArgs, callField);
    __ Lsr(declaredNumArgs, declaredNumArgs, JSMethod::NumArgsBits::START_BIT);
    __ And(declaredNumArgs.W(), declaredNumArgs.W(),
        LogicalImmediate::Create(JSMethod::NumArgsBits::Mask() >> JSMethod::NumArgsBits::START_BIT, RegWSize));
}

void AsmInterpreterCall::PushAsmInterpEntryFrame(ExtendedAssembler *assembler)
{
    Register glue = __ GlueRegister();
    Register fp(X29);
    Register sp(SP);

    if (!assembler->FromInterpreterHandler()) {
        __ CalleeSave();
    }

    [[maybe_unused]] TempRegister1Scope scope1(assembler);
    Register prevFrameRegister = __ TempRegister1();
    [[maybe_unused]] TempRegister2Scope scope2(assembler);
    Register frameTypeRegister = __ TempRegister2();

    __ PushFpAndLr();

    // prev managed fp is leave frame or nullptr(the first frame)
    __ Ldr(prevFrameRegister, MemoryOperand(glue, JSThread::GlueData::GetLeaveFrameOffset(false)));
    __ Mov(frameTypeRegister, Immediate(static_cast<int64_t>(FrameType::ASM_INTERPRETER_ENTRY_FRAME)));
    // 2 : prevSp & frame type
    __ Stp(prevFrameRegister, frameTypeRegister, MemoryOperand(sp, -2 * FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    // 2 : pc & glue
    __ Stp(glue, Register(Zero), MemoryOperand(sp, -2 * FRAME_SLOT_SIZE, AddrMode::PREINDEX));  // pc
    __ Add(fp, sp, Immediate(32));  // 32: skip frame type, prevSp, pc and glue
}

void AsmInterpreterCall::PopAsmInterpEntryFrame(ExtendedAssembler *assembler)
{
    Register sp(SP);

    [[maybe_unused]] TempRegister1Scope scope1(assembler);
    Register prevFrameRegister = __ TempRegister1();
    [[maybe_unused]] TempRegister2Scope scope2(assembler);
    Register glue = __ TempRegister2();
    // 2: glue & pc
    __ Ldp(glue, Register(Zero), MemoryOperand(sp, 2 * FRAME_SLOT_SIZE, AddrMode::POSTINDEX));
    // 2: skip frame type & prev
    __ Ldp(prevFrameRegister, Register(Zero), MemoryOperand(sp, 2 * FRAME_SLOT_SIZE, AddrMode::POSTINDEX));

    __ RestoreFpAndLr();
    __ Str(prevFrameRegister, MemoryOperand(glue, JSThread::GlueData::GetLeaveFrameOffset(false)));
    if (!assembler->FromInterpreterHandler()) {
        __ CalleeRestore();
    }
}

void AsmInterpreterCall::PushGeneratorFrameState(ExtendedAssembler *assembler, Register &prevSpRegister,
    Register &fpRegister, Register &currentSlotRegister, Register &callTargetRegister, Register &methodRegister,
    Register &contextRegister, Register &pcRegister, Register &operatorRegister)
{
    __ Mov(operatorRegister, Immediate(static_cast<int64_t>(FrameType::ASM_INTERPRETER_FRAME)));
    __ Stp(prevSpRegister, operatorRegister,
        MemoryOperand(currentSlotRegister, -2 * FRAME_SLOT_SIZE, AddrMode::PREINDEX));  // 2 : frameType and prevSp
    __ Ldr(pcRegister, MemoryOperand(methodRegister, JSMethod::GetBytecodeArrayOffset(false)));
    // offset need 8 align, GENERATOR_NREGS_OFFSET instead of GENERATOR_BC_OFFSET_OFFSET
    __ Ldr(operatorRegister, MemoryOperand(contextRegister, GeneratorContext::GENERATOR_NREGS_OFFSET));
    // 32: get high 32bit
    __ Lsr(operatorRegister, operatorRegister, 32);
    __ Add(pcRegister, operatorRegister, pcRegister);
    __ Add(pcRegister, pcRegister, Immediate(BytecodeInstruction::Size(BytecodeInstruction::Format::PREF_V8_V8)));
    // 2 : pc and fp
    __ Stp(fpRegister, pcRegister, MemoryOperand(currentSlotRegister, -2 * FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    // jumpSizeAfterCall
    __ Str(Register(Zero), MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    __ Ldr(operatorRegister, MemoryOperand(contextRegister, GeneratorContext::GENERATOR_LEXICALENV_OFFSET));
    // env
    __ Str(operatorRegister, MemoryOperand(currentSlotRegister, -FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    __ Ldr(operatorRegister, MemoryOperand(contextRegister, GeneratorContext::GENERATOR_ACC_OFFSET));
    __ Stp(callTargetRegister, operatorRegister,
        MemoryOperand(currentSlotRegister, -2 * FRAME_SLOT_SIZE, AddrMode::PREINDEX));  // 2 : acc and callTarget
}

void AsmInterpreterCall::CallBCStub(ExtendedAssembler *assembler, Register &newSp, Register &glue,
    Register &callTarget, Register &method, Register &pc, Register &temp)
{
    // prepare call entry
    __ Mov(Register(X19), glue);    // X19 - glue
    __ Mov(Register(FP), newSp);    // FP - sp
    __ Mov(Register(X20), pc);      // X20 - pc
    __ Ldr(Register(X21), MemoryOperand(callTarget, JSFunction::CONSTANT_POOL_OFFSET));     // X21 - constantpool
    __ Ldr(Register(X22), MemoryOperand(callTarget, JSFunction::PROFILE_TYPE_INFO_OFFSET)); // X22 - profileTypeInfo
    __ Mov(Register(X23), Immediate(JSTaggedValue::Hole().GetRawData()));                   // X23 - acc
    __ Ldr(Register(X24), MemoryOperand(method, JSMethod::GetHotnessCounterOffset(false))); // X24 - hotnessCounter

    // call the first bytecode handler
    __ Ldrb(temp.W(), MemoryOperand(pc, 0));
    // 3 : 3 means *8
    __ Add(temp, glue, Operand(temp.W(), UXTW, SHIFT_OF_FRAMESLOT));
    __ Ldr(temp, MemoryOperand(temp, JSThread::GlueData::GetBCStubEntriesOffset(false)));
    __ Br(temp);
}

void AsmInterpreterCall::CallNativeEntry(ExtendedAssembler *assembler)
{
    Register glue(X0);
    Register argv(X5);
    Register method(X2);
    Register function(X1);
    Register nativeCode(X7);
    Register temp(X9);

    Register sp(SP);
    // 2: function & align
    __ Stp(function, Register(Zero), MemoryOperand(sp, -2 * FRAME_SLOT_SIZE, AddrMode::PREINDEX));
    // 2: skip argc & thread
    __ Sub(sp, sp, Immediate(2 * FRAME_SLOT_SIZE));
    PushBuiltinFrame(assembler, glue, FrameType::BUILTIN_ENTRY_FRAME, temp, argv);
    // get native pointer
    __ Ldr(nativeCode, MemoryOperand(method, JSMethod::GetBytecodeArrayOffset(false)));
    __ Mov(temp, argv);
    __ Sub(Register(X0), temp, Immediate(2 * FRAME_SLOT_SIZE));  // 2: skip argc & thread
    CallNativeInternal(assembler, nativeCode);

    // 4: skip function
    __ Add(sp, sp, Immediate(4 * FRAME_SLOT_SIZE));
    __ Ret();
}

void AsmInterpreterCall::ThrowStackOverflowExceptionAndReturn(ExtendedAssembler *assembler, Register glue,
    Register fp, Register op)
{
    __ Mov(Register(SP), fp);
    __ Mov(op, Immediate(kungfu::RuntimeStubCSigns::ID_ThrowStackOverflowException));
    // 3 : 3 means *8
    __ Add(op, glue, Operand(op, LSL, 3));
    __ Ldr(op, MemoryOperand(op, JSThread::GlueData::GetRTStubEntriesOffset(false)));
    if (glue.GetId() != X0) {
        __ Mov(Register(X0), glue);
    }
    __ Blr(op);
    __ RestoreFpAndLr();
    __ Ret();
}
#undef __
}  // panda::ecmascript::aarch64