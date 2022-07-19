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

#include "containers_hashmap.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/js_api_hashmap.h"
#include "ecmascript/js_api_hashmap_iterator.h"
#include "ecmascript/js_function.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_array-inl.h"
#include "ecmascript/tagged_hash_array.h"
#include "ecmascript/tagged_node.h"
#include "ecmascript/tagged_queue.h"

namespace panda::ecmascript::containers {
JSTaggedValue ContainersHashMap::HashMapConstructor(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Constructor);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();

    JSHandle<JSTaggedValue> newTarget = GetNewTarget(argv);
    if (newTarget->IsUndefined()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "new target can't be undefined", JSTaggedValue::Exception());
    }

    JSHandle<JSTaggedValue> constructor = GetConstructor(argv);
    JSHandle<JSObject> obj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(constructor), newTarget);
    RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);

    JSHandle<JSAPIHashMap> hashMap = JSHandle<JSAPIHashMap>::Cast(obj);
    JSTaggedValue hashMapArray = TaggedHashArray::Create(thread);
    hashMap->SetTable(thread, hashMapArray);
    hashMap->SetSize(0);

    return hashMap.GetTaggedValue();
}

JSTaggedValue ContainersHashMap::Keys(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Keys);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> iter =
        JSAPIHashMapIterator::CreateHashMapIterator(thread, self, IterationKind::KEY);
    return iter.GetTaggedValue();
}

JSTaggedValue ContainersHashMap::Values(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Values);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> iter =
        JSAPIHashMapIterator::CreateHashMapIterator(thread, self, IterationKind::VALUE);
    return iter.GetTaggedValue();
}

JSTaggedValue ContainersHashMap::Entries(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Entries);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> iter =
        JSAPIHashMapIterator::CreateHashMapIterator(thread, self, IterationKind::KEY_AND_VALUE);
    return iter.GetTaggedValue();
}

JSTaggedValue ContainersHashMap::ForEach(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, ForEach);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> thisHandle = GetThis(argv);
    if (!thisHandle->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> callbackFnHandle = GetCallArg(argv, 0);
    if (!callbackFnHandle->IsCallable()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "the callbackfun is not callable.", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> thisArgHandle = GetCallArg(argv, 1);
    JSHandle<JSAPIHashMap> hashMap = JSHandle<JSAPIHashMap>::Cast(thisHandle);
    JSHandle<TaggedHashArray> table(thread, hashMap->GetTable());
    uint32_t len = table->GetLength();
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSMutableHandle<TaggedQueue> queue(thread, factory->NewTaggedQueue(0));
    JSMutableHandle<TaggedNode> node(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> key(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> value(thread, JSTaggedValue::Undefined());
    uint32_t index = 0;
    JSHandle<JSTaggedValue> undefined = thread->GlobalConstants()->GetHandledUndefined();
    while (index < len) {
        node.Update(TaggedHashArray::GetCurrentNode(thread, queue, table, index));
        if (!node.GetTaggedValue().IsHole()) {
            key.Update(node->GetKey());
            value.Update(node->GetValue());
            EcmaRuntimeCallInfo *info =
                EcmaInterpreter::NewRuntimeCallInfo(thread, callbackFnHandle,
                                                    thisArgHandle, undefined, 3); // 3: three args
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
            info->SetCallArg(value.GetTaggedValue(), key.GetTaggedValue(), thisHandle.GetTaggedValue());
            JSTaggedValue funcResult = JSFunction::Call(info);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, funcResult);
        }
    }
    return JSTaggedValue::Undefined();
}

JSTaggedValue ContainersHashMap::Set(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Set);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> key = GetCallArg(argv, 0);
    JSHandle<JSTaggedValue> value = GetCallArg(argv, 1);
    JSHandle<JSAPIHashMap> hashMap = JSHandle<JSAPIHashMap>::Cast(self);
    JSAPIHashMap::Set(thread, hashMap, key, value);
    RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
    return hashMap.GetTaggedValue();
}

JSTaggedValue ContainersHashMap::SetAll(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, SetAll);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }

    JSHandle<JSTaggedValue> obj = GetCallArg(argv, 0);
    if (!obj->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Incorrect parameters, it should be HashMap", JSTaggedValue::Exception());
    }

    JSHandle<JSAPIHashMap> dmap = JSHandle<JSAPIHashMap>::Cast(self);
    JSHandle<JSAPIHashMap> smap = JSHandle<JSAPIHashMap>::Cast(obj);
    JSAPIHashMap::SetAll(thread, dmap, smap);
    return self.GetTaggedValue();
}

JSTaggedValue ContainersHashMap::Get(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Get);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> key = GetCallArg(argv, 0);
    JSHandle<JSAPIHashMap> hashMap = JSHandle<JSAPIHashMap>::Cast(self);
    return hashMap->Get(thread, key.GetTaggedValue());
}

JSTaggedValue ContainersHashMap::Remove(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Remove);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> key = GetCallArg(argv, 0);
    JSHandle<JSAPIHashMap> hashMap = JSHandle<JSAPIHashMap>::Cast(self);
    return JSAPIHashMap::Remove(thread, hashMap, key.GetTaggedValue());
}

JSTaggedValue ContainersHashMap::HasKey(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, HasKey);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> key = GetCallArg(argv, 0);
    JSHandle<JSAPIHashMap> hashMap = JSHandle<JSAPIHashMap>::Cast(self);
    return hashMap->HasKey(thread, key.GetTaggedValue());
}

JSTaggedValue ContainersHashMap::HasValue(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, HasValue);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> value = GetCallArg(argv, 0);
    JSHandle<JSAPIHashMap> hashMap = JSHandle<JSAPIHashMap>::Cast(self);
    return JSAPIHashMap::HasValue(thread, hashMap, value);
}

JSTaggedValue ContainersHashMap::Replace(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Replace);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> key = GetCallArg(argv, 0);
    JSHandle<JSTaggedValue> newValue = GetCallArg(argv, 1);
    JSHandle<JSAPIHashMap> jsHashMap = JSHandle<JSAPIHashMap>::Cast(self);
    return jsHashMap->Replace(thread, key.GetTaggedValue(), newValue.GetTaggedValue());
}

JSTaggedValue ContainersHashMap::Clear(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, Clear);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSAPIHashMap> jsHashMap = JSHandle<JSAPIHashMap>::Cast(self);
    jsHashMap->Clear(thread);
    return JSTaggedValue::Undefined();
}

JSTaggedValue ContainersHashMap::GetLength(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, GetLength);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSAPIHashMap> jsHashMap = JSHandle<JSAPIHashMap>::Cast(self);
    return jsHashMap->GetLength();
}

JSTaggedValue ContainersHashMap::IsEmpty(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashMap, IsEmpty);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashMap()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashMap", JSTaggedValue::Exception());
    }
    JSHandle<JSAPIHashMap> jsHashMap = JSHandle<JSAPIHashMap>::Cast(self);
    return jsHashMap->IsEmpty();
}
} // namespace panda::ecmascript::containers
