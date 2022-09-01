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

#include "dumpheapsnapshot3_fuzzer.h"

#include "ecmascript/napi/include/jsnapi.h"
#include "ecmascript/napi/include/dfx_jsnapi.h"
#include "ecmascript/ecma_string-inl.h"
#include "ecmascript/tooling/interface/file_stream.h"

using namespace panda;
using namespace panda::ecmascript;
using panda::ecmascript::FileStream;

#define MAXBYTELEN sizeof(double)

namespace OHOS {
    void DumpHeapSnapshot3FuzzTest(const uint8_t* data, size_t size)
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        EcmaVM *vm = JSNApi::CreateJSVM(option);
        double input = 0;
        if (size > MAXBYTELEN) {
            size = MAXBYTELEN;
        }
        if (memcpy_s(&input, MAXBYTELEN, data, size) != 0) {
            std::cout << "memcpy_s failed!";
            UNREACHABLE();
        }
        bool isVmMode = true;
        bool isPrivate = false;
        std::string path((char*)data);
        FileStream stream(path);
        Progress *progress = nullptr;
        DFXJSNApi::DumpHeapSnapshot(vm, input, &stream, progress, isVmMode, isPrivate);
        JSNApi::DestroyJSVM(vm);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DumpHeapSnapshot3FuzzTest(data, size);
    return 0;
}