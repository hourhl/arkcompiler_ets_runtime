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

// === tests/cases/conformance/types/typeRelationships/typeAndMemberIdentity/objectTypesIdentityWithComplexConstraints.ts ===
declare function AssertType(value:any, type:string):void;

interface A {
      <T extends {
            <S extends A>(x: T, y: S): void
      }>(x: T, y: T): void
}

interface B {
      <U extends B>(x: U, y: U): void
}

// ok, not considered identical because the steps of contextual signature instantiation create fresh type parameters
function foo(x: A);
function foo(x: B); // error after constraints above made illegal
function foo(x: any) { }

