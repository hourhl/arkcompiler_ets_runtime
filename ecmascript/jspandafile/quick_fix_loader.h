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

#ifndef ECMASCRIPT_JSPANDAFILE_QUICK_FIX_LOADER_H
#define ECMASCRIPT_JSPANDAFILE_QUICK_FIX_LOADER_H

#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/mem/c_containers.h"

namespace panda::ecmascript {
class QuickFixLoader {
public:
    QuickFixLoader() = default;
    ~QuickFixLoader();

    bool LoadPatch(JSThread *thread, const std::string &patchFileName, const std::string &baseFileName);
    bool LoadPatch(JSThread *thread, const std::string &patchFileName, const void *patchBuffer, size_t patchSize,
                   const std::string &baseFileName);
    bool UnLoadPatch(JSThread *thread, const std::string &patchFileName);
    bool IsQuickFixCausedException(JSThread *thread,
                                   const JSHandle<JSTaggedValue> &exceptionInfo,
                                   const std::string &patchFileName);

private:
    bool ReplaceMethod(JSThread *thread,
                       const JSHandle<ConstantPool> &baseConstpool,
                       const JSHandle<ConstantPool> &patchConstpool,
                       const JSHandle<Program> &patchProgram);
    CUnorderedSet<CString> ParseStackInfo(const CString &stackInfo);
    void ReplaceMethodInner(JSThread *thread,
                            Method  *destMethod,
                            MethodLiteral *srcMethodLiteral,
                            JSTaggedValue srcConstpool);
    void ClearReservedInfo()
    {
        reservedBaseMethodInfo_.clear();
        reservedBaseClassInfo_.clear();
    }

    const JSPandaFile *baseFile_ {nullptr};
    const JSPandaFile *patchFile_ {nullptr};

    // For method unload patch.
    // key: base constpool index, value: base methodLiteral.
    CUnorderedMap<uint32_t, MethodLiteral *> reservedBaseMethodInfo_ {};

    // For class unload patch.
    // key: base constpool index.
    // key: class literal tagged array index, value: base methodLiteral.
    CUnorderedMap<uint32_t, CUnorderedMap<uint32_t, MethodLiteral *>> reservedBaseClassInfo_ {};
    bool hasLoadedPatch_ {false};
};
}  // namespace panda::ecmascript
#endif // ECMASCRIPT_JSPANDAFILE_QUICK_FIX_LOADER_H