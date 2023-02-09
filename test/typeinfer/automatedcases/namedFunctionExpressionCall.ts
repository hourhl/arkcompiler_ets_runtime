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

// === tests/cases/compiler/namedFunctionExpressionCall.ts ===
declare function AssertType(value:any, type:string):void;
let recurser = function foo() {
AssertType(recurser, "() => void");
AssertType(function foo() {    // using the local name    foo();    // using the globally visible name    recurser();}, "() => void");
AssertType(foo, "() => void");

    // using the local name
    foo();
AssertType(foo(), "void");
AssertType(foo, "() => void");

    // using the globally visible name
    recurser();
AssertType(recurser(), "void");
AssertType(recurser, "() => void");

};


(function bar() {
AssertType((function bar() {    bar();}), "() => void");
AssertType(function bar() {    bar();}, "() => void");
AssertType(bar, "() => void");

    bar();
AssertType(bar(), "void");
AssertType(bar, "() => void");

});

