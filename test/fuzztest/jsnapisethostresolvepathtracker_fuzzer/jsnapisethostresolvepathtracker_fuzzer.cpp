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

#include "jsnapisethostresolvepathtracker_fuzzer.h"

#include "ecmascript/napi/include/jsnapi.h"

using namespace panda;
using namespace panda::ecmascript;

namespace OHOS {
    void JSNApiSetHostResolvePathTrackerFuzzTest(const uint8_t* data, size_t size)
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        EcmaVM *vm = JSNApi::CreateJSVM(option);
        [[maybe_unused]] LocalScope scope(vm);
        std::string dirPath(data, data + size);
        std::string requestPath(data, data + size);
        using CallbackType = std::function<std::string(std::string, std::string)>;
        CallbackType callback = [dirPath, requestPath](std::string dp, std::string rp) -> std::string {
            dp = dirPath;
            rp = requestPath;
            return dp + rp;
        };
        JSNApi::SetHostResolvePathTracker(vm, callback);
        JSNApi::DestroyJSVM(vm);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::JSNApiSetHostResolvePathTrackerFuzzTest(data, size);
    return 0;
}