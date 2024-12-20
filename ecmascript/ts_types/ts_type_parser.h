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

#ifndef ECMASCRIPT_TS_TYPES_TS_TYPE_PARSER_H
#define ECMASCRIPT_TS_TYPES_TS_TYPE_PARSER_H

#include "ecmascript/jspandafile/type_literal_extractor.h"
#include "ecmascript/pgo_profiler/pgo_profiler_decoder.h"
#include "ecmascript/ts_types/ts_type_table_generator.h"

namespace panda::ecmascript {
/* TSTypeParser parses types recorded in abc files into TSTypes. VM uses TSTypeTables to
 * store TSTypes. Each TSTypeTable is used to store all types from the same record.
 * Since VM can only record types in GlobalTSTypeRef::MAX_MODULE_ID records and
 * can only record GlobalTSTypeRef::MAX_LOCAL_ID types in one record, all types outside
 * this range will not be parsed and will be treated as any.
 * In addition to this, in the following case, types will not be parsed and will be treated as any.
 * 1. Import types with module request that does not point to one abc file
 * 2. Import types with module request that point to one abc file which is generated by JS
 * 3. Types with kind that are not supported temporarily
 */
class TSTypeParser {
public:
    struct PGOInfo {
        const JSPandaFile *jsPandaFile;
        const CString &recordName;
        uint32_t methodOffset;
        uint32_t cpIdx;
        PGOSampleType pgoType;
        kungfu::PGOBCInfo::Type type;
        PGOProfilerDecoder *decoder;
        bool enableOptTrackField;
    };

    explicit TSTypeParser(TSManager *tsManager);
    ~TSTypeParser() = default;

    GlobalTSTypeRef PUBLIC_API CreateGT(const JSPandaFile *jsPandaFile, const CString &recordName, uint32_t typeId);

    GlobalTSTypeRef PUBLIC_API CreatePGOGT(PGOInfo info);

    inline static bool IsUserDefinedType(const uint32_t typeId)
    {
        return typeId > USER_DEFINED_TYPE_OFFSET;
    }

    inline const std::unordered_map<uint32_t, kungfu::MethodInfo> &GetMethodList() const
    {
        ASSERT(bcInfo_ != nullptr);
        return bcInfo_->GetMethodList();
    }

private:
    static constexpr size_t BUILDIN_TYPE_OFFSET = 20;
    static constexpr size_t USER_DEFINED_TYPE_OFFSET = 100;
    static constexpr size_t DEFAULT_INDEX = 0;
    static constexpr size_t NUM_INDEX_SIG_INDEX = 1;
    static constexpr size_t NUM_GENERICS_PARA_INDEX = 1;
    static constexpr size_t GENERICS_PARA_OFFSET = BUILDIN_TYPE_OFFSET + 1;

    inline GlobalTSTypeRef GetAndStoreGT(const JSPandaFile *jsPandaFile, uint32_t typeId, const CString &recordName,
                                         uint32_t moduleId = 0, uint32_t localId = 0)
    {
        GlobalTSTypeRef gt(moduleId, localId);
        GlobalTypeID gId(jsPandaFile, typeId);
        tsManager_->AddElementToIdGTMap(gId, gt, recordName);
        return gt;
    }

    inline GlobalTSTypeRef GetAndStoreGT(const JSPandaFile *jsPandaFile, PGOSampleType pgoTypeId,
                                         uint32_t moduleId = 0, uint32_t localId = 0)
    {
        GlobalTSTypeRef gt(moduleId, localId);
        GlobalTypeID gId(jsPandaFile, pgoTypeId);
        tsManager_->AddElementToIdGTMap(gId, gt);
        return gt;
    }

    inline GlobalTSTypeRef GetAndStoreImportGT(const JSPandaFile *jsPandaFile, uint32_t typeId,
                                               const CString &recordName, GlobalTSTypeRef gt)
    {
        GlobalTypeID gId(jsPandaFile, typeId);
        tsManager_->AddElementToIdGTMap(gId, gt, recordName, true);
        return gt;
    }

    inline void SetTSType(JSHandle<TSTypeTable> table, JSHandle<JSTaggedValue> type,
                          const GlobalTSTypeRef &gt)
    {
        JSHandle<TSType>(type)->SetGT(gt);
        uint32_t localId = gt.GetLocalId();
        table->Set(thread_, localId, type);
    }

    inline bool TypeNeedResolve(const TypeLiteralExtractor *typeLiteralExtractor) const
    {
        if (typeLiteralExtractor->IsGenerics()) {
            return true;
        }

        TSTypeKind kind = typeLiteralExtractor->GetTypeKind();
        return kind == TSTypeKind::IMPORT || kind == TSTypeKind::INDEXSIG;
    }

    // Primitive typetable does not correspond to a real TSTypeTable.
    // We use gt with moduleId of ModuleTableIdx::PRIMITIVE to represent some special values.
    // In particular, localId after GENERICS_PARA_OFFSET is denotes to
    // the index of type parameters in generics types by subtracting this offset.
    inline GlobalTSTypeRef EncodeParaType(const uint32_t typeId)
    {
        uint32_t absValue = (~typeId);
        return GlobalTSTypeRef(static_cast<uint32_t>(ModuleTableIdx::PRIMITIVE), absValue + GENERICS_PARA_OFFSET);
    }

    inline uint32_t DecodePrarIndex(GlobalTSTypeRef gt)
    {
        ASSERT(IsGenericsParaType(gt));
        return (gt.GetLocalId() - GENERICS_PARA_OFFSET);
    }

    inline bool IsGenericsParaType(GlobalTSTypeRef gt) const
    {
        return gt.IsPrimitiveModule() && (gt.GetLocalId() >= GENERICS_PARA_OFFSET);
    }

    inline bool IsGenericsArrayType(TypeLiteralExtractor *typeLiteralExtractor) const
    {
        return typeLiteralExtractor->GetTypeKind() == TSTypeKind::BUILTIN_INSTANCE &&
               typeLiteralExtractor->GetIntValue(DEFAULT_INDEX) == static_cast<uint32_t>(BuiltinTypeId::ARRAY);
    }

    GlobalTSTypeRef ParseType(const JSPandaFile *jsPandaFile, const CString &recordName, uint32_t typeId);

    GlobalTSTypeRef ParseBuiltinObjType(uint32_t typeId);

    GlobalTSTypeRef ResolveType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                TypeLiteralExtractor *typeLiteralExtractor);

    GlobalTSTypeRef ResolveImportType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                      TypeLiteralExtractor *typeLiteralExtractor);

    GlobalTSTypeRef ParseIndexSigType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                      TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<JSTaggedValue> ParseNonImportType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                               TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<TSClassType> ParseClassType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                         TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<JSTaggedValue> ParseClassInstanceType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                                   TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<TSInterfaceType> ParseInterfaceType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                                 TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<TSUnionType> ParseUnionType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                         TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<TSFunctionType> ParseFunctionType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                               TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<TSArrayType> ParseArrayType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                         TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<TSObjectType> ParseObjectType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                           TypeLiteralExtractor *typeLiteralExtractor);

    GlobalTSTypeRef ParsePGOType(PGOInfo &info);

    JSHandle<JSTaggedValue> ParseNonImportPGOType(GlobalTSTypeRef gt, PGOInfo &info);

    JSHandle<JSTaggedValue> ParseObjectPGOType(GlobalTSTypeRef gt, PGOInfo &info);

    bool VerifyObjIhcPGOType(JSHandle<JSObject> obj, const PGOHClassLayoutDesc &desc);

    void FillPropTypes(const JSPandaFile *jsPandaFile,
                       const CString &recordName,
                       const JSHandle<TSObjectType> &objectType,
                       TypeLiteralExtractor *typeLiteralExtractor,
                       const uint32_t numOfFieldIndex,
                       const uint32_t gap);

    void FillInterfaceMethodTypes(const JSPandaFile *jsPandaFile,
                                  const CString &recordName,
                                  const JSHandle<TSObjectType> &objectType,
                                  TypeLiteralExtractor *typeLiteralExtractor,
                                  const uint32_t numExtends);

    void SetClassName(const JSHandle<TSClassType> &classType,
                      const JSPandaFile *jsPandaFile,
                      TypeLiteralExtractor *typeLiteralExtractor);

    void SetSuperClassType(const JSHandle<TSClassType> &classType,
                           const JSPandaFile *jsPandaFile,
                           const CString &recordName,
                           TypeLiteralExtractor *typeLiteralExtractor);

    void SetFunctionThisType(const JSHandle<TSFunctionType> &functionType,
                             const JSPandaFile *jsPandaFile,
                             const CString &recordName,
                             TypeLiteralExtractor *typeLiteralExtractor);

    void StoreMethodOffset(const JSHandle<TSFunctionType> &functionType, TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<JSTaggedValue> GenerateExportTableFromRecord(const JSPandaFile *jsPandaFile, const CString &recordName,
                                                          const JSHandle<TSTypeTable> &table);

    JSHandle<EcmaString> GenerateImportRelativePath(JSHandle<EcmaString> importRel) const;

    JSHandle<EcmaString> GenerateImportVar(JSHandle<EcmaString> import) const;

    GlobalTSTypeRef GetExportGTByName(JSHandle<EcmaString> target, JSHandle<TaggedArray> &exportTable,
                                      const JSPandaFile *jsPandaFile, const CString &recordName,
                                      std::unordered_set<CString> &markSet);

    GlobalTSTypeRef IterateStarExport(JSHandle<EcmaString> target, const JSPandaFile *jsPandaFile,
                                      const CString &recordName, std::unordered_set<CString> &markSet);

    GlobalTSTypeRef ParseGenericsType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                      TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<JSTaggedValue> ParseGenericsInstanceType(const JSPandaFile *jsPandaFile, const CString &recordName,
                                                      TypeLiteralExtractor *typeLiteralExtractor);

    JSHandle<JSTaggedValue> InstantiateGenericsType(const JSHandle<JSTaggedValue> &genericsType,
                                                    const std::vector<GlobalTSTypeRef> &paras);

    JSHandle<TSFunctionType> InstantiateFuncGenericsType(const JSHandle<TSFunctionType> &genericsType,
                                                         const std::vector<GlobalTSTypeRef> &paras);

    JSHandle<TSClassType> InstantiateClassGenericsType(const JSHandle<TSClassType> &genericsType,
                                                       const std::vector<GlobalTSTypeRef> &paras);

    void CopyClassName(const JSHandle<TSClassType> &genericsType, const JSHandle<TSClassType> &classType);

    JSHandle<TSInterfaceType> InstantiateInterfaceGenericsType(const JSHandle<TSInterfaceType> &genericsType,
                                                               const std::vector<GlobalTSTypeRef> &paras);

    JSHandle<TSObjectType> InstantiateObjGenericsType(const JSHandle<TSObjectType> &oldObjType,
                                                      const std::vector<GlobalTSTypeRef> &paras);

    GlobalTSTypeRef TryReplaceTypePara(GlobalTSTypeRef gt, const std::vector<GlobalTSTypeRef> &paras);

    TSManager *tsManager_ {nullptr};
    EcmaVM *vm_ {nullptr};
    JSThread *thread_ {nullptr};
    ObjectFactory *factory_ {nullptr};
    TSTypeTableGenerator tableGenerator_;
    kungfu::BCInfo *bcInfo_ {nullptr};
};

struct ClassLiteralInfo {
    explicit ClassLiteralInfo(const TypeLiteralExtractor *typeLiteralExtractor);
    ~ClassLiteralInfo() = default;

    static constexpr uint32_t SUPER_CLASS_INDEX = 1;
    static constexpr uint32_t NUM_IMPLEMENTS_INDEX = 2;

    uint32_t numNonStaticFieldsIndex {0};
    uint32_t numNonStaticMethodsIndex {0};
    uint32_t numStaticFieldsIndex {0};
    uint32_t numStaticMethodsIndex {0};
};

struct InterfaceLiteralInfo {
    explicit InterfaceLiteralInfo(const TypeLiteralExtractor *typeLiteralExtractor);
    ~InterfaceLiteralInfo() = default;

    static constexpr uint32_t NUM_EXTENDS_INDEX = 0;

    uint32_t numFieldsIndex {0};
    uint32_t numMethodsIndex {0};
};

struct FunctionLiteralInfo {
    explicit FunctionLiteralInfo(const TypeLiteralExtractor *typeLiteralExtractor);
    ~FunctionLiteralInfo() = default;

    static constexpr uint32_t BITFIELD_INDEX = 0;
    static constexpr uint32_t NAME_INDEX = 1;
    static constexpr uint32_t HAS_THIS_TYPE_INDEX = 2;

    uint32_t numParasIndex {0};
    uint32_t returnTypeIndex {0};
};
}  // panda::ecmascript
#endif  // ECMASCRIPT_TS_TYPES_TS_TYPE_PARSER_H
