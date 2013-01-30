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


//## NameSpace NameSpace.GetNameSpace(String name);
static KMETHOD NameSpace_GetNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	KPackage *pkg = KRequirePackage(kString_text(sfp[1].asString), trace);
	kNameSpace *ns = pkg == NULL ? (pkg->packageNS) : KNULL(NameSpace);
	KReturn(ns);
}

//## Syntax NameSpace.GetSyntax(Symbol keyword);
static KMETHOD NameSpace_GetSyntax(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kSyntax_(sfp[0].asNameSpace, (ksymbol_t)sfp[1].intValue));
}

//## void NameSpace.DefineSyntax(Syntax syntax);
static KMETHOD NameSpace_DefineSyntax(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	kNameSpace *ns = sfp[0].asNameSpace;
	kSyntaxVar *syn = (kSyntaxVar *)sfp[1].asObject;
	KFieldSet(syn, syn->packageNameSpace, ns);
	SUGAR kNameSpace_AddSyntax(kctx, ns, (kSyntax *)syn, trace);
	KReturnVoid();
}

//## void NameSpace.AddSyntaxPattern(Symbol symbol, String pattern);
static KMETHOD NameSpace_AddSyntaxPattern(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	kNameSpace *ns = sfp[0].asNameSpace;
	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
	const char *pattern = kString_text(sfp[2].asString);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, symbol), pattern, 0, trace);
	KReturnVoid();
}

//## void NameSpace.CompileAllDefinedMethod()
static KMETHOD NameSpace_CompileAllDefinedMethod(KonohaContext *kctx, KonohaStack *sfp)
{
//	kNameSpace *ns = sfp[0].asNameSpace;
//
	KRuntime *share = kctx->share;
	size_t i;
	for(i = 0; i < kArray_size(share->GlobalConstList); i++) {
		kObject *o = share->GlobalConstList->ObjectItems[i];
		if(kObject_class(o) == KClass_NameSpace) {
			kNameSpace *ns = (kNameSpace  *) o;
			size_t j;
			for(j = 0; j < kArray_size(ns->methodList_OnList); j++) {
				kMethod *mtd = ns->methodList_OnList->MethodItems[j];
				if(IS_NameSpace(mtd->LazyCompileNameSpace)) {
					KLIB kMethod_DoLazyCompilation(kctx, mtd, NULL, HatedLazyCompile|CrossCompile);
				}
			}
		}
	}
	KReturnVoid();
}

//## Token[] NameSpace.tokenize(String s);
static KMETHOD NameSpace_Tokenize(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *a = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	KTokenSeq source = {sfp[0].asNameSpace, a};
	SUGAR Tokenize(kctx, source.ns, kString_text(sfp[1].asString), 0, 0, source.tokenList);
	KReturnWith(a, RESET_GCSTACK());
}

//## Token[] NameSpace.Preprocess(String s);
static KMETHOD NameSpace_Preprocess(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *a = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	KTokenSeq source = {sfp[0].asNameSpace, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	SUGAR Tokenize(kctx, source.ns, kString_text(sfp[1].asString), 0, 0, source.tokenList);
	KTokenSeq_End(kctx, source);
	KTokenSeq tokens = {source.ns, a, 0};
	tokens.TargetPolicy.ExpandingBraceGroup = true;
	SUGAR Preprocess(kctx, source.ns, RangeTokenSeq(source), NULL, tokens.tokenList);
	KTokenSeq_Pop(kctx, source);
	KReturnWith(a, RESET_GCSTACK());
}

static void Syntax_defineNameSpaceMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Static, _F(NameSpace_GetNameSpace), KType_NameSpace, KType_NameSpace, KMethodName_("GetPackage"), 1, TP_name,
		_Public|_Const, _F(NameSpace_GetSyntax), KType_Syntax, KType_NameSpace, KMethodName_("GetSyntax"), 1, TP_kw,
		_Public|_Compilation, _F(NameSpace_DefineSyntax), KType_void, KType_NameSpace, KMethodName_("DefineSyntax"), 1, TP_syntax,
		_Public|_Compilation, _F(NameSpace_AddSyntaxPattern), KType_void, KType_NameSpace, KMethodName_("AddSyntaxPattern"), 2, TP_kw, KType_String, KFieldName_("pattern"),
		_Public, _F(NameSpace_CompileAllDefinedMethod), KType_void, KType_NameSpace, KMethodName_("CompileAllDefinedMethod"), 0,
		_Public|_Im, _F(NameSpace_Tokenize), KType_TokenArray, KType_NameSpace, KMethodName_("Tokenize"), 1, TP_source,
		_Public|_Im, _F(NameSpace_Preprocess), KType_TokenArray, KType_NameSpace, KMethodName_("Preprocess"), 1, TP_source,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}
