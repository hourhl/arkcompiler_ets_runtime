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

// === tests/cases/compiler/commentsInheritance.ts ===
declare function AssertType(value:any, type:string):void;
/** i1 is interface with properties*/
interface i1 {
    /** i1_p1*/
    i1_p1: number;
    /** i1_f1*/
    i1_f1(): void;
    /** i1_l1*/
    i1_l1: () => void;
    // il_nc_p1
    i1_nc_p1: number;
    i1_nc_f1(): void;
    i1_nc_l1: () => void;
    p1: number;
    f1(): void;
    l1: () => void;
    nc_p1: number;
    nc_f1(): void;
    nc_l1: () => void;
}
class c1 implements i1 {
    public i1_p1: number;
    // i1_f1
    public i1_f1() {
    }
    public i1_l1: () => void;
    public i1_nc_p1: number;
    public i1_nc_f1() {
    }
    public i1_nc_l1: () => void;
    /** c1_p1*/
    public p1: number;
    /** c1_f1*/
    public f1() {
    }
    /** c1_l1*/
    public l1: () => void;
    /** c1_nc_p1*/
    public nc_p1: number;
    /** c1_nc_f1*/
    public nc_f1() {
    }
    /** c1_nc_l1*/
    public nc_l1: () => void;
}
let i1_i: i1;
AssertType(i1_i, "i1");

let c1_i = new c1();
AssertType(c1_i, "c1");
AssertType(new c1(), "c1");
AssertType(c1, "typeof c1");

// assign to interface
i1_i = c1_i;
AssertType(i1_i = c1_i, "c1");
AssertType(i1_i, "i1");
AssertType(c1_i, "c1");

class c2 {
    /** c2 c2_p1*/
    public c2_p1: number;
    /** c2 c2_f1*/
    public c2_f1() {
    }
    /** c2 c2_prop*/
    public get c2_prop() {
AssertType(10, "int");
        return 10;
    }
    public c2_nc_p1: number;
    public c2_nc_f1() {
    }
    public get c2_nc_prop() {
AssertType(10, "int");
        return 10;
    }
    /** c2 p1*/
    public p1: number;
    /** c2 f1*/
    public f1() {
    }
    /** c2 prop*/
    public get prop() {
AssertType(10, "int");
        return 10;
    }
    public nc_p1: number;
    public nc_f1() {
    }
    public get nc_prop() {
AssertType(10, "int");
        return 10;
    }
    /** c2 constructor*/
    constructor(a: number) {
        this.c2_p1 = a;
AssertType(this.c2_p1 = a, "number");
AssertType(this.c2_p1, "number");
AssertType(this, "this");
AssertType(a, "number");
    }
}
class c3 extends c2 {
    constructor() {
        super(10);
AssertType(super(10), "void");
AssertType(super, "typeof c2");
AssertType(10, "int");
    }
    /** c3 p1*/
    public p1: number;
    /** c3 f1*/
    public f1() {
    }
    /** c3 prop*/
    public get prop() {
AssertType(10, "int");
        return 10;
    }
    public nc_p1: number;
    public nc_f1() {
    }
    public get nc_prop() {
AssertType(10, "int");
        return 10;
    }
}
let c2_i = new c2(10);
AssertType(c2_i, "c2");
AssertType(new c2(10), "c2");
AssertType(c2, "typeof c2");
AssertType(10, "int");

let c3_i = new c3();
AssertType(c3_i, "c3");
AssertType(new c3(), "c3");
AssertType(c3, "typeof c3");

// assign
c2_i = c3_i;
AssertType(c2_i = c3_i, "c3");
AssertType(c2_i, "c2");
AssertType(c3_i, "c3");

class c4 extends c2 {
}
let c4_i = new c4(10);
AssertType(c4_i, "c4");
AssertType(new c4(10), "c4");
AssertType(c4, "typeof c4");
AssertType(10, "int");

interface i2 {
    /** i2_p1*/
    i2_p1: number;
    /** i2_f1*/
    i2_f1(): void;
    /** i2_l1*/
    i2_l1: () => void;
    // i2_nc_p1
    i2_nc_p1: number;
    i2_nc_f1(): void;
    i2_nc_l1: () => void;
    /** i2 p1*/
    p1: number;
    /** i2 f1*/
    f1(): void;
    /** i2 l1*/
    l1: () => void;
    nc_p1: number;
    nc_f1(): void;
    nc_l1: () => void;
}
interface i3 extends i2 {
    /** i3 p1 */
    p1: number;
    /**
    * i3 f1
    */
    f1(): void;
    /** i3 l1*/
    l1: () => void;
    nc_p1: number;
    nc_f1(): void;
    nc_l1: () => void;
}
let i2_i: i2;
AssertType(i2_i, "i2");

let i3_i: i3;
AssertType(i3_i, "i3");

// assign to interface
i2_i = i3_i;
AssertType(i2_i = i3_i, "i3");
AssertType(i2_i, "i2");
AssertType(i3_i, "i3");


