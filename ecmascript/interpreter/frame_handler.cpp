
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

#include "ecmascript/interpreter/frame_handler.h"

#include "ecmascript/file_loader.h"
#include "ecmascript/llvm_stackmap_parser.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/mem/heap.h"

#include "libpandafile/bytecode_instruction-inl.h"

namespace panda::ecmascript {
ARK_INLINE void FrameHandler::AdvanceToInterpretedFrame()
{
    if (!thread_->IsAsmInterpreter()) {
        return;
    }
    FrameIterator it(sp_, thread_);
    for (; !it.Done(); it.Advance()) {
        FrameType t = it.GetFrameType();
        if (IsInterpretedFrame(t) || IsInterpretedEntryFrame(t)) {
            break;
        }
    }
    sp_ = it.GetSp();
}

ARK_INLINE void FrameHandler::PrevInterpretedFrame()
{
    if (!thread_->IsAsmInterpreter()) {
        FrameIterator it(sp_, thread_);
        it.Advance();
        sp_ = it.GetSp();
        return;
    }
    AdvanceToInterpretedFrame();
    FrameIterator it(sp_, thread_);
    FrameType t = it.GetFrameType();
    if (t == FrameType::ASM_INTERPRETER_FRAME) {
        auto frame = it.GetFrame<AsmInterpretedFrame>();
        if (thread_->IsAsmInterpreter()) {
            fp_ = frame->GetCurrentFramePointer();
        }
    }
    it.Advance();
    sp_ = it.GetSp();
    AdvanceToInterpretedFrame();
}

JSTaggedType* FrameHandler::GetPrevInterpretedFrame()
{
    PrevInterpretedFrame();
    return GetSp();
}

uint32_t FrameHandler::GetNumberArgs()
{
    if (thread_->IsAsmInterpreter()) {
        auto *frame = AsmInterpretedFrame::GetFrameFromSp(sp_);
        return static_cast<uint32_t>(frame->GetCurrentFramePointer() - sp_);
    }
    ASSERT(IsInterpretedFrame());
    JSTaggedType *prevSp = nullptr;
    if (IsAsmInterpretedFrame()) {
        auto *frame = AsmInterpretedFrame::GetFrameFromSp(sp_);
        prevSp = frame->GetPrevFrameFp();
    } else {
        auto *frame = InterpretedFrame::GetFrameFromSp(sp_);
        prevSp = frame->GetPrevFrameFp();
    }
    auto prevSpEnd = reinterpret_cast<JSTaggedType*>(GetInterpretedFrameEnd(prevSp));
    return static_cast<uint32_t>(prevSpEnd - sp_);
}

JSTaggedValue FrameHandler::GetVRegValue(size_t index) const
{
    ASSERT(IsInterpretedFrame());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return JSTaggedValue(sp_[index]);
}

void FrameHandler::SetVRegValue(size_t index, JSTaggedValue value)
{
    ASSERT(IsInterpretedFrame());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sp_[index] = value.GetRawData();
}

JSTaggedValue FrameHandler::GetAcc() const
{
    ASSERT(IsInterpretedFrame());
    if (IsAsmInterpretedFrame()) {
        auto *frame = AsmInterpretedFrame::GetFrameFromSp(sp_);
        return frame->acc;
    } else {
        auto *frame = InterpretedFrame::GetFrameFromSp(sp_);
        return frame->acc;
    }
}

uint32_t FrameHandler::GetBytecodeOffset() const
{
    ASSERT(IsInterpretedFrame());
    JSMethod *method = GetMethod();
    auto offset = GetPc() - method->GetBytecodeArray();
    return static_cast<uint32_t>(offset);
}

JSMethod *FrameHandler::GetMethod() const
{
    ASSERT(IsInterpretedFrame());
    auto function = GetFunction();
    return ECMAObject::Cast(function.GetTaggedObject())->GetCallTarget();
}

JSMethod *FrameHandler::CheckAndGetMethod() const
{
    ASSERT(IsInterpretedFrame());
    auto function = GetFunction();
    if (function.IsJSFunctionBase() || function.IsJSProxy()) {
        return ECMAObject::Cast(function.GetTaggedObject())->GetCallTarget();
    }
    return nullptr;
}

JSTaggedValue FrameHandler::GetFunction() const
{
    ASSERT(IsInterpretedFrame());
    if (thread_->IsAsmInterpreter()) {
        FrameType type = GetFrameType();
        switch (type) {
            case FrameType::ASM_INTERPRETER_FRAME:
            case FrameType::INTERPRETER_CONSTRUCTOR_FRAME: {
                auto frame = AsmInterpretedFrame::GetFrameFromSp(sp_);
                return frame->function;
            }
            case FrameType::BUILTIN_FRAME_WITH_ARGV: {
                auto *frame = BuiltinWithArgvFrame::GetFrameFromSp(sp_);
                return frame->GetFunction();
            }
            case FrameType::BUILTIN_ENTRY_FRAME:
            case FrameType::BUILTIN_FRAME: {
                auto *frame = BuiltinFrame::GetFrameFromSp(sp_);
                return frame->GetFunction();
            }
            case FrameType::INTERPRETER_FRAME:
            case FrameType::INTERPRETER_FAST_NEW_FRAME:
            case FrameType::INTERPRETER_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_BRIDGE_FRAME:
            case FrameType::OPTIMIZED_FRAME:
            case FrameType::LEAVE_FRAME:
            case FrameType::LEAVE_FRAME_WITH_ARGV:
            case FrameType::OPTIMIZED_ENTRY_FRAME:
            default: {
                LOG_ECMA(FATAL) << "frame type error!";
                UNREACHABLE();
            }
        }
    } else {
        auto *frame = InterpretedFrame::GetFrameFromSp(sp_);
        return frame->function;
    }
}

const uint8_t *FrameHandler::GetPc() const
{
    ASSERT(IsInterpretedFrame());
    if (IsAsmInterpretedFrame()) {
        auto *frame = AsmInterpretedFrame::GetFrameFromSp(sp_);
        return frame->pc;
    } else {
        auto *frame = InterpretedFrame::GetFrameFromSp(sp_);
        return frame->pc;
    }
}

ConstantPool *FrameHandler::GetConstpool() const
{
    ASSERT(IsInterpretedFrame());
    auto function = GetFunction();
    JSTaggedValue constpool = JSFunction::Cast(function.GetTaggedObject())->GetConstantPool();
    return ConstantPool::Cast(constpool.GetTaggedObject());
}

JSTaggedValue FrameHandler::GetEnv() const
{
    ASSERT(IsInterpretedFrame());
    if (IsAsmInterpretedFrame()) {
        auto *frame = AsmInterpretedFrame::GetFrameFromSp(sp_);
        return frame->env;
    } else {
        auto *frame = InterpretedFrame::GetFrameFromSp(sp_);
        return frame->env;
    }
}

void FrameHandler::DumpStack(std::ostream &os) const
{
    size_t i = 0;
    FrameHandler frameHandler(thread_);
    for (; frameHandler.HasFrame(); frameHandler.PrevInterpretedFrame()) {
        os << "[" << i++
        << "]:" << frameHandler.GetMethod()->ParseFunctionName()
        << "\n";
    }
}

void FrameHandler::DumpPC(std::ostream &os, const uint8_t *pc) const
{
    FrameHandler frameHandler(thread_);
    ASSERT(frameHandler.HasFrame());
    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
    int offset = pc - frameHandler.GetMethod()->GetBytecodeArray();
    os << "offset: " << offset << "\n";
}

ARK_INLINE uintptr_t FrameHandler::GetInterpretedFrameEnd(JSTaggedType *prevSp) const
{
    uintptr_t end = 0U;
    FrameType type = FrameHandler::GetFrameType(prevSp);
    switch (type) {
        case FrameType::ASM_INTERPRETER_FRAME:
        case FrameType::INTERPRETER_CONSTRUCTOR_FRAME: {
            auto frame = AsmInterpretedFrame::GetFrameFromSp(prevSp);
            end = ToUintPtr(frame);
            break;
        }
        case FrameType::INTERPRETER_FRAME:
        case FrameType::INTERPRETER_FAST_NEW_FRAME: {
            auto frame = InterpretedFrame::GetFrameFromSp(prevSp);
            end = ToUintPtr(frame);
            break;
        }
        case FrameType::INTERPRETER_ENTRY_FRAME:
            end = ToUintPtr(GetInterpretedEntryFrameStart(prevSp));
            break;
        case FrameType::BUILTIN_FRAME_WITH_ARGV:
        case FrameType::BUILTIN_ENTRY_FRAME:
        case FrameType::BUILTIN_FRAME:
        case FrameType::OPTIMIZED_FRAME:
        case FrameType::LEAVE_FRAME:
        case FrameType::LEAVE_FRAME_WITH_ARGV:
        case FrameType::OPTIMIZED_ENTRY_FRAME:
        case FrameType::ASM_INTERPRETER_ENTRY_FRAME:
        case FrameType::ASM_INTERPRETER_BRIDGE_FRAME:
        default: {
            LOG_ECMA(FATAL) << "frame type error!";
            UNREACHABLE();
        }
    }
    return end;
}

ARK_INLINE JSTaggedType *FrameHandler::GetInterpretedEntryFrameStart(const JSTaggedType *sp)
{
    ASSERT(FrameHandler::GetFrameType(sp) == FrameType::INTERPRETER_ENTRY_FRAME);
    JSTaggedType *argcSp = const_cast<JSTaggedType *>(sp) - INTERPRETER_ENTRY_FRAME_STATE_SIZE - 1;
    uint32_t argc = argcSp[0];
    return argcSp - argc - RESERVED_CALL_ARGCOUNT;
}

void FrameHandler::IterateRsp(const RootVisitor &v0, const RootRangeVisitor &v1)
{
    JSTaggedType *current = const_cast<JSTaggedType *>(thread_->GetLastLeaveFrame());
    IterateFrameChain(current, v0, v1);
}

void FrameHandler::IterateSp(const RootVisitor &v0, const RootRangeVisitor &v1)
{
    JSTaggedType *current = const_cast<JSTaggedType *>(thread_->GetCurrentSPFrame());
    while (current) {
        // only interpreter entry frame is on thread sp when rsp is enable
        ASSERT(FrameHandler::GetFrameType(current) == FrameType::INTERPRETER_ENTRY_FRAME);
        FrameIterator it(current, thread_);
        auto frame = it.GetFrame<InterpretedEntryFrame>();
        frame->GCIterate(it, v0, v1);
        it.Advance();
        current = it.GetSp();
    }
}

void FrameHandler::Iterate(const RootVisitor &v0, const RootRangeVisitor &v1)
{
    if (thread_->IsAsmInterpreter()) {
        IterateSp(v0, v1);
        IterateRsp(v0, v1);
        return;
    }
    JSTaggedType *current = const_cast<JSTaggedType *>(thread_->GetCurrentSPFrame());
    FrameType frameType = FrameHandler::GetFrameType(current);
    if (frameType != FrameType::INTERPRETER_ENTRY_FRAME) {
        auto leaveFrame = const_cast<JSTaggedType *>(thread_->GetLastLeaveFrame());
        if (leaveFrame != nullptr) {
            current = leaveFrame;
        }
    }
    IterateFrameChain(current, v0, v1);
}

void FrameHandler::IterateFrameChain(JSTaggedType *start, const RootVisitor &v0, const RootRangeVisitor &v1) const
{
    ChunkMap<DerivedDataKey, uintptr_t> *derivedPointers = thread_->GetEcmaVM()->GetHeap()->GetDerivedPointers();
    bool isVerifying = false;

#if ECMASCRIPT_ENABLE_HEAP_VERIFY
    isVerifying = thread_->GetEcmaVM()->GetHeap()->IsVerifying();
#endif
    JSTaggedType *current = start;
    for (FrameIterator it(current, thread_); !it.Done(); it.Advance()) {
        FrameType type = it.GetFrameType();
        switch (type) {
            case FrameType::OPTIMIZED_FRAME: {
                auto frame = it.GetFrame<OptimizedFrame>();
                frame->GCIterate(it, v0, v1, derivedPointers, isVerifying);
                break;
            }
            case FrameType::OPTIMIZED_JS_FUNCTION_FRAME: {
                auto frame = it.GetFrame<OptimizedJSFunctionFrame>();
                frame->GCIterate(it, v0, v1, derivedPointers, isVerifying);
                break;
            }
            case FrameType::ASM_INTERPRETER_FRAME:
            case FrameType::INTERPRETER_CONSTRUCTOR_FRAME: {
                auto frame = it.GetFrame<AsmInterpretedFrame>();
                frame->GCIterate(it, v0, v1, derivedPointers, isVerifying);
                break;
            }
            case FrameType::INTERPRETER_FRAME:
            case FrameType::INTERPRETER_FAST_NEW_FRAME: {
                auto frame = it.GetFrame<InterpretedFrame>();
                frame->GCIterate(it, v0, v1);
                break;
            }
            case FrameType::LEAVE_FRAME: {
                auto frame = it.GetFrame<OptimizedLeaveFrame>();
                frame->GCIterate(it, v0, v1, derivedPointers, isVerifying);
                break;
            }
            case FrameType::LEAVE_FRAME_WITH_ARGV: {
                auto frame = it.GetFrame<OptimizedWithArgvLeaveFrame>();
                frame->GCIterate(it, v0, v1, derivedPointers, isVerifying);
                break;
            }
            case FrameType::BUILTIN_FRAME_WITH_ARGV: {
                auto frame = it.GetFrame<BuiltinWithArgvFrame>();
                frame->GCIterate(it, v0, v1, derivedPointers, isVerifying);
                break;
            }
            case FrameType::BUILTIN_ENTRY_FRAME:
            case FrameType::BUILTIN_FRAME: {
                auto frame = it.GetFrame<BuiltinFrame>();
                frame->GCIterate(it, v0, v1, derivedPointers, isVerifying);
                break;
            }
            case FrameType::INTERPRETER_ENTRY_FRAME: {
                auto frame = it.GetFrame<InterpretedEntryFrame>();
                frame->GCIterate(it, v0, v1);
                break;
            }
            case FrameType::OPTIMIZED_JS_FUNCTION_ARGS_CONFIG_FRAME:
            case FrameType::OPTIMIZED_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_BRIDGE_FRAME: {
                break;
            }
            default: {
                LOG_ECMA(FATAL) << "frame type error!";
                UNREACHABLE();
            }
        }
    }
}

std::string FrameHandler::GetAotExceptionFuncName(JSTaggedType* fp) const
{
    ASSERT(FrameHandler::GetFrameType(fp) == FrameType::OPTIMIZED_JS_FUNCTION_FRAME);
    JSTaggedValue func = JSTaggedValue(*(fp + 3)); // 3: skip returnaddr and argc
    JSMethod *method = JSFunction::Cast(func.GetTaggedObject())->GetMethod();
    return method->GetMethodName();
}

void FrameHandler::CollectBCOffsetInfo()
{
    thread_->GetEcmaVM()->ClearExceptionBCList();
    JSTaggedType *current = const_cast<JSTaggedType *>(thread_->GetLastLeaveFrame());
    for (FrameIterator it(current, thread_); !it.Done(); it.Advance()) {
        FrameType type = it.GetFrameType();
        switch (type) {
            case FrameType::OPTIMIZED_JS_FUNCTION_ARGS_CONFIG_FRAME:
            case FrameType::OPTIMIZED_JS_FUNCTION_FRAME: {
                auto frame = it.GetFrame<OptimizedJSFunctionFrame>();
                auto returnAddr = frame->GetReturnAddr();
                auto constInfo = thread_->GetEcmaVM()->GetFileLoader()->GetStackMapParser()->GetConstInfo(returnAddr);
                if (!constInfo.empty()) {
                    auto prevFp = frame->GetPrevFrameFp();
                    auto name = GetAotExceptionFuncName(prevFp);
                    thread_->GetEcmaVM()->StoreBCOffsetInfo(name, constInfo[0]);
                }
                break;
            }
            case FrameType::LEAVE_FRAME: {
                auto frame = it.GetFrame<OptimizedLeaveFrame>();
                auto returnAddr = frame->GetReturnAddr();
                auto constInfo = thread_->GetEcmaVM()->GetFileLoader()->GetStackMapParser()->GetConstInfo(returnAddr);
                if (!constInfo.empty()) {
                    auto prevFp = frame->GetPrevFrameFp();
                    auto name = GetAotExceptionFuncName(prevFp);
                    thread_->GetEcmaVM()->StoreBCOffsetInfo(name, constInfo[0]);
                }
                break;
            }
            case FrameType::OPTIMIZED_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_FRAME:
            case FrameType::INTERPRETER_CONSTRUCTOR_FRAME:
            case FrameType::INTERPRETER_FRAME:
            case FrameType::INTERPRETER_FAST_NEW_FRAME:
            case FrameType::OPTIMIZED_FRAME:
            case FrameType::LEAVE_FRAME_WITH_ARGV:
            case FrameType::BUILTIN_FRAME_WITH_ARGV:
            case FrameType::BUILTIN_ENTRY_FRAME:
            case FrameType::BUILTIN_FRAME:
            case FrameType::INTERPRETER_ENTRY_FRAME: {
                break;
            }
            default: {
                LOG_ECMA(FATAL) << "frame type error!";
                UNREACHABLE();
            }
        }
    }
}
}  // namespace panda::ecmascript