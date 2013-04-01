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

//## int String.asciiAt(int pos);
static KMETHOD String_AsciiAt(KonohaContext *kctx, KonohaStack *sfp)
{
	int ch = 0;
	size_t n = (size_t)sfp[1].intValue, len = kString_size(sfp[0].asString);
	if(n < len) {
		ch = ((unsigned char *)kString_text(sfp[0].asString))[n];
	}
	KReturnUnboxValue(ch);
}

//## @Const @Static int NameSpace.Ascii(String source);
static KMETHOD NameSpace_Ascii(KonohaContext *kctx, KonohaStack *sfp)
{
	int ch = ((unsigned char *)kString_text(sfp[1].asString))[0];
	KReturnUnboxValue(ch);
}

//## int Token.SetText(Symbol keyword, String s, int begin, int end);
static KMETHOD Token_SetParsedText(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_NOTNULL(tk)) {
		kString *text = sfp[2].asString;
		size_t len = kString_size(text);
		size_t beginIdx = (size_t)sfp[3].intValue;
		size_t endIdx   = (size_t)sfp[4].intValue;
		if(beginIdx <= endIdx && endIdx <= len) {
			ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
			tk->tokenType = keyword;
			KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, kString_text(text) + beginIdx, endIdx - beginIdx, 0));
		}
	}
	KReturnUnboxValue(sfp[4].intValue);
}

//## int Token.setError(String s, String s, int begin, int end);
static KMETHOD Token_SetError(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_NOTNULL(tk)) {
		SUGAR kToken_ToError(kctx, tk, ErrTag, "%s", kString_text(sfp[1].asString));
	}
	KReturnUnboxValue(sfp[4].intValue);
}

static void KBuffer_WriteToken(KonohaContext *kctx, KBuffer *wb, kToken *tk)
{
	char c = kToken_GetOpenChar(tk);
	if(IS_String(tk)) {
		if(c != 0) {
			KLIB KBuffer_Write(kctx, wb, &c, 1);
			KLIB KBuffer_Write(kctx, wb, kString_text(tk->text), kString_size(tk->text));
			KLIB KBuffer_Write(kctx, wb, &c, 1);
		}
		else {
			KLIB KBuffer_Write(kctx, wb, kString_text(tk->text), kString_size(tk->text));
		}
	}
	else if(IS_Array(tk)) {
		size_t i;
		kArray *a = tk->GroupTokenList;
		KLIB KBuffer_Write(kctx, wb, &c, 1);
		for(i = 0; i < kArray_size(a); i++) {
			KLIB KBuffer_Write(kctx, wb, " ", 1);
			KBuffer_WriteToken(kctx, wb, a->TokenItems[i]);
		}
		KLIB KBuffer_Write(kctx, wb, " ", 1);
		c = kToken_GetCloseChar(tk);
		KLIB KBuffer_Write(kctx, wb, &c, 1);
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "%s%s", KSymbol_Fmt2(tk->symbol));
	}
}

//## String Token.toString();
static KMETHOD Token_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_String(tk->text)) {
		KReturn(tk->text);
	}
	else {
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KBuffer_WriteToken(kctx, &wb, tk);
		KReturn(KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer));
	}
}

//## Token Token.new(Symbol key);
static KMETHOD Token_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	tk->symbol = key;
	KReturn(tk);
}

//## boolean Token.Is(Symbol s);
static KMETHOD Token_Is(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	KReturnUnboxValue(tk->resolvedSyntaxInfo != NULL && tk->resolvedSyntaxInfo->keyword == keyword);
}

//## String Token.getText();
static KMETHOD Token_GetParsedText(KonohaContext *kctx, KonohaStack *sfp)
{
	kToken *tk = sfp[0].asToken;
	if(IS_String(tk->text)) {
		KReturnField(tk->text);
	}
}

//## Object Token.getType();
static KMETHOD Token_GetType(KonohaContext *kctx, KonohaStack *sfp)
{
	kToken *tk = sfp[0].asToken;
	if(tk->resolvedSyntaxInfo->keyword) {
		KReturnField(KLIB Knull(kctx, KClass_(tk->resolvedTypeId)));
	}
}


//## void Token.getGroupTokenList();
static KMETHOD Token_GetGroupTokenList(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_Array(tk->GroupTokenList)) {
		KReturnField(tk->GroupTokenList);
	}
}

//## boolean Token.isBeforeWhiteSpace();
static KMETHOD Token_isBeforeWhiteSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kToken_Is(BeforeWhiteSpace, sfp[0].asToken));
}

static void Syntax_defineTokenFunc(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Const|_Static, _F(NameSpace_Ascii), KType_String, KType_NameSpace, KMethodName_("Ascii"), 1, TP_source,
		_Public, _F(String_AsciiAt), KType_Int, KType_String, KMethodName_("AsciiAt"), 1, TP_pos,
		_Public, _F(Token_SetParsedText), KType_Int, KType_Token, KMethodName_("SetParsedText"), 4, TP_kw, TP_source, TP_begin, TP_end,
		_Public, _F(Token_SetError), KType_Int, KType_Token, KMethodName_("SetError"), 4, TP_message, TP_source, TP_begin, TP_end,
		_Public|_Coercion|_Im|_Const, _F(Token_toString), KType_String, KType_Token, KMethodName_To(KType_String), 0,
		_Public|_Const, _F(Token_Is), KType_Boolean, KType_Token, KMethodName_("Is"), 1, TP_kw,
		_Public|_Const, _F(Token_GetParsedText), KType_String, KType_Token, KMethodName_("GetParsedText"), 0,
		_Public|_Const, _F(Token_GetType), KType_Object, KType_Token, KMethodName_("GetType"), 0,
		_Public|_Const, _F(Token_GetGroupTokenList), KType_TokenArray, KType_Token, KMethodName_("GetGroupTokenList"), 0,
		_Public, _F(Token_isBeforeWhiteSpace), KType_Boolean, KType_Token, KMethodName_("IsBeforeWhiteSpace"), 0,
		_Public, _F(Token_new), KType_Token, KType_Token, KMethodName_("new"), 1, TP_kw,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

