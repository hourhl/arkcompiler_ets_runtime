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

// === tests/cases/conformance/jsdoc/declarations/cls.js ===
declare function AssertType(value:any, type:string):void;
const Bar = require("./bar");
AssertType(Bar, "typeof Bar");
AssertType(require("./bar"), "typeof Bar");
AssertType(require, "any");
AssertType("./bar", "string");

const Strings = {
AssertType(Strings, "{ a: string; b: string; }");
AssertType({    a: "A",    b: "B"}, "{ a: string; b: string; }");

    a: "A",
AssertType(a, "string");
AssertType("A", "string");

    b: "B"
AssertType(b, "string");
AssertType("B", "string");

};
class Foo extends Bar {}
module.exports = Foo;
AssertType(module.exports = Foo, "typeof Foo");
AssertType(module.exports, "typeof Foo");
AssertType(Foo, "typeof Foo");

module.exports.Strings = Strings;
AssertType(module.exports.Strings = Strings, "{ a: string; b: string; }");
AssertType(module.exports.Strings, "{ a: string; b: string; }");
AssertType(Strings, "{ a: string; b: string; }");


