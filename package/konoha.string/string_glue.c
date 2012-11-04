/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

/* ************************************************************************ */

#define USE_STRINGLIB 1

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>

#define _Public kMethod_Public
#define _Static kMethod_Static
#define _Const  kMethod_Const
#define _JS     kMethod_JSCompatible
#define _Im     kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#include "string_api.h"
#include "rope_string.h"
#include "java_string.h"

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------------ */

//## boolean String.<(String s);
static KMETHOD String_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res < 0);
}

//## boolean String.<=(String s);
static KMETHOD String_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res <= 0);
}

//## boolean String.>(String s);
static KMETHOD String_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res > 0);
}

//## boolean String.>=(String s);
static KMETHOD String_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res >= 0);
}

//## String String.get(int index);
static KMETHOD String_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	size_t offset = sfp[1].intValue;
	if(offset >= S_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[1].intValue);
	}
	kString *ret = (kString_is(ASCII, self)) ?
		KLIB new_kString(kctx, OnStack, S_text(self) + offset, 1, StringPolicy_ASCII) :
		new_UTF8SubString(kctx, self, offset, 1);
	KReturn(ret);
}

//## String String.fromCharCode(int charCode);
static KMETHOD String_fromCharCode(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t c = sfp[1].intValue;
	if(c < 0x0000 || c > 0x10FFFF) { /* FIXME: out of unicode range */
		KReturn(KNULL(String));
	}
	char buf[5] = {0};
	size_t length = 0;
	int policy = 0;
	if(c <= 0x007F) { /* 1 byte */
		buf[0] = (char)c;
		length = 1;
		policy |= StringPolicy_ASCII;
	}
	else if(c <= 0x07FF) { /* 2 bytes */
		buf[0] = (char)(0xC0 | (c >> 6));
		buf[1] = (char)(0x80 | (c & 0x3F));
		length = 2;
		policy |= StringPolicy_UTF8;
	}
	else if(c <= 0xFFFF) { /* 3 bytes */
		buf[0] = (char)(0xE0 | (c >> 12));
		buf[1] = (char)(0x80 | ((c >> 6) & 0x3F));
		buf[2] = (char)(0x80 | (c & 0x3F));
		length = 3;
		policy |= StringPolicy_UTF8;
	}
	else { /* 4 bytes */
		buf[0] = (char)(0xF0 | (c >> 18));
		buf[1] = (char)(0x80 | ((c >> 12) & 0x3F));
		buf[2] = (char)(0x80 | ((c >> 6) & 0x3F));
		buf[3] = (char)(0x80 | (c & 0x3F));
		length = 4;
		policy |= StringPolicy_UTF8;
	}
	KReturn(KLIB new_kString(kctx, OnStack, (const char *)buf, length, policy));
}

// --------------------------------------------------------------------------
// String Interpolation

/* copied from src/sugar/sugarfunc.h */
static kString *kToken_resolvedEscapeSequence(KonohaContext *kctx, kToken *tk, size_t start)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	const char *text = S_text(tk->text) + start;
	const char *end  = S_text(tk->text) + S_size(tk->text);
	KLIB Kwb_write(kctx, &wb, S_text(tk->text), start);
	while(text < end) {
		int ch = *text;
		if(ch == '\\' && *(text+1) != '\0') {
			switch (*(text+1)) {
			/*
			 * compatible with ECMA-262
			 * http://ecma-international.org/ecma-262/5.1/#sec-7.8.4
			 */
			case 'b':  ch = '\b'; text++; break;
			case 't':  ch = '\t'; text++; break;
			case 'n':  ch = '\n'; text++; break;
			case 'v':  ch = '\v'; text++; break;
			case 'f':  ch = '\f'; text++; break;
			case 'r':  ch = '\r'; text++; break;
			case '"':  ch = '"';  text++; break;
			case '\'': ch = '\''; text++; break;
			case '\\': ch = '\\'; text++; break;
			default: return NULL;
			}
		}
		{
			char buf[1] = {ch};
			KLIB Kwb_write(kctx, &wb, (const char *)buf, 1);
		}
		text++;
	}
	kString *s = KLIB new_kString(kctx, OnGcStack, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	KLIB Kwb_free(&wb);
	return s;
}

static kString *remove_escapes(KonohaContext *kctx, kToken *tk)
{
	kString *text = tk->text;
	if(kToken_is(RequiredReformat, tk)) {
		const char *escape = strchr(S_text(text), '\\');
		DBG_ASSERT(escape != NULL);
		text = kToken_resolvedEscapeSequence(kctx, tk, escape - S_text(text));
	}
	return text;
}

static KMETHOD TypeCheck_ExtendedTextLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken  *tk   = expr->termToken;
	INIT_GCSTACK();
	kString *text = remove_escapes(kctx, tk);
	if(text == NULL) {
		KReturnWith(K_NULLEXPR, RESET_GCSTACK());
	}

	const char *start = NULL, *end = NULL;
	const char *str = S_text(text);
	expr = SUGAR kExpr_setConstValue(kctx, expr, TY_String, UPCAST(text));
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kMethod *concat = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, TY_String, MN_("+"), 1);

	expr = new_ConstValueExpr(kctx, TY_String, UPCAST(TS_EMPTY));
	while(true) {
		start = strstr(str, "${");
		if(start == NULL) {
			break;
		}
		if(start == strstr(str, "${}")) {
			str += 3;
			continue;
		}
		end = strchr(start, '}');
		if(end == NULL) {
			break;
		}

		KGrowingBuffer wb;
		KLIB Kwb_init(&(kctx->stack->cwb), &wb);
		KLIB Kwb_write(kctx, &wb, "(", 1);
		KLIB Kwb_write(kctx, &wb, start+2, end-(start+2));
		KLIB Kwb_write(kctx, &wb, ")", 1);

		TokenSequence range = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
		TokenSequence_push(kctx, range);
		const char *buf = KLIB Kwb_top(kctx, &wb, 1);
		SUGAR TokenSequence_tokenize(kctx, &range, buf, 0);

		{
			TokenSequence tokens = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
			TokenSequence_push(kctx, tokens);
			SUGAR TokenSequence_resolved(kctx, &tokens, NULL, &range, range.beginIdx);
			/* +1 means for skiping first indent token. */
			kExpr *newexpr = SUGAR kStmt_parseExpr(kctx, stmt, tokens.tokenList, tokens.beginIdx+1, tokens.endIdx, NULL);
			TokenSequence_pop(kctx, tokens);

			if(start - str > 0) {
				kExpr *first = new_ConstValueExpr(kctx, TY_String,
						UPCAST(KLIB new_kString(kctx, OnGcStack, str, (start - str), 0)));
				expr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_String, concat, 2, expr, first);
			}
			expr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_String, concat, 2, expr, newexpr);
		}
		TokenSequence_pop(kctx, range);
		KLIB Kwb_free(&wb);
		str = end + 1;
	}

	if((start == NULL) || (start != NULL && end == NULL)) {
		kExpr *rest = new_ConstValueExpr(kctx, TY_String,
				UPCAST(KLIB new_kString(kctx, OnGcStack, str, strlen(str), 0)));
		expr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_String, concat, 2, expr, rest);
	}
	KReturnWith(expr, RESET_GCSTACK());
}


// --------------------------------------------------------------------------


static kbool_t string_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	LoadRopeMethod(kctx, ns, trace);
	int FN_s = FN_("s");
	int FN_n = FN_("n");

	KDEFINE_SYNTAX SYNTAX[] = {
		{ KW_TextPattern, 0,  NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_ExtendedTextLiteral, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, trace);

	KDEFINE_METHOD MethodData[] = {
		_JS|_Public|_Const|_Im, _F(String_opLT),  TY_boolean, TY_String, MN_("<"),  1, TY_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opLTE),  TY_boolean, TY_String, MN_("<="),  1, TY_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opGT),  TY_boolean, TY_String, MN_(">"),  1, TY_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opGTE),  TY_boolean, TY_String, MN_(">="),  1, TY_String, FN_s,
		_JS|_Public|_Const|_Im, _F(KString_length), TY_int, TY_String, MN_("getlength"), 0,
		_JS|_Public|_Const|_Im, _F(String_get), TY_String, TY_String, MN_("charAt"), 1, TY_int, FN_n,
		_JS|_Public|_Static|_Const|_Im, _F(String_fromCharCode), TY_String, TY_String, MN_("fromCharCode"), 1, TY_int, FN_n,
		_Public|_Const|_Im, _F(String_get),        TY_String, TY_String, MN_("get"), 1, TY_int, FN_n,
		_JS|_Public|_Const|_Im, _F(KString_charAt), TY_int, TY_String, MN_("charCodeAt"), 1, TY_int, FN_n,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	/**
MiniKonoha 0.2 (Old Riga) (0b9d1831, Nov  4 2012)
[gcc 4.2.1 (Based on Apple Inc. build 5658) (LLVM build 2336.9.00)]
options: minivm sugar
>>> import("konoha.string")
Reading symbols for shared libraries . done

Program received signal EXC_BAD_ACCESS, Could not access memory.
Reason: KERN_INVALID_ADDRESS at address: 0x00000000000000e0
0x000000010001c897 in KonohaClass_shortName (kctx=0x100100fa8, ct=0x0) at datatype.h:793
793		if(ct->shortClassNameNULL_OnGlobalConstList == NULL) {
(gdb) bt
#0  0x000000010001c897 in KonohaClass_shortName (kctx=0x100100fa8, ct=0x0) at datatype.h:793
#1  0x000000010002660f in CT_s_ (kctx=0x100100fa8, ct=0x0) at klib.h:81
#2  0x00000001000266aa in TY_s_ (kctx=0x100100fa8, ty=118) at klib.h:89
#3  0x000000010002e23d in kNameSpace_AddMethod (kctx=0x100100fa8, ns=0x101022b00, mtd=0x101023c80, trace=0x7fff5fbfeff8)
    at namespace.h:792
#4  0x000000010002f206 in kNameSpace_LoadMethodData (kctx=0x100100fa8, ns=0x101022b00, data=0x7fff5fbfe148, trace=0x7fff5fbfeff8)
    at namespace.h:859
#5  0x000000010024897f in LoadJavaAPI (kctx=0x100100fa8, ns=0x101022b00, trace=0x7fff5fbfeff8) at java_string.h:399
#6  0x000000010024a8f8 in string_initPackage (kctx=0x100100fa8, ns=0x101022b00, argc=0, args=0x0, trace=0x7fff5fbfeff8)
    at string_glue.c:283
#7  0x000000010002f7bd in loadPackageNULL (kctx=0x100100fa8, packageId=5, trace=0x7fff5fbfeff8) at namespace.h:930
#8  0x000000010002f937 in getPackageNULL (kctx=0x100100fa8, packageId=5, trace=0x7fff5fbfeff8) at namespace.h:951
#9  0x0000000100030072 in kNameSpace_importPackage (kctx=0x100100fa8, ns=0x101020080, name=0x101005868 "konoha.string",
    trace=0x7fff5fbfeff8) at namespace.h:1034
#10 0x0000000100047923 in NameSpace_importPackage (kctx=0x100100fa8, sfp=0x100200080) at sugar.c:255
#11 0x0000000100049999 in KonohaVirtualMachine_run (kctx=0x100100fa8, sfp0=0x100200040, pc=0x1001137c8) at tracevm.h:412
#12 0x00000001000552c6 in MethodFunc_runVirtualMachine (kctx=0x100100fa8, sfp=0x100200040) at compiler.c:1045
#13 0x00000001000184f0 in KonohaRuntime_tryCallMethod (kctx=0x100100fa8, sfp=0x100200040) at klibexec.h:500
#14 0x000000010003a51e in kMethod_runEval (kctx=0x100100fa8, mtd=0x101022700, rtype=0) at tycheck.h:413
#15 0x000000010003a395 in kBlock_genEvalCode (kctx=0x100100fa8, bk=0x101005940, mtd=0x101022700) at tycheck.h:398
#16 0x000000010003a95f in TokenSequence_eval (kctx=0x100100fa8, source=0x7fff5fbff6f0) at tycheck.h:469
#17 0x000000010004674d in kNameSpace_eval (kctx=0x100100fa8, ns=0x101020080, script=0x10081ba00 "import(\"konoha.string\")",
    uline=8589934593) at sugar.c:63
#18 0x00000001000468e7 in MODSUGAR_eval (kctx=0x100100fa8, script=0x10081ba00 "import(\"konoha.string\")", uline=8589934593)
    at sugar.c:76
#19 0x000000010002194e in konoha_eval (konoha=0x100100fa8, script=0x10081ba00 "import(\"konoha.string\")", uline=8589934593)
    at minikonoha.c:262
#20 0x0000000100008a33 in shell (kctx=0x100100fa8) at minishell.h:120
#21 0x0000000100008c44 in konoha_shell (konoha=0x100100fa8) at minishell.h:157
#22 0x000000010000a98e in konoha_parseopt (konoha=0x100100fa8, argc=1, argv=0x7fff5fbffbd0) at command.c:523
#23 0x000000010000aaa5 in main (argc=1, argv=0x7fff5fbffbd0) at command.c:546
(gdb) The program is running.  Exit anyway? (y or n) y
	 */

	// This cause SEGV
	// LoadJavaAPI(kctx, ns, trace);
	KSetClassFunc(ns->packageId, CT_String, unbox, String2_unbox, trace);
	KSetClassFunc(ns->packageId, CT_String, free, String2_free, trace);
	KSetClassFunc(ns->packageId, CT_String, reftrace, String2_reftrace, trace);
	KSetKLibFunc(ns->packageId, new_kString, new_kString, trace);
	return true;
}

static kbool_t string_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* string_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.initPackage    = string_initPackage;
	d.setupPackage   = string_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
