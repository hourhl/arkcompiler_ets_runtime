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

#ifndef PANDA_RUNTIME_ECMASCRIPT_JS_MAP_ITERATOR_H
#define PANDA_RUNTIME_ECMASCRIPT_JS_MAP_ITERATOR_H

#include "js_iterator.h"
#include "js_object.h"
namespace panda::ecmascript {
class JSMapIterator : public JSObject {
public:
    static JSMapIterator *Cast(ObjectHeader *obj)
    {
        ASSERT(JSTaggedValue(obj).IsJSMapIterator());
        return static_cast<JSMapIterator *>(obj);
    }
    static JSHandle<JSTaggedValue> CreateMapIterator(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                                     IterationKind kind);

    static JSTaggedValue Next(EcmaRuntimeCallInfo *argv);
    void Update(const JSThread *thread);

    static constexpr size_t ITERATED_MAP_OFFSET = JSObject::SIZE;
    ACCESSORS(IteratedMap, ITERATED_MAP_OFFSET, NEXT_INDEX_OFFSET);
    ACCESSORS(NextIndex, NEXT_INDEX_OFFSET, ITERATION_KIND_OFFSET);
    ACCESSORS(IterationKind, ITERATION_KIND_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSObject, ITERATED_MAP_OFFSET, SIZE)

    DECL_DUMP()
};
}  // namespace panda::ecmascript

#endif  // PANDA_RUNTIME_ECMASCRIPT_JS_MAP_ITERATOR_H
