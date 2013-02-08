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
/* ClosureModel */
static KMETHOD KMethodFunc_ObjectFunctionGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	KReturn((sfp[0].asFunc)->env->fieldObjectItems[delta]);
}
static KMETHOD KMethodFunc_UnboxFunctionGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	KReturnUnboxValue((sfp[0].asFunc)->env->fieldUnboxItems[delta]);
}
static KMETHOD KMethodFunc_ObjectFunctionSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	kObjectVar *o = (kObjectVar *) sfp[0].asFunc->env;
	KFieldSet(o, o->fieldObjectItems[delta], sfp[1].asObject);
	KReturn(sfp[1].asObject);
}

static KMETHOD KMethodFunc_UnboxFunctionSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	kObjectVar *o = (kObjectVar *) sfp[0].asFunc->env;
	o->fieldUnboxItems[delta] = sfp[1].unboxValue;
	KReturnUnboxValue(sfp[1].unboxValue);
}

static kMethod *new_FunctionGetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, ksymbol_t sym, ktypeattr_t ty, int idx)
{
	kmethodn_t mn = KMethodName_ToGetter(sym);
	KMethodFunc f = KType_Is(UnboxType, ty) ? KMethodFunc_UnboxFunctionGetter : KMethodFunc_ObjectFunctionGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar *)mtd)->delta = idx;
	return mtd;
}

static kMethod *new_FunctionSetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, kmethodn_t sym, ktypeattr_t ty, int idx)
{
	kmethodn_t mn = KMethodName_ToSetter(sym);
	KMethodFunc f = (KType_Is(UnboxType, ty)) ? KMethodFunc_UnboxFunctionSetter : KMethodFunc_ObjectFunctionSetter;
	kparamtype_t p = {ty, KFieldName_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 1, &p);
	((kMethodVar *)mtd)->delta = idx;
	return mtd;
}

/* ------------------------------------------------------------------------ */
// @Public @Hidden Func[T] Func[T].Create(Object env, Method mtd)
static KMETHOD Func_Create(KonohaContext *kctx, KonohaStack *sfp)
{
	kFuncVar *fo = (kFuncVar *) sfp[0].asFunc;
	kObject *env = sfp[1].asObject;
	kMethod *mtd = sfp[2].asMethod;
	KFieldSet(fo, fo->env, env);
	KFieldSet(fo, fo->method, mtd);
	KReturn(fo);
}

/* ------------------------------------------------------------------------ */

static kbool_t SetParamType(KonohaContext *kctx, kNode *stmt, int n, kparamtype_t *p)
{
	kToken *typeToken  = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
	kNode  *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
	DBG_ASSERT(typeToken != NULL);
	DBG_ASSERT(expr != NULL);
	if(kNode_isSymbolTerm(expr)) {
		kToken *tkN = expr->TermToken;
		p[n].name = tkN->symbol;
		p[n].attrTypeId = Token_typeLiteral(typeToken);
		return true;
	}
	return false;
}

/* copied from src/parser/import/syntax.h */
static kParam *kNode_GetParamNULL(KonohaContext *kctx, kNode *stmt, kToken *typeTk, kNameSpace* ns)
{
	kNode *params = (kNode *) kNode_GetObjectNULL(kctx, stmt, KSymbol_ParamPattern);
	/* parsing parameter of closure function */
	size_t i, psize = kNode_GetNodeListSize(kctx, params);
	kparamtype_t *p = ALLOCA(kparamtype_t, psize);
	for(i = 0; i < psize; i++) {
		kNode *node = params->NodeList->NodeItems[i];
		if(node->syn->keyword != KSymbol_TypeDeclPattern || !SetParamType(kctx, node, i, p)) {
			/*"invalid parameter"*/
			break;
		}
	}
	return new_kParam(kctx, typeTk->resolvedTypeId, psize, p);
}

#define KPushGammaStack(G,B) struct KGammaLocalData *oldbuf_ = kNameSpace_PushGamma(kctx, G, B)
#define KPopGammaStack(G,B)  kNameSpace_PopGamma(kctx, G, oldbuf_, B)

static struct KGammaLocalData *kNameSpace_PushGamma(KonohaContext *kctx, kNameSpace *ns, struct KGammaLocalData *newone)
{
	struct KGammaLocalData *oldone = ns->genv;
	ns->genv = newone;
	return oldone;
}

static struct KGammaLocalData *kNameSpace_PopGamma(KonohaContext *kctx, kNameSpace *ns, struct KGammaLocalData *oldone, struct KGammaLocalData *checksum)
{
	struct KGammaLocalData *newone = ns->genv;
	assert(checksum == newone);
	ns->genv = oldone;
	if(newone->localScope.allocsize > 0) {
		KFree(newone->localScope.varItems, newone->localScope.allocsize);
	}
	return newone;
}

static void Object_InitToMakeDefaultValueAsNull(KonohaContext *kctx, kObject *o, void *conf)
{
	kObjectVar *of = (kObjectVar *)o;
	KClass *c = kObject_class(o);
	bzero(of->fieldObjectItems, c->cstruct_size - sizeof(kObjectHeader));
}

static KClassVar* kNameSpace_DefineClassName(KonohaContext *kctx, kNameSpace *ns, const char *text, size_t len)
{
	KDEFINE_CLASS defNewClass = {0};
	defNewClass.cflag       = 0;
	defNewClass.typeId      = KTypeAttr_NewId;
	defNewClass.baseTypeId  = KType_Object;
	defNewClass.superTypeId = KType_Object;
	defNewClass.init = Object_InitToMakeDefaultValueAsNull;

	kString *name = KLIB new_kString(kctx, GcUnsafe, text, len, StringPolicy_TEXT);
	KBaseTrace(trace);
	return (KClassVar *) KLIB kNameSpace_DefineClass(kctx, ns, name, &defNewClass, trace);
}

static KClass *CreateEnvClass(KonohaContext *kctx, kNameSpace *ns, kToken *typeTk, KClass **EnvObjectClass)
{
	INIT_GCSTACK();
	size_t i = 0, esize = ns->genv->localScope.varsize;
	size_t start = 0, end = esize;
	KGammaStackDecl *oldenv = ns->genv->localScope.varItems;
	kparamtype_t *p = ALLOCA(kparamtype_t, esize);
	char buf[256] = {'_', '_', '_', 'E', 'N', 'V', 0}, *text = buf + 6;
	if(ns->genv->thisClass == KClass_NameSpace) {
		start = 1;
		end = esize - 1;
	}
	assert(end < 256);
	for(i = start; i <= end; i++) {
		p[i-1].name       = oldenv[i].name;
		p[i-1].attrTypeId = oldenv[i].attrTypeId;
		*(text++) = (KType_text(p[i-1].attrTypeId))[0];
	}

	*EnvObjectClass = KLIB kNameSpace_GetClassByFullName(kctx, ns, buf, text - buf, NULL);
	if(*EnvObjectClass == NULL) {
		*EnvObjectClass = kNameSpace_DefineClassName(kctx, ns, buf, text - buf);
	}

	KClass *ct = KLIB KClass_Generics(kctx, KClass_Func, typeTk->resolvedTypeId, end, p);
	if(end >= 1) {
		((KClassVar *)ct)->classMethodList = new_(MethodArray, end*2, OnGlobalConstList);
		for(i = start; i <= end; i++) {
			int n = i - 1;
			ksymbol_t sym    = p[n].name;
			ktypeattr_t type = KTypeAttr_Unmask(p[n].attrTypeId);
			kMethod *getter = new_FunctionGetter(kctx, _GcStack, ct->typeId, sym, type, n);
			kMethod *setter = new_FunctionSetter(kctx, _GcStack, ct->typeId, sym, type, n);
			KLIB kArray_Add(kctx, ct->classMethodList, getter);
			KLIB kArray_Add(kctx, ct->classMethodList, setter);
		}
	}
	RESET_GCSTACK();
	return ct;
}

static void kNameSpace_InitParam(KonohaContext *kctx, kNameSpace *ns, struct KGammaLocalData *env, kParam *pa, KClass *envCt)
{
	size_t i;
	env->localScope.varItems[0].attrTypeId = envCt->typeId;
	env->localScope.varItems[0].name = KFieldName_("env");
	for(i = 0; i < pa->psize; i++) {
		const kparamtype_t *pn = (pa->paramtypeItems)+i;
		env->localScope.varItems[i+1].name       = pn->name;
		env->localScope.varItems[i+1].attrTypeId = pn->attrTypeId;
	}
	env->localScope.varsize += pa->psize + 1;
}

static kMethod *CompileClosure(KonohaContext *kctx, kNameSpace *ns, kNode *expr, KClass *envCt, kToken *typeTk, kNode **texprRef)
{
	INIT_GCSTACK();
	kParam *pa = kNode_GetParamNULL(kctx, expr, typeTk, ns);
	kMethodVar *mtd = (kMethodVar *) KLIB new_kMethod(kctx, _GcStack, 0, envCt->typeId, 0/*mn*/, NULL);
	KLIB kMethod_SetParam(kctx, mtd, pa->rtype, pa->psize, (kparamtype_t *)pa->paramtypeItems);

	int errorCount = KGetParserContext(kctx)->errorMessageCount;
	KGammaStackDecl lvarItems[32] = {};
	struct KGammaLocalData newgma = {};
	newgma.flag = 0;
	newgma.currentWorkingMethod = mtd;
	newgma.thisClass            = envCt;
	newgma.localScope.varItems  = lvarItems;
	newgma.localScope.capacity  = 32;
	newgma.localScope.varsize   = 0;
	newgma.localScope.allocsize = 0;
	kNameSpace_InitParam(kctx, ns, &newgma, pa, envCt);

	KPushGammaStack(ns, &newgma);
	*texprRef = SUGAR TypeCheckNodeByName(kctx, expr, KSymbol_BlockPattern, ns, KClass_var, TypeCheckPolicy_AllowVoid);

	kNode *block = SUGAR kNode_GetNode(kctx, expr, KSymbol_BlockPattern, NULL);
	KLIB kMethod_GenCode(kctx, mtd, block, HatedLazyCompile);

	KPopGammaStack(ns, &newgma);
	kMethod_Set(StaticError, mtd, KGetParserContext(kctx)->errorMessageCount > errorCount);
	RESET_GCSTACK();
	return mtd;
}

/* ------------------------------------------------------------------------ */
static KMETHOD TypeCheck_Closure(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(expr, ns, reqc);
	kNode *texpr = K_NULLNODE;
	INIT_GCSTACK();
	kToken *typeTk   = SUGAR kNode_GetToken(kctx, expr, KSymbol_TypePattern, NULL);
	KClass *EnvObjectClass = NULL;
	KClass *envCt = CreateEnvClass(kctx, ns, typeTk, &EnvObjectClass);

	kMethod *mtd = CompileClosure(kctx, ns, expr, envCt, typeTk, &texpr);
	/* type check is OK */
	if(texpr != K_NULLNODE) {
		/*
		 * FunctionExpression
		 * 0: Method
		 * 1: EnvObject's Default Object
		 * 2: Current LocalScope Variable
		 * 3: ditto
		 * 4: ...
		 */
		kNode_Type(kctx, texpr, KNode_Function, envCt->typeId);
		KFieldSet(expr, texpr->NodeList, new_(Array, 0, OnField));
		KLIB kArray_Add(kctx, texpr->NodeList, mtd);
		KLIB kArray_Add(kctx, texpr->NodeList, KLIB Knull(kctx, EnvObjectClass));
		size_t i = 0;
		struct KGammaLocalData *genv = ns->genv;
		if(genv->thisClass == KClass_NameSpace) {
			i = 1;
		}
		for(; i < genv->localScope.varsize; i++) {
			kNode *node = new_VariableNode(kctx, ns, KNode_Local, genv->localScope.varItems[i].attrTypeId, i);
			KLIB kArray_Add(kctx, texpr->NodeList, node);
		}
	}
	RESET_GCSTACK();
	KReturn(texpr);
}



// --------------------------------------------------------------------------
static kbool_t ClosureModel_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("function"), SYNFLAG_CTypeFunc, 0, Precedence_CStyleAssign, {SUGAR patternParseFunc}, {SUGARFUNC TypeCheck_Closure}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("function")), "\"function\" $Param [$Type] $Block", 0, NULL);

	KDEFINE_METHOD MethodData[] = {
		_Public|_Hidden, _F(Func_Create), KType_Func, KType_Func, KMethodName_("_Create"), 2, KType_Object, KFieldName_("env"), KType_Method, KFieldName_("mtd"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t ClosureModel_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *ClosureModel_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "ClosureModel", "0.0");
	d.PackupNameSpace = ClosureModel_PackupNameSpace;
	d.ExportNameSpace = ClosureModel_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
