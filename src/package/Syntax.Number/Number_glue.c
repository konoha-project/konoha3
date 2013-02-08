/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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
 * AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* Number Syntax */

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

static KMETHOD TokenFunc_ExtendedIntLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[1].asObject;
	const char *source = kString_text(sfp[2].asString);
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
	if(c == '0') {
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
		if(c == '_') continue;
		if(parseDigit(c) == -1)
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
	if(base != 10 && c != '.' && c != 'e' && c != 'E') {
		goto L_emit;
	}
	if(c == '.') {
		isFloat = true;
		source++;
		for (; (c = *source) != 0; ++source) {
			if(c == '_') continue;
			if(parseDecimalDigit(c) == -1)
				break;
		}
	}
	if(c == 'e' || c == 'E') {
		isFloat = true;
		c = *(++source);
		if(!('0' <= c && c <= '9') && !(c == '+' || c == '-')) {
			source--;
			goto L_emit;
		}
		if(c == '+' || c == '-') {
			c = *source++;
		}
		for (; (c = *source) != 0; ++source) {
			if(c == '_') continue;
			if(parseDecimalDigit(c) == -1)
				break;
		}
	}

	L_emit:;
	if(IS_NOTNULL(tk)) {
		/* skip unit */
		for (; (c = *source) != 0; ++source) {
			if(c == '_') continue;
			if(!isalpha(c))
				break;
		}
		end = source;
		KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, start, end - start, StringPolicy_ASCII));
		tk->tokenType = isFloat ? KSymbol_("$Float") : TokenType_Number;
	}
	KReturnUnboxValue(source - start);
}

static kint_t _kstrtoll(const char *p, char (*parseDigit)(char), int base)
{
	long long tmp = 0, prev = 0;
	char c;
	for (; (c = *p) != 0; ++p) {
		if(c == '_') continue;
		c = parseDigit(c);
		if(c == -1)
			break;
		tmp = tmp * base + c;
		if(tmp < prev) {
			/* Overflow!! */
			return 0;
		}
		prev = tmp;
	}
	return (kint_t) tmp;
}

static kint_t kstrtoll(const char *p)
{
	if(*p == '0') {
		if(*(p+1) == 'x' || *(p+1) == 'X') {
		return _kstrtoll(p+2, parseHexDigit, 16);
		}
		if(*(p+1) == 'b') {
			return _kstrtoll(p+2, parseBinaryDigit, 2);
		}
		if('0' <= *(p+1) && *(p+1) <= '7') {
			return _kstrtoll(p+1, parseOctalDigit, 8);
		}
	}
	return _kstrtoll(p, parseDecimalDigit, 10);
}

static KMETHOD TypeCheck_ExtendedIntLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(expr, gma, reqty);
	kToken *tk = expr->TermToken;
	long long n = kstrtoll(kString_text(tk->text));
	KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_Int, (uintptr_t)n));
}

// --------------------------------------------------------------------------
static kbool_t Number_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_NumberPattern, SYNFLAG_CTokenFunc|SYNFLAG_CTypeFunc, 0, 0, {SUGAR termParseFunc}, {SUGARFUNC TypeCheck_ExtendedIntLiteral}, KonohaChar_Digit, {SUGARFUNC TokenFunc_ExtendedIntLiteral}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);

	return true;
}

static kbool_t Number_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Number_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "Number", "0.0");
	d.PackupNameSpace = Number_PackupNameSpace;
	d.ExportNameSpace = Number_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
