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

#include "ecmascript/compiler/type_inference/type_infer.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/jspandafile/program_object.h"

namespace panda::ecmascript::kungfu {
void TypeInfer::TraverseCircuit()
{
    size_t gateCount = circuit_->GetGateCount();
    std::vector<bool> inQueue(gateCount, true);
    std::vector<bool> visited(gateCount, false);
    std::queue<GateRef> pendingQueue;
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    for (auto gate : gateList) {
        pendingQueue.push(gate);
    }

    while (!pendingQueue.empty()) {
        auto curGate = pendingQueue.front();
        inQueue[gateAccessor_.GetId(curGate)] = false;
        pendingQueue.pop();
        auto uses = gateAccessor_.ConstUses(curGate);
        for (auto useIt = uses.begin(); useIt != uses.end(); useIt++) {
            auto gateId = gateAccessor_.GetId(*useIt);
            if (Infer(*useIt) && !inQueue[gateId]) {
                inQueue[gateId] = true;
                pendingQueue.push(*useIt);
            }
        }
    }

    if (IsLogEnabled()) {
        PrintAllGatesTypes();
    }

    if (tsManager_->AssertTypes()) {
        Verify();
    }

    if (tsManager_->PrintAnyTypes()) {
        FilterAnyTypeGates();
    }
}

bool TypeInfer::UpdateType(GateRef gate, const GateType type)
{
    auto preType = gateAccessor_.GetGateType(gate);
    if (type.IsTSType() && !type.IsAnyType() && type != preType) {
        gateAccessor_.SetGateType(gate, type);
        return true;
    }
    return false;
}

bool TypeInfer::UpdateType(GateRef gate, const GlobalTSTypeRef &typeRef)
{
    auto type = GateType(typeRef);
    return UpdateType(gate, type);
}

bool TypeInfer::ShouldInfer(const GateRef gate) const
{
    auto op = gateAccessor_.GetOpCode(gate);
    // handle phi gates
    if (op == OpCode::VALUE_SELECTOR) {
        return true;
    }
    if (op == OpCode::JS_BYTECODE ||
        op == OpCode::CONSTANT ||
        op == OpCode::RETURN) {
        const auto &gateToBytecode = builder_->GetGateToBytecode();
        // handle gates generated by ecma.* bytecodes (not including Jump)
        if (gateToBytecode.find(gate) != gateToBytecode.end()) {
            return !builder_->GetByteCodeInfo(gate).IsJump();
        }
    }
    return false;
}

bool TypeInfer::Infer(GateRef gate)
{
    if (!ShouldInfer(gate)) {
        return false;
    }
    if (gateAccessor_.GetOpCode(gate) == OpCode::VALUE_SELECTOR) {
        return InferPhiGate(gate);
    }
    // infer ecma.* bytecode gates
    EcmaBytecode op = builder_->GetByteCodeOpcode(gate);
    switch (op) {
        case EcmaBytecode::LDNAN_PREF:
        case EcmaBytecode::LDINFINITY_PREF:
        case EcmaBytecode::SUB2DYN_PREF_V8:
        case EcmaBytecode::MUL2DYN_PREF_V8:
        case EcmaBytecode::DIV2DYN_PREF_V8:
        case EcmaBytecode::MOD2DYN_PREF_V8:
        case EcmaBytecode::SHL2DYN_PREF_V8:
        case EcmaBytecode::ASHR2DYN_PREF_V8:
        case EcmaBytecode::SHR2DYN_PREF_V8:
        case EcmaBytecode::AND2DYN_PREF_V8:
        case EcmaBytecode::OR2DYN_PREF_V8:
        case EcmaBytecode::XOR2DYN_PREF_V8:
        case EcmaBytecode::TONUMBER_PREF_V8:
        case EcmaBytecode::TONUMERIC_PREF_V8:
        case EcmaBytecode::NEGDYN_PREF_V8:
        case EcmaBytecode::NOTDYN_PREF_V8:
        case EcmaBytecode::INCDYN_PREF_V8:
        case EcmaBytecode::DECDYN_PREF_V8:
        case EcmaBytecode::EXPDYN_PREF_V8:
        case EcmaBytecode::STARRAYSPREAD_PREF_V8_V8:
            return SetNumberType(gate);
        case EcmaBytecode::LDTRUE_PREF:
        case EcmaBytecode::LDFALSE_PREF:
        case EcmaBytecode::EQDYN_PREF_V8:
        case EcmaBytecode::NOTEQDYN_PREF_V8:
        case EcmaBytecode::LESSDYN_PREF_V8:
        case EcmaBytecode::LESSEQDYN_PREF_V8:
        case EcmaBytecode::GREATERDYN_PREF_V8:
        case EcmaBytecode::GREATEREQDYN_PREF_V8:
        case EcmaBytecode::ISINDYN_PREF_V8:
        case EcmaBytecode::INSTANCEOFDYN_PREF_V8:
        case EcmaBytecode::STRICTNOTEQDYN_PREF_V8:
        case EcmaBytecode::STRICTEQDYN_PREF_V8:
        case EcmaBytecode::ISTRUE_PREF:
        case EcmaBytecode::ISFALSE_PREF:
        case EcmaBytecode::SETOBJECTWITHPROTO_PREF_V8_V8:
        case EcmaBytecode::DELOBJPROP_PREF_V8_V8:
            return SetBooleanType(gate);
        case EcmaBytecode::LDUNDEFINED_PREF:
            return InferLdUndefined(gate);
        case EcmaBytecode::LDNULL_PREF:
            return InferLdNull(gate);
        case EcmaBytecode::LDAI_DYN_IMM32:
            return InferLdai(gate);
        case EcmaBytecode::FLDAI_DYN_IMM64:
            return InferFLdai(gate);
        case EcmaBytecode::LDSYMBOL_PREF:
            return InferLdSymbol(gate);
        case EcmaBytecode::THROWDYN_PREF:
            return InferThrow(gate);
        case EcmaBytecode::TYPEOFDYN_PREF:
            return InferTypeOf(gate);
        case EcmaBytecode::ADD2DYN_PREF_V8:
            return InferAdd2(gate);
        case EcmaBytecode::LDOBJBYINDEX_PREF_V8_IMM32:
            return InferLdObjByIndex(gate);
        case EcmaBytecode::STGLOBALVAR_PREF_ID32:
        case EcmaBytecode::STCONSTTOGLOBALRECORD_PREF_ID32:
        case EcmaBytecode::TRYSTGLOBALBYNAME_PREF_ID32:
        case EcmaBytecode::STLETTOGLOBALRECORD_PREF_ID32:
        case EcmaBytecode::STCLASSTOGLOBALRECORD_PREF_ID32:
            return SetStGlobalBcType(gate);
        case EcmaBytecode::LDGLOBALVAR_PREF_ID32:
            return InferLdGlobalVar(gate);
        case EcmaBytecode::RETURNUNDEFINED_PREF:
            return InferReturnUndefined(gate);
        case EcmaBytecode::RETURN_DYN:
            return InferReturn(gate);
        case EcmaBytecode::LDOBJBYNAME_PREF_ID32_V8:
            return InferLdObjByName(gate);
        case EcmaBytecode::LDA_STR_ID32:
            return InferLdStr(gate);
        case EcmaBytecode::CALLARG0DYN_PREF_V8:
        case EcmaBytecode::CALLARG1DYN_PREF_V8_V8:
        case EcmaBytecode::CALLARGS2DYN_PREF_V8_V8_V8:
        case EcmaBytecode::CALLARGS3DYN_PREF_V8_V8_V8_V8:
        case EcmaBytecode::CALLSPREADDYN_PREF_V8_V8_V8:
        case EcmaBytecode::CALLIRANGEDYN_PREF_IMM16_V8:
        case EcmaBytecode::CALLITHISRANGEDYN_PREF_IMM16_V8:
            return InferCallFunction(gate);
        case EcmaBytecode::LDOBJBYVALUE_PREF_V8_V8:
            return InferLdObjByValue(gate);
        case EcmaBytecode::GETNEXTPROPNAME_PREF_V8:
            return InferGetNextPropName(gate);
        case EcmaBytecode::DEFINEGETTERSETTERBYVALUE_PREF_V8_V8_V8_V8:
            return InferDefineGetterSetterByValue(gate);
        case EcmaBytecode::NEWOBJDYNRANGE_PREF_IMM16_V8:
        case EcmaBytecode::NEWOBJAPPLY_PREF_V8_V8:
            return InferNewObject(gate);
        case EcmaBytecode::SUPERCALL_PREF_IMM16_V8:
        case EcmaBytecode::SUPERCALLSPREAD_PREF_V8:
            return InferSuperCall(gate);
        case EcmaBytecode::TRYLDGLOBALBYNAME_PREF_ID32:
            return InferTryLdGlobalByName(gate);
        case EcmaBytecode::LDLEXVARDYN_PREF_IMM4_IMM4:
        case EcmaBytecode::LDLEXVARDYN_PREF_IMM8_IMM8:
        case EcmaBytecode::LDLEXVARDYN_PREF_IMM16_IMM16:
            return InferLdLexVarDyn(gate);
        case EcmaBytecode::STLEXVARDYN_PREF_IMM4_IMM4_V8:
        case EcmaBytecode::STLEXVARDYN_PREF_IMM8_IMM8_V8:
        case EcmaBytecode::STLEXVARDYN_PREF_IMM16_IMM16_V8:
            return InferStLexVarDyn(gate);
        default:
            break;
    }
    return false;
}

bool TypeInfer::InferPhiGate(GateRef gate)
{
    ASSERT(gateAccessor_.GetOpCode(gate) == OpCode::VALUE_SELECTOR);
    CVector<GlobalTSTypeRef> typeList;
    auto ins = gateAccessor_.ConstIns(gate);
    for (auto it =  ins.begin(); it != ins.end(); it++) {
        // assuming that VALUE_SELECTOR is NO_DEPEND and NO_ROOT
        if (gateAccessor_.GetOpCode(*it) == OpCode::MERGE) {
            continue;
        }
        if (gateAccessor_.GetOpCode(*it) == OpCode::LOOP_BEGIN) {
            auto loopInGate = gateAccessor_.GetValueIn(gate);
            auto loopInType = gateAccessor_.GetGateType(loopInGate);
            return UpdateType(gate, loopInType);
        }
        auto valueInType = gateAccessor_.GetGateType(*it);
        if (valueInType.IsAnyType()) {
            return UpdateType(gate, valueInType);
        }
        typeList.emplace_back(GlobalTSTypeRef(valueInType.GetType()));
    }
    // deduplicate
    auto deduplicateIndex = std::unique(typeList.begin(), typeList.end());
    typeList.erase(deduplicateIndex, typeList.end());
    if (typeList.size() > 1) {
        auto unionType = tsManager_->GetOrCreateUnionType(typeList);
        return UpdateType(gate, unionType);
    }
    auto type = typeList.at(0);
    return UpdateType(gate, type);
}

bool TypeInfer::SetNumberType(GateRef gate)
{
    auto numberType = GateType::NumberType();
    return UpdateType(gate, numberType);
}

bool TypeInfer::SetBooleanType(GateRef gate)
{
    auto booleanType = GateType::BooleanType();
    return UpdateType(gate, booleanType);
}

bool TypeInfer::InferLdUndefined(GateRef gate)
{
    auto undefinedType = GateType::UndefinedType();
    return UpdateType(gate, undefinedType);
}

bool TypeInfer::InferLdNull(GateRef gate)
{
    auto nullType = GateType::NullType();
    return UpdateType(gate, nullType);
}

bool TypeInfer::InferLdai(GateRef gate)
{
    auto numberType = GateType::NumberType();
    return UpdateType(gate, numberType);
}

bool TypeInfer::InferFLdai(GateRef gate)
{
    auto numberType = GateType::NumberType();
    return UpdateType(gate, numberType);
}

bool TypeInfer::InferLdSymbol(GateRef gate)
{
    auto symbolType = GateType::SymbolType();
    return UpdateType(gate, symbolType);
}

bool TypeInfer::InferThrow(GateRef gate)
{
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 1);
    auto gateType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    return UpdateType(gate, gateType);
}

bool TypeInfer::InferTypeOf(GateRef gate)
{
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 1);
    auto gateType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    return UpdateType(gate, gateType);
}

bool TypeInfer::InferAdd2(GateRef gate)
{
    // 2: number of value inputs
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 2);
    auto firInType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    auto secInType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 1));
    if (firInType.IsStringType() || secInType.IsStringType()) {
        return UpdateType(gate, GateType::StringType());
    }
    if (firInType.IsNumberType() && secInType.IsNumberType()) {
        return UpdateType(gate, GateType::NumberType());
    }
    return UpdateType(gate, GateType::AnyType());
}

bool TypeInfer::InferLdObjByIndex(GateRef gate)
{
    // 2: number of value inputs
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 2);
    auto inValueType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    if (tsManager_->IsArrayTypeKind(inValueType)) {
        auto type = tsManager_->GetArrayParameterTypeGT(inValueType);
        return UpdateType(gate, type);
    }
    return false;
}

bool TypeInfer::SetStGlobalBcType(GateRef gate)
{
    auto byteCodeInfo = builder_->GetByteCodeInfo(gate);
    ASSERT(byteCodeInfo.inputs.size() == 1);
    auto stringId = std::get<StringId>(byteCodeInfo.inputs[0]).GetId();
    // 2: number of value inputs
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 2);
    auto inValueType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 1));
    if (stringIdToGateType_.find(stringId) != stringIdToGateType_.end()) {
        stringIdToGateType_[stringId] = inValueType;
    } else {
        stringIdToGateType_.emplace(stringId, inValueType);
    }
    return UpdateType(gate, inValueType);
}

bool TypeInfer::InferLdGlobalVar(GateRef gate)
{
    auto byteCodeInfo = builder_->GetByteCodeInfo(gate);
    ASSERT(byteCodeInfo.inputs.size() == 1);
    auto stringId = std::get<StringId>(byteCodeInfo.inputs[0]).GetId();
    auto iter = stringIdToGateType_.find(stringId);
    if (iter != stringIdToGateType_.end()) {
        return UpdateType(gate, iter->second);
    }
    return false;
}

bool TypeInfer::InferReturnUndefined(GateRef gate)
{
    auto undefinedType = GateType::UndefinedType();
    return UpdateType(gate, undefinedType);
}

bool TypeInfer::InferReturn(GateRef gate)
{
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 1);
    auto gateType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    return UpdateType(gate, gateType);
}

bool TypeInfer::InferLdObjByName(GateRef gate)
{
    // 2: number of value inputs
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 2);
    auto objType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 1));
    if (objType.IsTSType()) {
        if (tsManager_->IsArrayTypeKind(objType)) {
            auto builtinGt = GlobalTSTypeRef(TSModuleTable::BUILTINS_TABLE_ID, TSManager::BUILTIN_ARRAY_ID);
            auto builtinInstanceType = tsManager_->CreateClassInstanceType(builtinGt);
            objType = GateType(builtinInstanceType);
        }
        // If this object has no gt type, we cannot get its internal property type
        if (IsObjectOrClass(objType)) {
            auto index = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 0));
            auto name = constantPool_->GetObjectFromCache(index);
            auto type = GetPropType(objType, name);
            return UpdateType(gate, type);
        }
    }
    return false;
}

bool TypeInfer::InferNewObject(GateRef gate)
{
    if (gateAccessor_.GetGateType(gate).IsAnyType()) {
        ASSERT(gateAccessor_.GetNumValueIn(gate) > 0);
        auto classType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
        if (tsManager_->IsClassTypeKind(classType)) {
            auto classInstanceType = tsManager_->CreateClassInstanceType(classType);
            return UpdateType(gate, classInstanceType);
        }
    }
    return false;
}

bool TypeInfer::InferLdStr(GateRef gate)
{
    auto stringType = GateType::StringType();
    return UpdateType(gate, stringType);
}

bool TypeInfer::InferCallFunction(GateRef gate)
{
    // 0 : the index of function
    auto funcType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    if (funcType.IsTSType() && tsManager_->IsFunctionTypeKind(funcType)) {
        auto returnType = tsManager_->GetFuncReturnValueTypeGT(funcType);
        return UpdateType(gate, returnType);
    }
    return false;
}

bool TypeInfer::InferLdObjByValue(GateRef gate)
{
    auto objType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    if (objType.IsTSType()) {
        // handle array
        if (tsManager_->IsArrayTypeKind(objType)) {
            auto elementType = tsManager_->GetArrayParameterTypeGT(objType);
            return UpdateType(gate, elementType);
        }
        // handle object
        if (IsObjectOrClass(objType)) {
            auto valueGate = gateAccessor_.GetValueIn(gate, 1);
            if (gateAccessor_.GetOpCode(valueGate) == OpCode::CONSTANT) {
                auto value = gateAccessor_.GetBitField(valueGate);
                auto type = GetPropType(objType, value);
                return UpdateType(gate, type);
            }
        }
    }
    return false;
}

bool TypeInfer::InferGetNextPropName(GateRef gate)
{
    auto stringType = GateType::StringType();
    return UpdateType(gate, stringType);
}

bool TypeInfer::InferDefineGetterSetterByValue(GateRef gate)
{
    // 0 : the index of obj
    auto objType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    return UpdateType(gate, objType);
}

bool TypeInfer::InferSuperCall(GateRef gate)
{
    ArgumentAccessor argAcc(circuit_);
    auto newTarget = argAcc.GetCommonArgGate(CommonArgIdx::NEW_TARGET);
    auto funcType = gateAccessor_.GetGateType(newTarget);
    if (!funcType.IsUndefinedType()) {
        return UpdateType(gate, funcType);
    }
    return false;
}

bool TypeInfer::InferTryLdGlobalByName(GateRef gate)
{
    // todo by hongtao, should consider function of .d.ts
    auto byteCodeInfo = builder_->GetByteCodeInfo(gate);
    ASSERT(byteCodeInfo.inputs.size() == 1);
    auto stringId = std::get<StringId>(byteCodeInfo.inputs[0]).GetId();
    auto iter = stringIdToGateType_.find(stringId);
    if (iter != stringIdToGateType_.end()) {
        return UpdateType(gate, iter->second);
    }
    return false;
}

bool TypeInfer::InferLdLexVarDyn(GateRef gate)
{
    auto level = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 0));
    auto slot = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 1));
    auto type = lexEnvManager_->GetLexEnvElementType(methodId_, level, slot);
    return UpdateType(gate, type);
}

bool TypeInfer::InferStLexVarDyn(GateRef gate)
{
    auto level = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 0));
    auto slot = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 1));
    auto type = lexEnvManager_->GetLexEnvElementType(methodId_, level, slot);
    if (type.IsAnyType()) {
        auto valueType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 2));
        if (!valueType.IsAnyType()) {
            lexEnvManager_->SetLexEnvElementType(methodId_, level, slot, valueType);
            return true;
        }
    }
    return false;
}

void TypeInfer::PrintAllGatesTypes() const
{
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);

    const JSPandaFile *jsPandaFile = builder_->GetJSPandaFile();
    const MethodLiteral *methodLiteral = builder_->GetMethod();
    EntityId methodId = builder_->GetMethod()->GetMethodId();
    DebugInfoExtractor *debugExtractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
    const std::string &sourceFileName = debugExtractor->GetSourceFile(methodId);
    const std::string functionName = methodLiteral->ParseFunctionName(jsPandaFile, methodId);

    std::string log;
    for (const auto &gate : gateList) {
        auto op = gateAccessor_.GetOpCode(gate);
        const auto &gateToBytecode = builder_->GetGateToBytecode();
        if ((op == OpCode::VALUE_SELECTOR) || (( op == OpCode::JS_BYTECODE || op == OpCode::CONSTANT ||
                                                 op == OpCode::RETURN) &&
                                                 gateToBytecode.find(gate) != gateToBytecode.end()))  {
            log += CollectGateTypeLogInfo(gate, debugExtractor, "[TypePrinter] ");
        }
    }

    LOG_COMPILER(INFO) << "[TypePrinter] [" << sourceFileName << ":" << functionName << "] begin:";
    LOG_COMPILER(INFO) << log << "[TypePrinter] end";
}

void TypeInfer::Verify() const
{
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    for (const auto &gate : gateList) {
        auto op = gateAccessor_.GetOpCode(gate);
        if (op == OpCode::JS_BYTECODE) {
            TypeCheck(gate);
        }
    }
}

/*
 * Let v be a variable in one ts-file and t be a type. To check whether the type of v is t after
 * type inferenece, one should declare a function named "AssertType(value:any, type:string):void"
 * in ts-file and call it with arguments v and t, where t is the expected type string.
 * The following interface performs such a check at compile time.
 */
void TypeInfer::TypeCheck(GateRef gate) const
{
    auto info = builder_->GetByteCodeInfo(gate);
    if (!info.IsBc(EcmaBytecode::CALLARGS2DYN_PREF_V8_V8_V8)) {
        return;
    }
    auto func = gateAccessor_.GetValueIn(gate, 0);
    auto funcInfo = builder_->GetByteCodeInfo(func);
    if (!funcInfo.IsBc(EcmaBytecode::TRYLDGLOBALBYNAME_PREF_ID32)) {
        return;
    }
    auto funcName = gateAccessor_.GetValueIn(func, 0);
    if (constantPool_->GetStdStringByIdx(gateAccessor_.GetBitField(funcName)) ==  "AssertType") {
        GateRef expectedGate = gateAccessor_.GetValueIn(gateAccessor_.GetValueIn(gate, 2), 0);
        auto expectedTypeStr = constantPool_->GetStdStringByIdx(gateAccessor_.GetBitField(expectedGate));
        GateRef valueGate = gateAccessor_.GetValueIn(gate, 1);
        auto type = gateAccessor_.GetGateType(valueGate);
        if (expectedTypeStr != tsManager_->GetTypeStr(type)) {
            const JSPandaFile *jsPandaFile = builder_->GetJSPandaFile();
            const MethodLiteral *methodLiteral = builder_->GetMethod();
            EntityId methodId = builder_->GetMethod()->GetMethodId();
            DebugInfoExtractor *debugExtractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
            const std::string &sourceFileName = debugExtractor->GetSourceFile(methodId);
            const std::string functionName = methodLiteral->ParseFunctionName(jsPandaFile, methodId);

            std::string log = CollectGateTypeLogInfo(valueGate, debugExtractor, "[TypeAssertion] ");
            log += "[TypeAssertion] but expected type: " + expectedTypeStr + "\n";

            LOG_COMPILER(ERROR) << "[TypeAssertion] [" << sourceFileName << ":" << functionName << "] begin:";
            LOG_COMPILER(FATAL) << log << " [TypeAssertion] end";
        }
    }
}

void TypeInfer::FilterAnyTypeGates() const
{
    const JSPandaFile *jsPandaFile = builder_->GetJSPandaFile();
    const MethodLiteral *methodLiteral = builder_->GetMethod();
    EntityId methodId = methodLiteral->GetMethodId();

    DebugInfoExtractor *debugExtractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
    const std::string &sourceFileName = debugExtractor->GetSourceFile(methodId);
    const std::string functionName = methodLiteral->ParseFunctionName(jsPandaFile, methodId);

    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    std::string log;
    for (const auto &gate : gateList) {
        GateType type = gateAccessor_.GetGateType(gate);
        if (ShouldInfer(gate) && type.IsAnyType()) {
            log += CollectGateTypeLogInfo(gate, debugExtractor, "[TypeFilter] ");
        }
    }

    LOG_COMPILER(INFO) << "[TypeFilter] [" << sourceFileName << ":" << functionName << "] begin:";
    LOG_COMPILER(INFO) << log << "[TypeFilter] end";
}

std::string TypeInfer::CollectGateTypeLogInfo(GateRef gate, DebugInfoExtractor *debugExtractor,
                                              const std::string &logPreFix) const
{
    std::string log(logPreFix);
    log += "gate id: "+ std::to_string(gateAccessor_.GetId(gate)) + ", ";
    OpCode op = gateAccessor_.GetOpCode(gate);
    log += "op: " + op.Str() + ", ";
    if (op != OpCode::VALUE_SELECTOR) {
    // handle ByteCode gate: print gate id, bytecode and line number in source code.
        log += "bytecode: " + builder_->GetBytecodeStr(gate) + ", ";
        GateType type = gateAccessor_.GetGateType(gate);
        if (type.IsTSType()) {
            log += "type: " + tsManager_->GetTypeStr(type) + ", ";
            if (!tsManager_->IsPrimitiveTypeKind(type)) {
                GlobalTSTypeRef gt = GlobalTSTypeRef(type.GetType());
                log += "[moduleId: " + std::to_string(gt.GetModuleId()) + ", ";
                log += "localId: " + std::to_string(gt.GetLocalId()) + "], ";
            }
        }

        int32_t lineNumber = 0;
        auto callbackLineFunc = [&lineNumber](int32_t line) -> bool {
            lineNumber = line + 1;
            return true;
        };

        const auto &gateToBytecode = builder_->GetGateToBytecode();
        const uint8_t *pc = gateToBytecode.at(gate).second;
        const MethodLiteral *methodLiteral = builder_->GetMethod();

        uint32_t offset = pc - methodLiteral->GetBytecodeArray();
        debugExtractor->MatchLineWithOffset(callbackLineFunc, methodLiteral->GetMethodId(), offset);

        log += "at line: " + std::to_string(lineNumber);
    } else {
    // handle phi gate: print gate id and input gates id list.
        log += "phi gate, ins: ";
        auto ins = gateAccessor_.ConstIns(gate);
        for (auto it =  ins.begin(); it != ins.end(); it++) {
            log += std::to_string(gateAccessor_.GetId(*it)) + " ";
        }
    }

    log += "\n compiler: ";
    return log;
}
}  // namespace panda::ecmascript
