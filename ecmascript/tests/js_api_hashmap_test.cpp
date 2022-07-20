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

#include "ecmascript/containers/containers_private.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_iterator.h"
#include "ecmascript/js_api_hashmap.h"
#include "ecmascript/js_api_hashmap_iterator.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda;
using namespace panda::ecmascript;

namespace panda::test {
class JSAPIHashMapTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        TestHelper::CreateEcmaVMWithScope(instance, thread, scope);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    EcmaVM *instance {nullptr};
    ecmascript::EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};

protected:
    JSAPIHashMap *CreateHashMap()
    {
        ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
        JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();

        JSHandle<JSTaggedValue> globalObject = env->GetJSGlobalObject();
        JSHandle<JSTaggedValue> key(factory->NewFromASCII("ArkPrivate"));
        JSHandle<JSTaggedValue> value =
            JSObject::GetProperty(thread, JSHandle<JSTaggedValue>(globalObject), key).GetValue();

        auto objCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
        objCallInfo->SetFunction(JSTaggedValue::Undefined());
        objCallInfo->SetThis(value.GetTaggedValue());
        objCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int>(containers::ContainerTag::HashMap)));

        [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, objCallInfo);
        JSTaggedValue result = containers::ContainersPrivate::Load(objCallInfo);
        TestHelper::TearDownFrame(thread, prev);

        JSHandle<JSTaggedValue> constructor(thread, result);
        JSHandle<JSAPIHashMap> map(factory->NewJSObjectByConstructor(JSHandle<JSFunction>(constructor), constructor));
        JSTaggedValue hashMapArray = TaggedHashArray::Create(thread);
        map->SetTable(thread, hashMapArray);
        map->SetSize(0);
        return *map;
    }
};

HWTEST_F_L0(JSAPIHashMapTest, HashMapCreate)
{
    JSAPIHashMap *map = CreateHashMap();
    EXPECT_TRUE(map != nullptr);
}

HWTEST_F_L0(JSAPIHashMapTest, HashMapSetAndGet)
{
    constexpr uint32_t NODE_NUMBERS = 8;
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSMutableHandle<JSTaggedValue> key(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> value(thread, JSTaggedValue::Undefined());

    // test JSAPIHashMap
    JSHandle<JSAPIHashMap> hashMap(thread, CreateHashMap());
    std::string myKey("mykey");
    std::string myValue("myvalue");
    for (uint32_t i = 0; i < NODE_NUMBERS; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());
        JSAPIHashMap::Set(thread, hashMap, key, value);
    }
    EXPECT_EQ(hashMap->GetSize(), NODE_NUMBERS);

    for (uint32_t i = 0; i < NODE_NUMBERS; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());

        // test get
        JSTaggedValue gValue = hashMap->Get(thread, key.GetTaggedValue());
        EXPECT_EQ(gValue, value.GetTaggedValue());
    }
}

HWTEST_F_L0(JSAPIHashMapTest, HashMapRemoveAndHas)
{
    constexpr uint32_t NODE_NUMBERS = 8;
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSMutableHandle<JSTaggedValue> key(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> value(thread, JSTaggedValue::Undefined());

    // test JSAPIHashMap
    JSHandle<JSAPIHashMap> hashMap(thread, CreateHashMap());
    std::string myKey("mykey");
    std::string myValue("myvalue");
    for (uint32_t i = 0; i < NODE_NUMBERS; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());
        JSAPIHashMap::Set(thread, hashMap, key, value);
    }
    EXPECT_EQ(hashMap->GetSize(), NODE_NUMBERS);

    for (uint32_t i = 0; i < NODE_NUMBERS / 2; i++) {
        std::string iKey = myKey + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        [[maybe_unused]] JSTaggedValue rValue = JSAPIHashMap::Remove(thread, hashMap, key.GetTaggedValue());
    }
    EXPECT_EQ(hashMap->GetSize(), NODE_NUMBERS / 2);

    for (uint32_t i = 0; i < NODE_NUMBERS / 2; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());

        // test has
        JSTaggedValue hasKey = hashMap->HasKey(thread, key.GetTaggedValue());
        EXPECT_EQ(hasKey, JSTaggedValue::False());
        JSTaggedValue hasValue = JSAPIHashMap::HasValue(thread, hashMap, value);
        EXPECT_EQ(hasValue, JSTaggedValue::False());
    }

    for (uint32_t i = NODE_NUMBERS / 2; i < NODE_NUMBERS; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());

        // test has
        JSTaggedValue hasKey = hashMap->HasKey(thread, key.GetTaggedValue());
        EXPECT_EQ(hasKey, JSTaggedValue::True());
        JSTaggedValue hasValue = JSAPIHashMap::HasValue(thread, hashMap, value);
        EXPECT_EQ(hasValue, JSTaggedValue::True());
    }
}

HWTEST_F_L0(JSAPIHashMapTest, HashMapReplaceAndClear)
{
    constexpr uint32_t NODE_NUMBERS = 8;
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSMutableHandle<JSTaggedValue> key(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> value(thread, JSTaggedValue::Undefined());
    // test TaggedHashMap
    JSHandle<JSAPIHashMap> hashMap(thread, CreateHashMap());
    std::string myKey("mykey");
    std::string myValue("myvalue");
    for (uint32_t i = 0; i < NODE_NUMBERS; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());
        JSAPIHashMap::Set(thread, hashMap, key, value);
    }
    EXPECT_EQ(hashMap->GetSize(), NODE_NUMBERS);
    for (uint32_t i = 0; i < NODE_NUMBERS / 2; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i + 1);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());
        // test replace
        JSTaggedValue success = hashMap->Replace(thread, key.GetTaggedValue(), value.GetTaggedValue());
        EXPECT_EQ(success, JSTaggedValue::True());
    }
    for (uint32_t i = 0; i < NODE_NUMBERS / 2; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i + 1);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());
        // test get
        JSTaggedValue gValue = hashMap->Get(thread, key.GetTaggedValue());
        EXPECT_EQ(gValue, value.GetTaggedValue());
    }
    for (uint32_t i = NODE_NUMBERS / 2; i < NODE_NUMBERS; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());
        // test get
        JSTaggedValue gValue = hashMap->Get(thread, key.GetTaggedValue());
        EXPECT_EQ(gValue, value.GetTaggedValue());
    }
    for (uint32_t i = 0; i < NODE_NUMBERS / 2; i++) {
        std::string iKey = myKey + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        [[maybe_unused]] JSTaggedValue rValue = JSAPIHashMap::Remove(thread, hashMap, key.GetTaggedValue());
    }
    hashMap->Clear(thread);
    EXPECT_EQ(hashMap->GetSize(), (uint32_t)0);
    for (uint32_t i = 0; i < NODE_NUMBERS; i++) {
        std::string iKey = myKey + std::to_string(i);
        std::string iValue = myValue + std::to_string(i);
        key.Update(factory->NewFromStdString(iKey).GetTaggedValue());
        value.Update(factory->NewFromStdString(iValue).GetTaggedValue());
        // test get
        JSTaggedValue gValue = hashMap->Get(thread, key.GetTaggedValue());
        EXPECT_EQ(gValue, JSTaggedValue::Undefined());
        // test has
        JSTaggedValue hasKey = hashMap->HasKey(thread, key.GetTaggedValue());
        EXPECT_EQ(hasKey, JSTaggedValue::False());
        JSTaggedValue hasValue = JSAPIHashMap::HasValue(thread, hashMap, value);
        EXPECT_EQ(hasValue, JSTaggedValue::False());
    }
}

HWTEST_F_L0(JSAPIHashMapTest, JSAPIHashMapIterator)
{
    constexpr uint32_t NODE_NUMBERS = 8;
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSAPIHashMap> hashMap(thread, CreateHashMap());
    JSMutableHandle<JSTaggedValue> key(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> value(thread, JSTaggedValue::Undefined());
    for (uint32_t i = 0; i < NODE_NUMBERS; i++) {
        key.Update(JSTaggedValue(i));
        value.Update(JSTaggedValue(i));
        JSAPIHashMap::Set(thread, hashMap, key, value);
    }
    // test key or value
    JSHandle<JSTaggedValue> keyIter(factory->NewJSAPIHashMapIterator(hashMap, IterationKind::KEY));
    JSHandle<JSTaggedValue> valueIter(factory->NewJSAPIHashMapIterator(hashMap, IterationKind::VALUE));
    JSMutableHandle<JSTaggedValue> keyIterResult(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> valueIterResult(thread, JSTaggedValue::Undefined());
    for (uint32_t i = 0; i < NODE_NUMBERS / 2; i++) {
        keyIterResult.Update(JSIterator::IteratorStep(thread, keyIter).GetTaggedValue());
        valueIterResult.Update(JSIterator::IteratorStep(thread, valueIter).GetTaggedValue());
        JSHandle<JSTaggedValue> tmpIterKey = JSIterator::IteratorValue(thread, keyIterResult);
        JSTaggedValue iterKeyFlag = hashMap->HasKey(thread, tmpIterKey.GetTaggedValue());
        EXPECT_EQ(JSTaggedValue::True(), iterKeyFlag);
        JSHandle<JSTaggedValue> tmpIterValue = JSIterator::IteratorValue(thread, valueIterResult);
        JSTaggedValue iterValueFlag = JSAPIHashMap::HasValue(thread, hashMap, tmpIterValue);
        EXPECT_EQ(JSTaggedValue::True(), iterValueFlag);
    }
    // test key and value
    JSHandle<JSTaggedValue> indexKey(thread, JSTaggedValue(0));
    JSHandle<JSTaggedValue> elementKey(thread, JSTaggedValue(1));
    JSHandle<JSTaggedValue> iter(factory->NewJSAPIHashMapIterator(hashMap, IterationKind::KEY_AND_VALUE));
    JSMutableHandle<JSTaggedValue> iterResult(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> result(thread, JSTaggedValue::Undefined());
    for (uint32_t i = 0; i < NODE_NUMBERS; i++) {
        iterResult.Update(JSIterator::IteratorStep(thread, iter).GetTaggedValue());
        result.Update(JSIterator::IteratorValue(thread, iterResult).GetTaggedValue());
        JSHandle<JSTaggedValue> tmpKey = JSObject::GetProperty(thread, result, indexKey).GetValue();
        JSTaggedValue iterKeyFlag = hashMap->HasKey(thread, tmpKey.GetTaggedValue());
        EXPECT_EQ(JSTaggedValue::True(), iterKeyFlag);
        JSHandle<JSTaggedValue> tmpValue = JSObject::GetProperty(thread, result, elementKey).GetValue();
        JSTaggedValue iterValueFlag = JSAPIHashMap::HasValue(thread, hashMap, tmpValue);
        EXPECT_EQ(JSTaggedValue::True(), iterValueFlag);
    }
    // test delete
    key.Update(JSTaggedValue(NODE_NUMBERS / 2));
    JSTaggedValue rValue = JSAPIHashMap::Remove(thread, hashMap, key.GetTaggedValue());
    EXPECT_EQ(rValue, JSTaggedValue(NODE_NUMBERS / 2));
    for (uint32_t i = NODE_NUMBERS / 2 + 1; i < NODE_NUMBERS; i++) {
        keyIterResult.Update(JSIterator::IteratorStep(thread, keyIter).GetTaggedValue());
        valueIterResult.Update(JSIterator::IteratorStep(thread, valueIter).GetTaggedValue());
        JSHandle<JSTaggedValue> tmpIterKey = JSIterator::IteratorValue(thread, keyIterResult);
        JSTaggedValue iterKeyFlag = hashMap->HasKey(thread, tmpIterKey.GetTaggedValue());
        EXPECT_EQ(JSTaggedValue::True(), iterKeyFlag);
        JSHandle<JSTaggedValue> tmpIterValue = JSIterator::IteratorValue(thread, valueIterResult);
        JSTaggedValue iterValueFlag = JSAPIHashMap::HasValue(thread, hashMap, tmpIterValue);
        EXPECT_EQ(JSTaggedValue::True(), iterValueFlag);
    }
    // test set
    key.Update(JSTaggedValue(NODE_NUMBERS));
    JSAPIHashMap::Set(thread, hashMap, key, key);
    keyIterResult.Update(JSIterator::IteratorStep(thread, keyIter).GetTaggedValue());
    JSHandle<JSTaggedValue> tmpIterKey = JSIterator::IteratorValue(thread, keyIterResult);
    JSTaggedValue iterKeyFlag = hashMap->HasKey(thread, tmpIterKey.GetTaggedValue());
    EXPECT_EQ(JSTaggedValue::True(), iterKeyFlag);
    EXPECT_EQ(hashMap->GetSize(), NODE_NUMBERS);
    keyIterResult.Update(JSIterator::IteratorStep(thread, keyIter).GetTaggedValue());
    EXPECT_EQ(JSTaggedValue::False(), keyIterResult.GetTaggedValue());
}
}  // namespace panda::test