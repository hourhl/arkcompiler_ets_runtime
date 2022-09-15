/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OLD_INSTRUCTION_ENUM_H
#define OLD_INSTRUCTION_ENUM_H

enum class Format : uint8_t {
    ID16,
    ID32,
    IMM16,
    IMM16_V16,
    IMM32,
    IMM4_V4_V4_V4,
    IMM4_V4_V4_V4_V4_V4,
    IMM64,
    IMM8,
    NONE,
    PREF_ID16_IMM16_IMM16_V8_V8,
    PREF_ID16_IMM16_V8,
    PREF_ID32,
    PREF_ID32_IMM8,
    PREF_ID32_V8,
    PREF_IMM16,
    PREF_IMM16_IMM16,
    PREF_IMM16_IMM16_V8,
    PREF_IMM16_V8,
    PREF_IMM16_V8_V8,
    PREF_IMM32,
    PREF_IMM4_IMM4,
    PREF_IMM4_IMM4_V8,
    PREF_IMM8_IMM8,
    PREF_IMM8_IMM8_V8,
    PREF_NONE,
    PREF_V4_V4,
    PREF_V8,
    PREF_V8_IMM32,
    PREF_V8_V8,
    PREF_V8_V8_V8,
    PREF_V8_V8_V8_V8,
    V16_V16,
    V4_IMM4,
    V4_IMM4_ID16,
    V4_V4,
    V4_V4_ID16,
    V4_V4_V4_IMM4_ID16,
    V4_V4_V4_V4_ID16,
    V8,
    V8_ID16,
    V8_ID32,
    V8_IMM16,
    V8_IMM32,
    V8_IMM64,
    V8_IMM8,
    V8_V8,
};

enum class Opcode {
    NOP = 0,
    MOV_V4_V4 = 1,
    MOV_V8_V8 = 2,
    MOV_V16_V16 = 3,
    MOV_64_V4_V4 = 4,
    MOV_64_V16_V16 = 5,
    MOV_OBJ_V4_V4 = 6,
    MOV_OBJ_V8_V8 = 7,
    MOV_OBJ_V16_V16 = 8,
    MOVI_V4_IMM4 = 9,
    MOVI_V8_IMM8 = 10,
    MOVI_V8_IMM16 = 11,
    MOVI_V8_IMM32 = 12,
    MOVI_64_V8_IMM64 = 13,
    FMOVI_64_V8_IMM64 = 14,
    MOV_NULL_V8 = 15,
    LDA_V8 = 16,
    LDA_64_V8 = 17,
    LDA_OBJ_V8 = 18,
    LDAI_IMM8 = 19,
    LDAI_IMM16 = 20,
    LDAI_IMM32 = 21,
    LDAI_64_IMM64 = 22,
    FLDAI_64_IMM64 = 23,
    LDA_STR_ID32 = 24,
    LDA_CONST_V8_ID32 = 25,
    LDA_TYPE_ID16 = 26,
    LDA_NULL = 27,
    STA_V8 = 28,
    STA_64_V8 = 29,
    STA_OBJ_V8 = 30,
    CMP_64_V8 = 31,
    FCMPL_64_V8 = 32,
    FCMPG_64_V8 = 33,
    JMP_IMM8 = 34,
    JMP_IMM16 = 35,
    JMP_IMM32 = 36,
    JEQ_OBJ_V8_IMM8 = 37,
    JEQ_OBJ_V8_IMM16 = 38,
    JNE_OBJ_V8_IMM8 = 39,
    JNE_OBJ_V8_IMM16 = 40,
    JEQZ_OBJ_IMM8 = 41,
    JEQZ_OBJ_IMM16 = 42,
    JNEZ_OBJ_IMM8 = 43,
    JNEZ_OBJ_IMM16 = 44,
    JEQZ_IMM8 = 45,
    JEQZ_IMM16 = 46,
    JNEZ_IMM8 = 47,
    JNEZ_IMM16 = 48,
    JLTZ_IMM8 = 49,
    JLTZ_IMM16 = 50,
    JGTZ_IMM8 = 51,
    JGTZ_IMM16 = 52,
    JLEZ_IMM8 = 53,
    JLEZ_IMM16 = 54,
    JGEZ_IMM8 = 55,
    JGEZ_IMM16 = 56,
    JEQ_V8_IMM8 = 57,
    JEQ_V8_IMM16 = 58,
    JNE_V8_IMM8 = 59,
    JNE_V8_IMM16 = 60,
    JLT_V8_IMM8 = 61,
    JLT_V8_IMM16 = 62,
    JGT_V8_IMM8 = 63,
    JGT_V8_IMM16 = 64,
    JLE_V8_IMM8 = 65,
    JLE_V8_IMM16 = 66,
    JGE_V8_IMM8 = 67,
    JGE_V8_IMM16 = 68,
    FNEG_64 = 69,
    NEG = 70,
    NEG_64 = 71,
    ADD2_V8 = 72,
    ADD2_64_V8 = 73,
    SUB2_V8 = 74,
    SUB2_64_V8 = 75,
    MUL2_V8 = 76,
    MUL2_64_V8 = 77,
    FADD2_64_V8 = 78,
    FSUB2_64_V8 = 79,
    FMUL2_64_V8 = 80,
    FDIV2_64_V8 = 81,
    FMOD2_64_V8 = 82,
    DIV2_V8 = 83,
    DIV2_64_V8 = 84,
    MOD2_V8 = 85,
    MOD2_64_V8 = 86,
    ADDI_IMM8 = 87,
    SUBI_IMM8 = 88,
    MULI_IMM8 = 89,
    ANDI_IMM32 = 90,
    ORI_IMM32 = 91,
    SHLI_IMM8 = 92,
    SHRI_IMM8 = 93,
    ASHRI_IMM8 = 94,
    DIVI_IMM8 = 95,
    MODI_IMM8 = 96,
    ADD_V4_V4 = 97,
    SUB_V4_V4 = 98,
    MUL_V4_V4 = 99,
    DIV_V4_V4 = 100,
    MOD_V4_V4 = 101,
    INCI_V4_IMM4 = 102,
    LDARR_8_V8 = 103,
    LDARRU_8_V8 = 104,
    LDARR_16_V8 = 105,
    LDARRU_16_V8 = 106,
    LDARR_V8 = 107,
    LDARR_64_V8 = 108,
    FLDARR_32_V8 = 109,
    FLDARR_64_V8 = 110,
    LDARR_OBJ_V8 = 111,
    STARR_8_V4_V4 = 112,
    STARR_16_V4_V4 = 113,
    STARR_V4_V4 = 114,
    STARR_64_V4_V4 = 115,
    FSTARR_32_V4_V4 = 116,
    FSTARR_64_V4_V4 = 117,
    STARR_OBJ_V4_V4 = 118,
    LENARR_V8 = 119,
    NEWARR_V4_V4_ID16 = 120,
    NEWOBJ_V8_ID16 = 121,
    INITOBJ_SHORT_V4_V4_ID16 = 122,
    INITOBJ_V4_V4_V4_V4_ID16 = 123,
    INITOBJ_RANGE_V8_ID16 = 124,
    LDOBJ_V8_ID16 = 125,
    LDOBJ_64_V8_ID16 = 126,
    LDOBJ_OBJ_V8_ID16 = 127,
    STOBJ_V8_ID16 = 128,
    STOBJ_64_V8_ID16 = 129,
    STOBJ_OBJ_V8_ID16 = 130,
    LDOBJ_V_V4_V4_ID16 = 131,
    LDOBJ_V_64_V4_V4_ID16 = 132,
    LDOBJ_V_OBJ_V4_V4_ID16 = 133,
    STOBJ_V_V4_V4_ID16 = 134,
    STOBJ_V_64_V4_V4_ID16 = 135,
    STOBJ_V_OBJ_V4_V4_ID16 = 136,
    LDSTATIC_ID16 = 137,
    LDSTATIC_64_ID16 = 138,
    LDSTATIC_OBJ_ID16 = 139,
    STSTATIC_ID16 = 140,
    STSTATIC_64_ID16 = 141,
    STSTATIC_OBJ_ID16 = 142,
    RETURN = 143,
    RETURN_64 = 144,
    RETURN_OBJ = 145,
    RETURN_VOID = 146,
    THROW_V8 = 147,
    CHECKCAST_ID16 = 148,
    ISINSTANCE_ID16 = 149,
    CALL_SHORT_V4_V4_ID16 = 150,
    CALL_V4_V4_V4_V4_ID16 = 151,
    CALL_RANGE_V8_ID16 = 152,
    CALL_ACC_SHORT_V4_IMM4_ID16 = 153,
    CALL_ACC_V4_V4_V4_IMM4_ID16 = 154,
    CALL_VIRT_SHORT_V4_V4_ID16 = 155,
    CALL_VIRT_V4_V4_V4_V4_ID16 = 156,
    CALL_VIRT_RANGE_V8_ID16 = 157,
    CALL_VIRT_ACC_SHORT_V4_IMM4_ID16 = 158,
    CALL_VIRT_ACC_V4_V4_V4_IMM4_ID16 = 159,
    MOV_DYN_V8_V8 = 160,
    MOV_DYN_V16_V16 = 161,
    LDA_DYN_V8 = 162,
    STA_DYN_V8 = 163,
    LDAI_DYN_IMM32 = 164,
    FLDAI_DYN_IMM64 = 165,
    RETURN_DYN = 166,
    CALLI_DYN_SHORT_IMM4_V4_V4_V4 = 167,
    CALLI_DYN_IMM4_V4_V4_V4_V4_V4 = 168,
    CALLI_DYN_RANGE_IMM16_V16 = 169,
    FMOVI_PREF_V8_IMM32 = 236,
    I32TOF64_PREF_NONE = 237,
    UCMP_PREF_V8 = 238,
    NOT_PREF_NONE = 239,
    ECMA_LDNAN_PREF_NONE = 255,
    FLDAI_PREF_IMM32 = 492,
    U32TOF64_PREF_NONE = 493,
    UCMP_64_PREF_V8 = 494,
    NOT_64_PREF_NONE = 495,
    ECMA_LDINFINITY_PREF_NONE = 511,
    FCMPL_PREF_V8 = 748,
    I64TOF64_PREF_NONE = 749,
    DIVU2_PREF_V8 = 750,
    AND2_PREF_V8 = 751,
    ECMA_LDGLOBALTHIS_PREF_NONE = 767,
    FCMPG_PREF_V8 = 1004,
    U64TOF64_PREF_NONE = 1005,
    DIVU2_64_PREF_V8 = 1006,
    AND2_64_PREF_V8 = 1007,
    ECMA_LDUNDEFINED_PREF_NONE = 1023,
    FNEG_PREF_NONE = 1260,
    F64TOI32_PREF_NONE = 1261,
    MODU2_PREF_V8 = 1262,
    OR2_PREF_V8 = 1263,
    ECMA_LDNULL_PREF_NONE = 1279,
    FADD2_PREF_V8 = 1516,
    F64TOI64_PREF_NONE = 1517,
    MODU2_64_PREF_V8 = 1518,
    OR2_64_PREF_V8 = 1519,
    ECMA_LDSYMBOL_PREF_NONE = 1535,
    FSUB2_PREF_V8 = 1772,
    F64TOU32_PREF_NONE = 1773,
    XOR2_PREF_V8 = 1775,
    ECMA_LDGLOBAL_PREF_NONE = 1791,
    FMUL2_PREF_V8 = 2028,
    F64TOU64_PREF_NONE = 2029,
    XOR2_64_PREF_V8 = 2031,
    ECMA_LDTRUE_PREF_NONE = 2047,
    FDIV2_PREF_V8 = 2284,
    I32TOU1_PREF_NONE = 2285,
    SHL2_PREF_V8 = 2287,
    ECMA_LDFALSE_PREF_NONE = 2303,
    FMOD2_PREF_V8 = 2540,
    I64TOU1_PREF_NONE = 2541,
    SHL2_64_PREF_V8 = 2543,
    ECMA_THROWDYN_PREF_NONE = 2559,
    I32TOF32_PREF_NONE = 2796,
    U32TOU1_PREF_NONE = 2797,
    SHR2_PREF_V8 = 2799,
    ECMA_TYPEOFDYN_PREF_NONE = 2815,
    U32TOF32_PREF_NONE = 3052,
    U64TOU1_PREF_NONE = 3053,
    SHR2_64_PREF_V8 = 3055,
    ECMA_LDLEXENVDYN_PREF_NONE = 3071,
    I64TOF32_PREF_NONE = 3308,
    I32TOI64_PREF_NONE = 3309,
    ASHR2_PREF_V8 = 3311,
    ECMA_POPLEXENVDYN_PREF_NONE = 3327,
    U64TOF32_PREF_NONE = 3564,
    I32TOI16_PREF_NONE = 3565,
    ASHR2_64_PREF_V8 = 3567,
    ECMA_GETUNMAPPEDARGS_PREF_NONE = 3583,
    F32TOF64_PREF_NONE = 3820,
    I32TOU16_PREF_NONE = 3821,
    XORI_PREF_IMM32 = 3823,
    ECMA_GETPROPITERATOR_PREF_NONE = 3839,
    F32TOI32_PREF_NONE = 4076,
    I32TOI8_PREF_NONE = 4077,
    AND_PREF_V4_V4 = 4079,
    ECMA_ASYNCFUNCTIONENTER_PREF_NONE = 4095,
    F32TOI64_PREF_NONE = 4332,
    I32TOU8_PREF_NONE = 4333,
    OR_PREF_V4_V4 = 4335,
    ECMA_LDHOLE_PREF_NONE = 4351,
    F32TOU32_PREF_NONE = 4588,
    I64TOI32_PREF_NONE = 4589,
    XOR_PREF_V4_V4 = 4591,
    ECMA_RETURNUNDEFINED_PREF_NONE = 4607,
    F32TOU64_PREF_NONE = 4844,
    U32TOI64_PREF_NONE = 4845,
    SHL_PREF_V4_V4 = 4847,
    ECMA_CREATEEMPTYOBJECT_PREF_NONE = 4863,
    F64TOF32_PREF_NONE = 5100,
    U32TOI16_PREF_NONE = 5101,
    SHR_PREF_V4_V4 = 5103,
    ECMA_CREATEEMPTYARRAY_PREF_NONE = 5119,
    U32TOU16_PREF_NONE = 5357,
    ASHR_PREF_V4_V4 = 5359,
    ECMA_GETITERATOR_PREF_NONE = 5375,
    U32TOI8_PREF_NONE = 5613,
    ECMA_THROWTHROWNOTEXISTS_PREF_NONE = 5631,
    U32TOU8_PREF_NONE = 5869,
    ECMA_THROWPATTERNNONCOERCIBLE_PREF_NONE = 5887,
    U64TOI32_PREF_NONE = 6125,
    ECMA_LDHOMEOBJECT_PREF_NONE = 6143,
    U64TOU32_PREF_NONE = 6381,
    ECMA_THROWDELETESUPERPROPERTY_PREF_NONE = 6399,
    ECMA_DEBUGGER_PREF_NONE = 6655,
    ECMA_ADD2DYN_PREF_V8 = 6911,
    ECMA_SUB2DYN_PREF_V8 = 7167,
    ECMA_MUL2DYN_PREF_V8 = 7423,
    ECMA_DIV2DYN_PREF_V8 = 7679,
    ECMA_MOD2DYN_PREF_V8 = 7935,
    ECMA_EQDYN_PREF_V8 = 8191,
    ECMA_NOTEQDYN_PREF_V8 = 8447,
    ECMA_LESSDYN_PREF_V8 = 8703,
    ECMA_LESSEQDYN_PREF_V8 = 8959,
    ECMA_GREATERDYN_PREF_V8 = 9215,
    ECMA_GREATEREQDYN_PREF_V8 = 9471,
    ECMA_SHL2DYN_PREF_V8 = 9727,
    ECMA_SHR2DYN_PREF_V8 = 9983,
    ECMA_ASHR2DYN_PREF_V8 = 10239,
    ECMA_AND2DYN_PREF_V8 = 10495,
    ECMA_OR2DYN_PREF_V8 = 10751,
    ECMA_XOR2DYN_PREF_V8 = 11007,
    ECMA_TONUMBER_PREF_V8 = 11263,
    ECMA_NEGDYN_PREF_V8 = 11519,
    ECMA_NOTDYN_PREF_V8 = 11775,
    ECMA_INCDYN_PREF_V8 = 12031,
    ECMA_DECDYN_PREF_V8 = 12287,
    ECMA_EXPDYN_PREF_V8 = 12543,
    ECMA_ISINDYN_PREF_V8 = 12799,
    ECMA_INSTANCEOFDYN_PREF_V8 = 13055,
    ECMA_STRICTNOTEQDYN_PREF_V8 = 13311,
    ECMA_STRICTEQDYN_PREF_V8 = 13567,
    ECMA_RESUMEGENERATOR_PREF_V8 = 13823,
    ECMA_GETRESUMEMODE_PREF_V8 = 14079,
    ECMA_CREATEGENERATOROBJ_PREF_V8 = 14335,
    ECMA_THROWCONSTASSIGNMENT_PREF_V8 = 14591,
    ECMA_GETTEMPLATEOBJECT_PREF_V8 = 14847,
    ECMA_GETNEXTPROPNAME_PREF_V8 = 15103,
    ECMA_CALLARG0DYN_PREF_V8 = 15359,
    ECMA_THROWIFNOTOBJECT_PREF_V8 = 15615,
    ECMA_ITERNEXT_PREF_V8 = 15871,
    ECMA_CLOSEITERATOR_PREF_V8 = 16127,
    ECMA_COPYMODULE_PREF_V8 = 16383,
    ECMA_SUPERCALLSPREAD_PREF_V8 = 16639,
    ECMA_DELOBJPROP_PREF_V8_V8 = 16895,
    ECMA_NEWOBJSPREADDYN_PREF_V8_V8 = 17151,
    ECMA_CREATEITERRESULTOBJ_PREF_V8_V8 = 17407,
    ECMA_SUSPENDGENERATOR_PREF_V8_V8 = 17663,
    ECMA_ASYNCFUNCTIONAWAITUNCAUGHT_PREF_V8_V8 = 17919,
    ECMA_THROWUNDEFINEDIFHOLE_PREF_V8_V8 = 18175,
    ECMA_CALLARG1DYN_PREF_V8_V8 = 18431,
    ECMA_COPYDATAPROPERTIES_PREF_V8_V8 = 18687,
    ECMA_STARRAYSPREAD_PREF_V8_V8 = 18943,
    ECMA_GETITERATORNEXT_PREF_V8_V8 = 19199,
    ECMA_SETOBJECTWITHPROTO_PREF_V8_V8 = 19455,
    ECMA_LDOBJBYVALUE_PREF_V8_V8 = 19711,
    ECMA_STOBJBYVALUE_PREF_V8_V8 = 19967,
    ECMA_STOWNBYVALUE_PREF_V8_V8 = 20223,
    ECMA_LDSUPERBYVALUE_PREF_V8_V8 = 20479,
    ECMA_STSUPERBYVALUE_PREF_V8_V8 = 20735,
    ECMA_LDOBJBYINDEX_PREF_V8_IMM32 = 20991,
    ECMA_STOBJBYINDEX_PREF_V8_IMM32 = 21247,
    ECMA_STOWNBYINDEX_PREF_V8_IMM32 = 21503,
    ECMA_CALLSPREADDYN_PREF_V8_V8_V8 = 21759,
    ECMA_ASYNCFUNCTIONRESOLVE_PREF_V8_V8_V8 = 22015,
    ECMA_ASYNCFUNCTIONREJECT_PREF_V8_V8_V8 = 22271,
    ECMA_CALLARGS2DYN_PREF_V8_V8_V8 = 22527,
    ECMA_CALLARGS3DYN_PREF_V8_V8_V8_V8 = 22783,
    ECMA_DEFINEGETTERSETTERBYVALUE_PREF_V8_V8_V8_V8 = 23039,
    ECMA_NEWOBJDYNRANGE_PREF_IMM16_V8 = 23295,
    ECMA_CALLRANGEDYN_PREF_IMM16_V8 = 23551,
    ECMA_CALLTHISRANGEDYN_PREF_IMM16_V8 = 23807,
    ECMA_SUPERCALL_PREF_IMM16_V8 = 24063,
    ECMA_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8 = 24319,
    ECMA_DEFINEFUNCDYN_PREF_ID16_IMM16_V8 = 24575,
    ECMA_DEFINENCFUNCDYN_PREF_ID16_IMM16_V8 = 24831,
    ECMA_DEFINEGENERATORFUNC_PREF_ID16_IMM16_V8 = 25087,
    ECMA_DEFINEASYNCFUNC_PREF_ID16_IMM16_V8 = 25343,
    ECMA_DEFINEMETHOD_PREF_ID16_IMM16_V8 = 25599,
    ECMA_NEWLEXENVDYN_PREF_IMM16 = 25855,
    ECMA_COPYRESTARGS_PREF_IMM16 = 26111,
    ECMA_CREATEARRAYWITHBUFFER_PREF_IMM16 = 26367,
    ECMA_CREATEOBJECTHAVINGMETHOD_PREF_IMM16 = 26623,
    ECMA_THROWIFSUPERNOTCORRECTCALL_PREF_IMM16 = 26879,
    ECMA_CREATEOBJECTWITHBUFFER_PREF_IMM16 = 27135,
    ECMA_LDLEXVARDYN_PREF_IMM4_IMM4 = 27391,
    ECMA_LDLEXVARDYN_PREF_IMM8_IMM8 = 27647,
    ECMA_LDLEXVARDYN_PREF_IMM16_IMM16 = 27903,
    ECMA_STLEXVARDYN_PREF_IMM4_IMM4_V8 = 28159,
    ECMA_STLEXVARDYN_PREF_IMM8_IMM8_V8 = 28415,
    ECMA_STLEXVARDYN_PREF_IMM16_IMM16_V8 = 28671,
    ECMA_DEFINECLASSWITHBUFFER_PREF_ID16_IMM16_IMM16_V8_V8 = 28927,
    ECMA_GETMODULENAMESPACE_PREF_ID32 = 29183,
    ECMA_STMODULEVAR_PREF_ID32 = 29439,
    ECMA_TRYLDGLOBALBYNAME_PREF_ID32 = 29695,
    ECMA_TRYSTGLOBALBYNAME_PREF_ID32 = 29951,
    ECMA_LDGLOBALVAR_PREF_ID32 = 30207,
    ECMA_STGLOBALVAR_PREF_ID32 = 30463,
    ECMA_LDOBJBYNAME_PREF_ID32_V8 = 30719,
    ECMA_STOBJBYNAME_PREF_ID32_V8 = 30975,
    ECMA_STOWNBYNAME_PREF_ID32_V8 = 31231,
    ECMA_LDSUPERBYNAME_PREF_ID32_V8 = 31487,
    ECMA_STSUPERBYNAME_PREF_ID32_V8 = 31743,
    ECMA_LDMODULEVAR_PREF_ID32_IMM8 = 31999,
    ECMA_CREATEREGEXPWITHLITERAL_PREF_ID32_IMM8 = 32255,
    ECMA_ISTRUE_PREF_NONE = 32511,
    ECMA_ISFALSE_PREF_NONE = 32767,
    ECMA_STCONSTTOGLOBALRECORD_PREF_ID32 = 33023,
    ECMA_STLETTOGLOBALRECORD_PREF_ID32 = 33279,
    ECMA_STCLASSTOGLOBALRECORD_PREF_ID32 = 33535,
    ECMA_STOWNBYVALUEWITHNAMESET_PREF_V8_V8 = 33791,
    ECMA_STOWNBYNAMEWITHNAMESET_PREF_ID32_V8 = 34047,
    ECMA_LDFUNCTION_PREF_NONE = 34303,
    ECMA_NEWLEXENVWITHNAMEDYN_PREF_IMM16_IMM16 = 34559,
    ECMA_LDBIGINT_PREF_ID32 = 34815,
    ECMA_TONUMERIC_PREF_V8 = 35071,
    ECMA_CREATEASYNCGENERATOROBJ_PREF_V8 = 35327,
    ECMA_ASYNCGENERATORRESOLVE_PREF_V8_V8_V8 = 35583,
    ECMA_DEFINEASYNCGENERATORFUNC_PREF_ID16_IMM16_V8 = 35839,
    ECMA_DYNAMICIMPORT_PREF_V8 = 36095,
    ECMA_LDPATCHVAR_PREF_IMM16 = 36351,
    ECMA_STPATCHVAR_PREF_IMM16 = 36607,
    ECMA_ASYNCGENERATORREJECT_PREF_V8_V8 = 36863,
    LAST = ECMA_ASYNCGENERATORREJECT_PREF_V8_V8
};

enum Flags : uint32_t {
    TYPE_ID = 0x1,
    METHOD_ID = 0x2,
    STRING_ID = 0x4,
    LITERALARRAY_ID = 0x8,
    FIELD_ID = 0x10,
    CALL = 0x20,
    CALL_VIRT = 0x40,
    RETURN = 0x80,
    SUSPEND = 0x100,
    JUMP = 0x200,
    CONDITIONAL = 0x400,
    FLOAT = 0x800,
    DYNAMIC = 0x1000,
    MAYBE_DYNAMIC = 0x2000,
    LANGUAGE_TYPE = 0x4000,
    INITIALIZE_TYPE = 0x8000,
    ACC_NONE = 0x10000,
    ACC_READ = 0x20000,
    ACC_WRITE = 0x40000,
};

#endif  // OLD_INSTRUCTION_ENUM_H
