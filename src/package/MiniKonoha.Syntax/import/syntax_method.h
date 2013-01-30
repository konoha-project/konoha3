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

//## Syntax Syntax.new(Symbol keyword);
static KMETHOD Syntax_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = new_(SyntaxVar, 0, OnStack);
	syn->keyword = (ksymbol_t)sfp[1].intValue;
	KReturn(syn);
}

//## Syntax Syntax.new(Syntax parent);
static KMETHOD Syntax_newParent(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntax *parentSyntax = sfp[1].asSyntax;
	kSyntaxVar *syn = new_(SyntaxVar, 0, OnStack);
	syn->keyword = parentSyntax->keyword;
	KReturn(syn);
}

//## void Syntax.SetTokenFunc(int konohaChar, Func[Int, Token, String] func);
static KMETHOD Syntax_SetTokenFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	syn->tokenKonohaChar = AsciiToKonohaChar(sfp[1].intValue);
	KFieldSet(syn, syn->TokenFuncNULL, sfp[2].asFunc);
	KReturnVoid();
}

//## void Syntax.SetParseFunc(int op1, int op2, Func[Int, Node, Symbol, Token[], Int, Int, Int] func);
static KMETHOD Syntax_SetParseFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	syn->precedence_op1 = sfp[1].intValue;
	syn->precedence_op2 = sfp[2].intValue;
	KFieldSet(syn, syn->ParseFuncNULL, sfp[3].asFunc);
	KReturnVoid();
}

//## void Syntax.SetTypeFunc(Func[Node, NameSpace, Object] func);
static KMETHOD Syntax_SetTypeFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	KFieldSet(syn, syn->TypeFuncNULL, sfp[1].asFunc);
	KReturnVoid();
}

//## void Syntax.SetMacro(int number, Symbol macro);
static KMETHOD Syntax_SetMacro(KonohaContext *kctx, KonohaStack *sfp)
{
	KTODO();
	//kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	//int number = sfp[1].intValue;
	//ksymbol_t macro = (ksymbol_t)sfp[2].intValue;
	//KReturnVoid();
}

//## void Syntax.SetMetaPattern(boolean flag);
static KMETHOD Syntax_SetMetaPattern(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	kbool_t flag = sfp[1].boolValue;
	kSyntax_Set(MetaPattern, syn, flag);
	KReturnVoid();
}

//## boolean Syntax.IsMetaPattern();
static KMETHOD Syntax_IsMetaPattern(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	KReturnUnboxValue(kSyntax_Is(MetaPattern, syn));
}

//## void Syntax.SetPattern(String pattern);
static KMETHOD Syntax_SetPattern(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	const char *pattern = kString_text(sfp[1].asString);
	SUGAR kSyntax_AddPattern(kctx, syn, pattern, 0, trace);
	KReturnVoid();
}

//## Func NameSpace.GetTermParseFunc();
static KMETHOD NameSpace_GetTermParseFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(SUGAR termParseFunc);
}

//## Func NameSpace.GetOpParseFunc();
static KMETHOD NameSpace_GetOpParseFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(SUGAR opParseFunc);
}

//## Func NameSpace.GetPatternParseFunc();
static KMETHOD NameSpace_GetPatternParseFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(SUGAR patternParseFunc);
}

//## Func NameSpace.GetMethodTypeFunc();
static KMETHOD NameSpace_GetMethodTypeFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(SUGAR methodTypeFunc);
}

static void Syntax_defineSyntaxMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	/* Func[Int, Token, String] */
	kparamtype_t P_FuncToken[] = {{KType_Token}, {KType_String}};
	int KType_FuncToken = (KLIB KClass_Generics(kctx, KClass_Func, KType_Int, 2, P_FuncToken))->typeId;
	/* Func[Int, Node, Symbol, Token[], Int, Int, Int] */
	kparamtype_t P_FuncParse[] = {{KType_Node}, {KType_Symbol}, {KType_TokenArray}, {KType_Int}, {KType_Int}, {KType_Int}};
	int KType_FuncParse = (KLIB KClass_Generics(kctx, KClass_Func, KType_Int, 6, P_FuncParse))->typeId;
	/* Func[Node, NameSpace, Object] */
	kparamtype_t P_FuncType[] = {{KType_Node}, {KType_NameSpace}, {KType_Object}};
	int KType_FuncType = (KLIB KClass_Generics(kctx, KClass_Func, KType_Node, 3, P_FuncType))->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(Syntax_new), KType_Syntax, KType_Syntax, KMethodName_("new"), 1, TP_kw,
		_Public|_Im, _F(Syntax_newParent), KType_Syntax, KType_Syntax, KMethodName_("new"), 1, KType_Syntax, KMethodName_("parent"),
		_Public,     _F(Syntax_SetTokenFunc), KType_void, KType_Syntax, KMethodName_("SetTokenFunc"), 2, KType_Int, KFieldName_("kchar"), KType_FuncToken, KFieldName_("func"),
		_Public,     _F(Syntax_SetParseFunc), KType_void, KType_Syntax, KMethodName_("SetParseFunc"), 3, KType_Int, KFieldName_("op1"), KType_Int, KFieldName_("op2"), KType_FuncParse, KFieldName_("func"),
		_Public,     _F(Syntax_SetTypeFunc), KType_void, KType_Syntax, KMethodName_("SetTypeFunc"), 1, KType_FuncType, KFieldName_("func"),
		_Public,     _F(Syntax_SetMacro), KType_void, KType_Syntax, KMethodName_("SetMacro"), 2, KType_Int, KFieldName_("number"), KType_Symbol, KFieldName_("macro"),
		_Public,     _F(Syntax_SetMetaPattern), KType_void, KType_Syntax, KMethodName_("SetMetaPattern"), 1, KType_Boolean, KFieldName_("flag"),
		_Public|_Im, _F(Syntax_IsMetaPattern), KType_Boolean, KType_Syntax, KMethodName_("IsMetaPattern"), 0,
		_Public,     _F(Syntax_SetPattern), KType_void, KType_Syntax, KMethodName_("SetPattern"), 1, KType_String, KFieldName_("pattern"),
		_Public|_Const, _F(NameSpace_GetTermParseFunc), KType_FuncParse, KType_NameSpace, KMethodName_("GetTermParseFunc"), 0,
		_Public|_Const, _F(NameSpace_GetOpParseFunc), KType_FuncParse, KType_NameSpace, KMethodName_("GetOpParseFunc"), 0,
		_Public|_Const, _F(NameSpace_GetPatternParseFunc), KType_FuncParse, KType_NameSpace, KMethodName_("GetPatternParseFunc"), 0,
		_Public|_Const, _F(NameSpace_GetMethodTypeFunc), KType_FuncType, KType_NameSpace, KMethodName_("GetMethodTypeFunc"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}
