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

// === tests/cases/compiler/nestedBlockScopedBindings3.ts ===
declare function AssertType(value:any, type:string):void;
function a0() {
    {
        for (let x = 0; x < 1; ) {
AssertType(x, "number");
AssertType(0, "int");
AssertType(x < 1, "boolean");
AssertType(x, "number");
AssertType(1, "int");

            () => x;
AssertType(() => x, "() => number");
AssertType(x, "number");
        }
    }
    {
        for (let x;;) {
AssertType(x, "any");

            () => x;
AssertType(() => x, "() => any");
AssertType(x, "any");
        }
    }
}

function a1() {
    for (let x; x < 1;) {
AssertType(x, "any");
AssertType(x < 1, "boolean");
AssertType(x, "any");
AssertType(1, "int");

        () => x;
AssertType(() => x, "() => any");
AssertType(x, "any");
    }
    for (let x;;) {
AssertType(x, "any");

        () => x;
AssertType(() => x, "() => any");
AssertType(x, "any");
    }
}

function a2() {
    for (let x; x < 1;) {
AssertType(x, "any");
AssertType(x < 1, "boolean");
AssertType(x, "any");
AssertType(1, "int");

        x = x + 1;
AssertType(x = x + 1, "any");
AssertType(x, "any");
AssertType(x + 1, "any");
AssertType(x, "any");
AssertType(1, "int");
    }
    for (let x;;) {
AssertType(x, "any");

        x = x + 2;
AssertType(x = x + 2, "any");
AssertType(x, "any");
AssertType(x + 2, "any");
AssertType(x, "any");
AssertType(2, "int");
    }
}


function a3() {
    for (let x; x < 1;) {
AssertType(x, "any");
AssertType(x < 1, "boolean");
AssertType(x, "any");
AssertType(1, "int");

        x = x + 1;
AssertType(x = x + 1, "any");
AssertType(x, "any");
AssertType(x + 1, "any");
AssertType(x, "any");
AssertType(1, "int");
    }
    switch (1) {
AssertType(1, "int");

        case 1:
AssertType(1, "int");

            let x;
AssertType(x, "any");

            break;
    }
}

function a4() {
    for (let x; x < 1;) {
AssertType(x, "any");
AssertType(x < 1, "boolean");
AssertType(x, "any");
AssertType(1, "int");

        x = x + 1;
AssertType(x = x + 1, "any");
AssertType(x, "any");
AssertType(x + 1, "any");
AssertType(x, "any");
AssertType(1, "int");

        () => x;
AssertType(() => x, "() => any");
AssertType(x, "any");
    }
    switch (1) {
AssertType(1, "int");

        case 1:
AssertType(1, "int");

            let x;
AssertType(x, "any");

            break;
    }
}


function a5() {
    for (let x; x < 1;) {
AssertType(x, "any");
AssertType(x < 1, "boolean");
AssertType(x, "any");
AssertType(1, "int");

        x = x + 1;
AssertType(x = x + 1, "any");
AssertType(x, "any");
AssertType(x + 1, "any");
AssertType(x, "any");
AssertType(1, "int");

        () => x;
AssertType(() => x, "() => any");
AssertType(x, "any");
    }
    switch (1) {
AssertType(1, "int");

        case 1:
AssertType(1, "int");

            let x;
AssertType(x, "any");

            () => x;
AssertType(() => x, "() => any");
AssertType(x, "any");

            break;
    }
}

