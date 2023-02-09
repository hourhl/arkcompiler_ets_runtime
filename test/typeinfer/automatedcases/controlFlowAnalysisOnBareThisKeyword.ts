/*
* Copyright (c) Microsoft Corporation. All rights reserved.
* Copyright (c) 2023 Huawei Device Co., Ltd.
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
*
* This file has been modified by Huawei to verify type inference by adding verification statements.
*/

// === tests/cases/compiler/controlFlowAnalysisOnBareThisKeyword.ts ===
declare function AssertType(value:any, type:string):void;
declare function isBig(x: any): x is { big: true };
function bigger(this: {}) {
    if (isBig(this)) {
AssertType(isBig(this), "boolean");
AssertType(isBig, "(any) => x is { big: true; }");
AssertType(this, "{}");

        this.big; // Expect property to exist
AssertType(this.big, "boolean");
AssertType(this, "{ big: true; }");
    }
}

function bar(this: string | number) {
    if (typeof this === "string") {
AssertType(typeof this === "string", "boolean");
AssertType(typeof this, "union");
AssertType(this, "union");
AssertType("string", "string");

        const x: string = this;
AssertType(x, "string");
AssertType(this, "string");
    }
}

