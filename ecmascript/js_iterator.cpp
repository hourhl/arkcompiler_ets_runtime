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

#include "ecmascript/js_iterator.h"

#include "ecmascript/accessor_data.h"
#include "ecmascript/ecma_macros.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/interpreter/fast_runtime_stub-inl.h"
#include "ecmascript/js_symbol.h"
#include "ecmascript/object_factory.h"

namespace panda::ecmascript {
JSTaggedValue JSIterator::IteratorCloseAndReturn(JSThread *thread, const JSHandle<JSTaggedValue> &iter)
{
    ASSERT(thread->HasPendingException());
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSTaggedValue exception = thread->GetException();
    JSHandle<JSTaggedValue> record = JSHandle<JSTaggedValue>(factory->NewCompletionRecord(CompletionRecordType::THROW,
        JSHandle<JSTaggedValue>(thread, exception)));
    JSHandle<JSTaggedValue> result = JSIterator::IteratorClose(thread, iter, record);
    if (result->IsCompletionRecord()) {
        return CompletionRecord::Cast(result->GetTaggedObject())->GetValue();
    }
    return result.GetTaggedValue();
}

JSHandle<JSTaggedValue> JSIterator::GetIterator(JSThread *thread, const JSHandle<JSTaggedValue> &obj)
{
    // 1.ReturnIfAbrupt(obj).
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, obj);
    // 2.If method was not passed, then
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSTaggedValue> iteratorSymbol = env->GetIteratorSymbol();
    JSHandle<JSTaggedValue> func = JSObject::GetMethod(thread, obj, iteratorSymbol);
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, obj);

    return GetIterator(thread, obj, func);
}

JSHandle<JSTaggedValue> JSIterator::GetIterator(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                                const JSHandle<JSTaggedValue> &method)
{
    // 1.ReturnIfAbrupt(obj).
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, obj);
    // 3.Let iterator be Call(method,obj).
    JSHandle<JSTaggedValue> undefined = thread->GlobalConstants()->GetHandledUndefined();
    EcmaRuntimeCallInfo *info = EcmaInterpreter::NewRuntimeCallInfo(thread, method, obj, undefined, 0);
    JSTaggedValue ret = JSFunction::Call(info);
    JSHandle<JSTaggedValue> iter(thread, ret);
    // 4.ReturnIfAbrupt(iterator).
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, iter);
    // 5.If Type(iterator) is not Object, throw a TypeError exception
    if (!iter->IsECMAObject()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "", undefined);
    }
    return iter;
}  // namespace panda::ecmascript
// 7.4.2
JSHandle<JSTaggedValue> JSIterator::IteratorNext(JSThread *thread, const JSHandle<JSTaggedValue> &iter)
{
    const GlobalEnvConstants *globalConst = thread->GlobalConstants();

    // 1.If value was not passed, then Let result be Invoke(iterator, "next", «‍ »).
    JSHandle<JSTaggedValue> key(globalConst->GetHandledNextString());
    JSHandle<JSTaggedValue> next(JSObject::GetMethod(thread, iter, key));
    JSHandle<JSTaggedValue> undefined = globalConst->GetHandledUndefined();
    EcmaRuntimeCallInfo *info = EcmaInterpreter::NewRuntimeCallInfo(thread, next, iter, undefined, 0);
    JSTaggedValue ret = JSFunction::Call(info);
    // 3.ReturnIfAbrupt(result)
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, undefined);
    // 4.If Type(result) is not Object, throw a TypeError exception.
    if (!ret.IsECMAObject()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Is not Object", undefined);
    }
    JSHandle<JSTaggedValue> result(thread, ret);
    return result;
}

JSHandle<JSTaggedValue> JSIterator::IteratorNext(JSThread *thread, const JSHandle<JSTaggedValue> &iter,
                                            const JSHandle<JSTaggedValue> &value)
{
    const GlobalEnvConstants *globalConst = thread->GlobalConstants();
    // 2.Let result be Invoke(iterator, "next", «‍value»).
    JSHandle<JSTaggedValue> key(globalConst->GetHandledNextString());
    JSHandle<JSTaggedValue> next(JSObject::GetMethod(thread, iter, key));
    JSHandle<JSTaggedValue> undefined = globalConst->GetHandledUndefined();
    EcmaRuntimeCallInfo *info = EcmaInterpreter::NewRuntimeCallInfo(thread, next, iter, undefined, 1);
    info->SetCallArg(value.GetTaggedValue());
    JSTaggedValue ret = JSFunction::Call(info);
    // 3.ReturnIfAbrupt(result)
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, undefined);
    // 4.If Type(result) is not Object, throw a TypeError exception.
    if (!ret.IsECMAObject()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Is not Object", undefined);
    }
    JSHandle<JSTaggedValue> result(thread, ret);
    return result;
}
// 7.4.3
bool JSIterator::IteratorComplete(JSThread *thread, const JSHandle<JSTaggedValue> &iterResult)
{
    ASSERT_PRINT(iterResult->IsECMAObject(), "iterResult must be JSObject");
    // Return ToBoolean(Get(iterResult, "done")).
    JSHandle<JSTaggedValue> doneStr = thread->GlobalConstants()->GetHandledDoneString();
    JSHandle<JSTaggedValue> done = JSTaggedValue::GetProperty(thread, iterResult, doneStr).GetValue();
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
    return done->ToBoolean();
}
// 7.4.4
JSHandle<JSTaggedValue> JSIterator::IteratorValue(JSThread *thread, const JSHandle<JSTaggedValue> &iterResult)
{
    ASSERT_PRINT(iterResult->IsECMAObject(), "iterResult must be JSObject");
    // Return Get(iterResult, "value").
    JSHandle<JSTaggedValue> valueStr = thread->GlobalConstants()->GetHandledValueString();
    JSHandle<JSTaggedValue> value = JSTaggedValue::GetProperty(thread, iterResult, valueStr).GetValue();
    return value;
}
// 7.4.5
JSHandle<JSTaggedValue> JSIterator::IteratorStep(JSThread *thread, const JSHandle<JSTaggedValue> &iter)
{
    // 1.Let result be IteratorNext(iterator).
    JSHandle<JSTaggedValue> result = IteratorNext(thread, iter);
    // 2.ReturnIfAbrupt(result).
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, result);
    // 3.Let done be IteratorComplete(result).
    bool done = IteratorComplete(thread, result);
    // 4.ReturnIfAbrupt(done).
    JSHandle<JSTaggedValue> doneHandle(thread, JSTaggedValue(done));
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, doneHandle);
    // 5.If done is true, return false.
    if (done) {
        JSHandle<JSTaggedValue> falseHandle(thread, JSTaggedValue::False());
        return falseHandle;
    }
    return result;
}
// 7.4.6
JSHandle<JSTaggedValue> JSIterator::IteratorClose(JSThread *thread, const JSHandle<JSTaggedValue> &iter,
                                                  const JSHandle<JSTaggedValue> &completion)
{
    // 1.Assert: Type(iterator) is Object.
    ASSERT_PRINT(iter->IsECMAObject(), "iter must be JSObject");
    const GlobalEnvConstants *globalConst = thread->GlobalConstants();
    JSHandle<JSTaggedValue> exceptionOnThread;
    if (thread->HasPendingException()) {
        exceptionOnThread = JSHandle<JSTaggedValue>(thread, thread->GetException());
        thread->ClearException();
    }
    JSTaggedValue returnStr = globalConst->GetReturnString();
    // 3.Let return be GetMethod(iterator, "return").
    JSTaggedValue func = FastRuntimeStub::FastGetPropertyByName(thread, iter.GetTaggedValue(), returnStr);
    // 4.ReturnIfAbrupt(return).
    JSHandle<JSTaggedValue> returnFunc(thread, func);
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, returnFunc);

    // 5.If return is undefined, return Completion(completion).
    if (returnFunc->IsUndefined() || returnFunc->IsNull()) {
        if (!exceptionOnThread.IsEmpty()) {
            thread->SetException(exceptionOnThread.GetTaggedValue());
        }
        return completion;
    }

    if (!returnFunc->IsCallable()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "return function is not Callable", returnFunc);
    }
    // 6.Let innerResult be Call(return, iterator, «‍ »).
    JSHandle<JSTaggedValue> undefined = globalConst->GetHandledUndefined();
    EcmaRuntimeCallInfo *info = EcmaInterpreter::NewRuntimeCallInfo(thread, returnFunc, iter, undefined, 0);
    JSTaggedValue ret = JSFunction::Call(info);
    if (!exceptionOnThread.IsEmpty()) {
        thread->SetException(exceptionOnThread.GetTaggedValue());
    }
    JSHandle<JSTaggedValue> innerResult(thread, ret);
    JSHandle<CompletionRecord> completionRecord(thread, globalConst->GetUndefined());
    if (completion->IsCompletionRecord()) {
        completionRecord = JSHandle<CompletionRecord>::Cast(completion);
    }
    // 7.If completion.[[type]] is throw, return Completion(completion).
    if (!completionRecord.GetTaggedValue().IsUndefined() && completionRecord->IsThrow()) {
        if (!exceptionOnThread.IsEmpty()) {
            thread->SetException(exceptionOnThread.GetTaggedValue());
        }
        return completion;
    }
    // 8.If innerResult.[[type]] is throw, return Completion(innerResult).
    if (thread->HasPendingException()) {
        return innerResult;
    }
    // 9.If Type(innerResult.[[value]]) is not Object, throw a TypeError exception.
    if (!innerResult->IsECMAObject()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "", undefined);
    }
    if (!exceptionOnThread.IsEmpty()) {
        thread->SetException(exceptionOnThread.GetTaggedValue());
    }
    return completion;
}
// 7.4.7
JSHandle<JSObject> JSIterator::CreateIterResultObject(JSThread *thread, const JSHandle<JSTaggedValue> &value, bool done)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    auto globalConst = thread->GlobalConstants();
    // 2. Let obj be OrdinaryObjectCreate(%Object.prototype%).
    JSHandle<JSHClass> klass = JSHandle<JSHClass>::Cast(globalConst->GetHandledIteratorResultClass());
    JSHandle<JSObject> obj = factory->NewJSObject(klass);

    // 3. Perform ! CreateDataPropertyOrThrow(obj, "value", value).
    // 4. Perform ! CreateDataPropertyOrThrow(obj, "done", done).
    obj->SetPropertyInlinedProps(thread, VALUE_INLINE_PROPERTY_INDEX, value.GetTaggedValue());
    obj->SetPropertyInlinedProps(thread, DONE_INLINE_PROPERTY_INDEX, JSTaggedValue(done));
    // 5. Return obj.
    return obj;
}
}  // namespace panda::ecmascript
