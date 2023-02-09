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

// === tests/cases/compiler/privacyCannotNameAccessorDeclFile_exporter.ts ===
declare function AssertType(value:any, type:string):void;
///<reference path='privacyCannotNameAccessorDeclFile_GlobalWidgets.ts'/>
import * as Widgets from "./privacyCannotNameAccessorDeclFile_Widgets";
import * as Widgets1 from "GlobalWidgets";
export function createExportedWidget1() {
AssertType(Widgets.createWidget1(), "Widgets.Widget1");
AssertType(Widgets.createWidget1, "() => Widgets.Widget1");
    return Widgets.createWidget1();
}
export function createExportedWidget2() {
AssertType(Widgets.SpecializedWidget.createWidget2(), "Widgets.SpecializedWidget.Widget2");
AssertType(Widgets.SpecializedWidget.createWidget2, "() => Widgets.SpecializedWidget.Widget2");
AssertType(Widgets.SpecializedWidget, "typeof Widgets.SpecializedWidget");
    return Widgets.SpecializedWidget.createWidget2();
}
export function createExportedWidget3() {
AssertType(Widgets1.createWidget3(), "Widgets1.Widget3");
AssertType(Widgets1.createWidget3, "() => Widgets1.Widget3");
    return Widgets1.createWidget3();
}
export function createExportedWidget4() {
AssertType(Widgets1.SpecializedGlobalWidget.createWidget4(), "Widgets1.SpecializedGlobalWidget.Widget4");
AssertType(Widgets1.SpecializedGlobalWidget.createWidget4, "() => Widgets1.SpecializedGlobalWidget.Widget4");
AssertType(Widgets1.SpecializedGlobalWidget, "typeof Widgets1.SpecializedGlobalWidget");
    return Widgets1.SpecializedGlobalWidget.createWidget4();
}


