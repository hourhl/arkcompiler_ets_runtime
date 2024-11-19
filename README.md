## 改进 - B型指令（beq，bne，blt）的实现和测试 

### 相关的Issue

#I8NXZG:适配ark编译器的riscv汇编assembler内容

### 原因（目的、解决的问题等）

增加了assembler中RISCV基本指令集的部分B型指令（包括beq，bne和blt）

### 描述（做了什么，变更了什么）

修改了ecmascript/compiler/assembler/riscv64/assembler_riscv64.h | ecmascript/compiler/assembler/riscv64/assembler_riscv64_constants.h |
ecmascript/compiler/assembler/tests/assembler_riscv64_test.cpp

### 测试用例（新增、改动、可能影响的功能）

测试代码位于 ecmascript/compiler/assembler/tests/assembler_riscv64_test.cpp
运行以下指令无报错
build.py --product-name rk3568 --build-target ark_compiler_unittest
