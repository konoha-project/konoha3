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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

static KMETHOD Int_opPlus(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(+(sfp[0].intValue));
}

static KMETHOD Int_opCompl (KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(~sfp[0].intValue);
}

static KMETHOD Int_opLSHIFT (KonohaContext *kctx, KonohaStack *sfp)
{
	int lshift = sfp[1].intValue;
	RETURNi_(sfp[0].intValue << lshift);
}

static KMETHOD Int_opRSHIFT (KonohaContext *kctx, KonohaStack *sfp)
{
	int rshift = sfp[1].intValue;
	RETURNi_(sfp[0].intValue >> rshift);
}

static KMETHOD Int_opAND(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].intValue & sfp[1].intValue);
}

static KMETHOD Int_opOR(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].intValue | sfp[1].intValue);
}

static KMETHOD Int_opXOR(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].intValue ^ sfp[1].intValue);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t int_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Int_opPlus), TY_int, TY_int, MN_("+"), 0,
		_Public|_Const|_Im, _F(Int_opCompl), TY_int, TY_int, MN_("~"), 0,
		_Public|_Const|_Im, _F(Int_opLSHIFT), TY_int, TY_int, MN_("<<"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Int_opRSHIFT), TY_int, TY_int, MN_(">>"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Int_opAND), TY_int, TY_int, MN_("&"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Int_opOR ), TY_int, TY_int, MN_("|"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Int_opXOR), TY_int, TY_int, MN_("^"), 1, TY_int, FN_x,
		//_Public|_Const|_Im, _F(Int_opINC), TY_int, TY_int, MN_("opINC"), 0,
		//_Public|_Const|_Im, _F(Int_opDEC), TY_int, TY_int, MN_("opDEC"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	KDEFINE_INT_CONST IntData[] = {
		{"INT_MAX", TY_int, KINT_MAX},
		{"INT_MIN", TY_int, KINT_MIN},
		{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);
	return true;
}

static kbool_t int_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static char parseHexDigit(char c)
{
	return ('0' <= c && c <= '9') ? c - '0' :
		('a' <= c && c <= 'f') ? c - 'a' + 10:
		('A' <= c && c <= 'F') ? c - 'A' + 10:-1;
}
static char parseOctalDigit(char c)
{
	return ('0' <= c && c <= '7') ? c - '0' : -1;
}
static char parseDecimalDigit(char c)
{
	return ('0' <= c && c <= '9') ? c - '0' : -1;
}

static char parseBinaryDigit(char c)
{
	return ('0' == c || c == '1') ? c - '0' : -1;
}

#include <stdio.h>
static KMETHOD parseNumber(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[1].o;
	const char *source = S_text(sfp[2].asString);
	const char *start = source, *end;
	int c = *source++;
	/*
	 * DIGIT  = 0-9
	 * DIGITS = DIGIT | DIGIT DIGITS
	 * HEX    = 0-9a-fA-F
	 * HEXS   = HEX | HEX HEXS
	 * BIN    = 0 | 1
	 * BINS   = BIN | BIN BINS
	 * TAG    = "0x"  | "0b"
	 * HEXINT = ("0x" | "0X") HEXS
	 * INT    = DIGITS | HEXS | BINS
	 */
	int base = 10;
	bool isFloat = false;
	char (*parseDigit)(char) = parseDecimalDigit;
	if (c == '0') {
		c = *source++;
		switch (c) {
			case 'b':
				base = 2;  parseDigit = parseBinaryDigit; break;
			case 'x':
				base = 16; parseDigit = parseHexDigit; break;
			case '0':case '1':case '2':case '3':
			case '4':case '5':case '6':case '7':
				base = 8; parseDigit = parseOctalDigit;
				break;
			default:
				source--;
				break;
		}
	}
	for (; (c = *source) != 0; ++source) {
		if (c == '_') continue;
		if (parseDigit(c) == -1)
			break;
	}

	/*
	 * DIGIT  = 0-9
	 * DIGITS = DIGIT | DIGIT DIGITS
	 * INT    = DIGIT | DIGIT1-9 DIGITS
	 * FLOAT  = INT
	 *        | INT FRAC
	 *        | INT EXP
	 *        | INT FRAC EXP
	 * FRAC   = "." digits
	 * EXP    = E digits
	 * E      = 'e' | 'e+' | 'e-' | 'E' | 'E+' | 'E-'
	 */
	if (base != 10 && c != '.' && c != 'e' && c != 'E') {
		goto L_emit;
	}
	if (c == '.') {
		isFloat = true;
		source++;
		for (; (c = *source) != 0; ++source) {
			if (c == '_') continue;
			if (parseDecimalDigit(c) == -1)
				break;
		}
	}
	if (c == 'e' || c == 'E') {
		isFloat = true;
		c = *(++source);
		if (!('0' <= c && c <= '9') && !(c == '+' || c == '-')) {
			source--;
			goto L_emit;
		}
		if (c == '+' || c == '-') {
			c = *source++;
		}
		for (; (c = *source) != 0; ++source) {
			if (c == '_') continue;
			if (parseDecimalDigit(c) == -1)
				break;
		}
	}

	L_emit:;
	if (IS_NOTNULL(tk)) {
		/* skip unit */
		for (; (c = *source) != 0; ++source) {
			if (c == '_') continue;
			if (!isalpha(c))
				break;
		}
		end = source;
		KSETv(tk, tk->text, KLIB new_kString(kctx, start, end - start, SPOL_ASCII));
		tk->unresolvedTokenType = isFloat ? SYM_("$Float") : TokenType_INT;
	}
	RETURNi_(source - start);
}

static kint_t _kstrtoll(const char *p, char (*parseDigit)(char), int base)
{
	long long tmp = 0, prev = 0;
	char c;
	for (; (c = *p) != 0; ++p) {
		if (c == '_') continue;
		c = parseDigit(c);
		if (c == -1)
			break;
		tmp = tmp * base + c;
		if (tmp < prev) {
			/* Overflow!! */
			return 0;
		}
		prev = tmp;
	}
	return (kint_t) tmp;
}

static kint_t kstrtoll(const char *p)
{
	if (*p == '0') {
		if (*(p+1) == 'x' || *(p+1) == 'X') {
		return _kstrtoll(p+2, parseHexDigit, 16);
		}
		if (*(p+1) == 'b') {
			return _kstrtoll(p+2, parseBinaryDigit, 2);
		}
		if ('0' <= *(p+1) && *(p+1) <= '7') {
			return _kstrtoll(p+1, parseOctalDigit, 8);
		}
	}
	return _kstrtoll(p, parseDecimalDigit, 10);
}

static KMETHOD ExprTyCheck_Int2(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	long long n = kstrtoll(S_text(tk->text));
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_int, (uintptr_t)n));
}

static kbool_t int_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KW_NumberPattern, 0,  NULL, 0, 0, NULL, NULL, NULL, NULL, ExprTyCheck_Int2, },
		{ SYM_("~"),  0, NULL, 0,                   C_PRECEDENCE_PREUNARY, NULL, NULL, NULL, NULL, NULL, },
		{ SYM_("<<"), 0, NULL, C_PRECEDENCE_SHIFT,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ SYM_(">>"), 0, NULL, C_PRECEDENCE_SHIFT,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ SYM_("&"),  0, NULL, C_PRECEDENCE_BITAND, 0,                     NULL, NULL, NULL, NULL, NULL, },
		{ SYM_("|"),  0, NULL, C_PRECEDENCE_BITOR,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ SYM_("^"),  0, NULL, C_PRECEDENCE_BITXOR, 0,                     NULL, NULL, NULL, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);

	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	kMethod *mtd = KLIB new_kMethod(kctx, 0, 0, 0, parseNumber);
	kFunc *fo = GCSAFE_new(Func, (uintptr_t) mtd);
	SUGAR kNameSpace_setTokenizeFunc(kctx, ns, '0', NULL, fo, 0);

	SugarSyntaxVar *syn = (SugarSyntaxVar*)SUGAR kNameSpace_getSyntax(kctx, ns, SYM_("+"), 0);
	if(syn != NULL) {
		syn->precedence_op1  = 16;
	}
	return true;
}

static kbool_t int_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* int_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "int", "1.0");
	d.initPackage    = int_initPackage;
	d.setupPackage   = int_setupPackage;
	d.initNameSpace  = int_initNameSpace;
	d.setupNameSpace = int_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
