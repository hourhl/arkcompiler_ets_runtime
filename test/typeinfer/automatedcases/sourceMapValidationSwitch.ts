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

// === tests/cases/compiler/sourceMapValidationSwitch.ts ===
declare function AssertType(value:any, type:string):void;
let x = 10;
AssertType(x, "number");
AssertType(10, "int");

switch (x) {
    case 5:
        x++;
        break;
    case 10:
        {
            x--;
AssertType(x--, "number");
AssertType(x, "number");

            break;
        }
    default:
        x = x *10;
}
switch (x)
{
    case 5:
        x++;
        break;
    case 10:
        {
            x--;
AssertType(x--, "number");
AssertType(x, "number");

            break;
        }
    default:
        {
            x = x * 10;
AssertType(x = x * 10, "number");
AssertType(x, "number");
AssertType(x * 10, "number");
AssertType(x, "number");
AssertType(10, "int");
        }
}

