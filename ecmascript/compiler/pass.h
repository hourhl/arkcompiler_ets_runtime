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

#ifndef ECMASCRIPT_COMPILER_PASS_H
#define ECMASCRIPT_COMPILER_PASS_H

#include "ecmascript/compiler/async_function_lowering.h"
#include "ecmascript/compiler/bytecode_circuit_builder.h"
#include "ecmascript/compiler/common_stubs.h"
#include "ecmascript/compiler/llvm_codegen.h"
#include "ecmascript/compiler/scheduler.h"
#include "ecmascript/compiler/slowpath_lowering.h"
#include "ecmascript/compiler/ts_type_lowering.h"
#include "ecmascript/compiler/type_inference/type_infer.h"
#include "ecmascript/compiler/type_lowering.h"
#include "ecmascript/compiler/verifier.h"

namespace panda::ecmascript::kungfu {
class PassData {
public:
    explicit PassData(Circuit *circuit, const CompilerLog *log, bool enableMethodLog)
        : circuit_(circuit), log_(log), enableMethodLog_(enableMethodLog) {}
    virtual ~PassData() = default;
    const ControlFlowGraph &GetScheduleResult() const
    {
        return cfg_;
    }

    void SetScheduleResult(const ControlFlowGraph &result)
    {
        cfg_ = result;
    }

    virtual Circuit* GetCircuit() const
    {
        return circuit_;
    }

    const CompilerLog *GetLog() const
    {
        return log_;
    }

    bool GetEnableMethodLog() const
    {
        return enableMethodLog_;
    }

private:
    Circuit *circuit_ {nullptr};
    ControlFlowGraph cfg_;
    const CompilerLog *log_ {nullptr};
    bool enableMethodLog_ {false};
};

template<typename T1>
class PassRunner {
public:
    explicit PassRunner(T1* data) : data_(data) {}
    virtual ~PassRunner() = default;
    template<typename T2, typename... Args>
    bool RunPass(Args... args)
    {
        T2 pass;
        return pass.Run(data_, std::forward<Args>(args)...);
    }

private:
    T1* data_;
};

class TypeInferPass {
public:
    bool Run(PassData* data, BytecodeCircuitBuilder *builder, const JSHandle<JSTaggedValue> &constantPool,
             TSManager *tsManager, LexEnvManager *lexEnvManager, size_t methodId)
    {
        if (builder->HasTypes()) {
            bool enableLog = data->GetEnableMethodLog() && data->GetLog()->OutputType();
            TypeInfer typeInfer(builder, data->GetCircuit(), constantPool, tsManager,
                                lexEnvManager, methodId, enableLog);
            typeInfer.TraverseCircuit();
        }
        return true;
    }
};

class TSTypeLoweringPass {
public:
    bool Run(PassData *data, BytecodeCircuitBuilder *builder, CompilationConfig *cmpCfg, TSManager *tsManager)
    {
        bool enableLog = data->GetEnableMethodLog() && data->GetLog()->OutputCIR();
        TSTypeLowering lowering(builder, data->GetCircuit(), cmpCfg, tsManager, enableLog);
        if (builder->HasTypes()) {
            lowering.RunTSTypeLowering();
        }
        return true;
    }
};

class TypeLoweringPass {
public:
    bool Run(PassData *data, BytecodeCircuitBuilder *builder, CompilationConfig *cmpCfg, TSManager *tsManager)
    {
        bool enableLog = data->GetEnableMethodLog() && data->GetLog()->OutputCIR();
        TypeLowering lowering(builder, data->GetCircuit(), cmpCfg, tsManager, enableLog);
        if (builder->HasTypes()) {
            lowering.RunTypeLowering();
        }
        return true;
    }
};

class SlowPathLoweringPass {
public:
    bool Run(PassData* data, BytecodeCircuitBuilder *builder, CompilationConfig *cmpCfg, TSManager *tsManager)
    {
        bool enableLog = data->GetEnableMethodLog() && data->GetLog()->OutputCIR();
        SlowPathLowering lowering(builder, data->GetCircuit(), cmpCfg, tsManager, enableLog);
        lowering.CallRuntimeLowering();
        return true;
    }
};

class VerifierPass {
public:
    bool Run(PassData* data)
    {
        bool enableLog = data->GetEnableMethodLog() && data->GetLog()->OutputCIR();
        bool isQualified = Verifier::Run(data->GetCircuit(), enableLog);
        if (!isQualified) {
            LOG_FULL(FATAL) << "VerifierPass fail";
            UNREACHABLE();
        }
        return isQualified;
    }
};

class SchedulingPass {
public:
    bool Run(PassData* data)
    {
        bool enableLog = data->GetEnableMethodLog() && data->GetLog()->OutputCIR();
        data->SetScheduleResult(Scheduler::Run(data->GetCircuit(), enableLog));
        return true;
    }
};

class LLVMIRGenPass {
public:
    void CreateCodeGen(LLVMModule *module, bool enableLog)
    {
        llvmImpl_ = std::make_unique<LLVMIRGeneratorImpl>(module, enableLog);
    }

    bool Run(PassData *data, LLVMModule *module,
             const MethodLiteral *methodLiteral, const JSPandaFile *jsPandaFile)
    {
        bool enableLog = data->GetEnableMethodLog() && data->GetLog()->OutputCIR();
        CreateCodeGen(module, enableLog);
        CodeGenerator codegen(llvmImpl_);
        codegen.Run(data->GetCircuit(), data->GetScheduleResult(), module->GetCompilationConfig(),
                    methodLiteral, jsPandaFile);
        return true;
    }
private:
    std::unique_ptr<CodeGeneratorImpl> llvmImpl_ {nullptr};
};

class AsyncFunctionLoweringPass {
public:
    bool Run(PassData* data, BytecodeCircuitBuilder *builder, CompilationConfig *cmpCfg)
    {
        bool enableLog = data->GetEnableMethodLog() && data->GetLog()->OutputCIR();
        AsyncFunctionLowering lowering(builder, data->GetCircuit(), cmpCfg, enableLog);
        if (lowering.IsAsyncRelated()) {
            lowering.ProcessAll();
        }
        return true;
    }
};
} // namespace panda::ecmascript::kungfu
#endif
