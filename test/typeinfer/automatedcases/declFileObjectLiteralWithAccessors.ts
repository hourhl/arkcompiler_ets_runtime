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

// === tests/cases/compiler/declFileObjectLiteralWithAccessors.ts ===
declare function AssertType(value:any, type:string):void;
function /*1*/makePoint(x: number) { 
AssertType({        b: 10,        get x() { return x; },        set x(a: number) { this.b = a; }    }, "{ b: number; x: number; }");
    return {

        b: 10,
AssertType(b, "number");
AssertType(10, "int");

        get x() { 
AssertType(x, "number");
AssertType(x, "number");
return x; },

        set x(a: number) { this.b = a; 
AssertType(x, "number");

AssertType(a, "number");

AssertType(this.b = a, "number");

AssertType(this.b, "any");

AssertType(this, "any");

AssertType(a, "number");
}

    };
};
let /*4*/point = makePoint(2);
AssertType(point, "{ b: number; x: number; }");
AssertType(makePoint(2), "{ b: number; x: number; }");
AssertType(makePoint, "(number) => { b: number; x: number; }");
AssertType(2, "int");

let /*2*/x = point.x;
AssertType(x, "number");
AssertType(point.x, "number");

point./*3*/x = 30;
AssertType(point./*3*/x = 30, "int");
AssertType(point./*3*/x, "number");
AssertType(30, "int");


