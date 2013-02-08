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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/import/methoddecl.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */

static void kException_AddStackTrace(KonohaContext *kctx, KonohaStack *sfp, kException *e)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	KBuffer wb;
	KLIB KBuffer_Init(&kctx->stack->cwb, &wb);
	kfileline_t uline = sfp[K_RTNIDX].calledFileLine;
	if(uline > 0) {
		const char *file = KFileLine_textFileName(uline);
		KLIB KBuffer_printf(kctx, &wb, "(%s:%d) %s.%s%s" , PLATAPI shortFilePath(file), (kushort_t)uline, kMethod_Fmt3(mtd));
	}
//	int i = 0, psize = kMethod_ParamSize(mtd);
//	kParam *pa = kMethod_GetParam(mtd);
//	KClass *thisClass = O_cid(sfp[0]);
//	for(i = 0; i < psize; i++) {
//		pa->paramtypeItems[0].attrTypeId;
//		if(i > 0) {
//			knh_putc(ctx, cwb->w, ',');
//		}
//		knh_Write_fn(ctx, cwb->w, p->fn);
//		knh_putc(ctx, cwb->w, '=');
//		knh_Write_sfp(ctx, cwb->w, type, &sfp[i+1], FMT_line);
//	}
	const char *msg = KLIB KBuffer_text(kctx, &wb, EnsureZero);
	KLIB new_kString(kctx, e->StackTraceList, msg, strlen(msg), 0);
//	if((mtd)->mn != MN_LAMBDA) {
//		knh_uline_t uline = knh_stack_uline(ctx, sfp);
//		knh_Write_uline(ctx, cwb->w, uline);
//		knh_Write_type(ctx, cwb->w, (mtd)->cid);
//		knh_putc(ctx, cwb->w, '.');
//		knh_Write_mn(ctx, cwb->w, (mtd)->mn);
//		knh_putc(ctx, cwb->w, '(');
//		knh_putc(ctx, cwb->w, ')');
//		if(DP(e)->tracesNULL == NULL) {
//			KNH_INITv(DP(e)->tracesNULL, new_Array(ctx, CLASS_String, 0));
//		}
//		knh_Array_Add(ctx, DP(e)->tracesNULL, knh_cwb_newString(ctx, cwb));
//	}
}

static kbool_t isCalledMethod(KonohaContext *kctx, KonohaStack *sfp)
{
//	kMethod *mtd = sfp[0].calledMethod;
//	if(knh_isObject(kctx, mtd) && IS_Method(mtd)) {
//		//DBG_P("FOUND calledMethod: shift=%d, pc=%d", sfp[-2].shift, sfp[-1].pc);
//		return true;
//	}
	return false;
}

static void Kthrow(KonohaContext *kctx, KonohaStack *sfp, kException *e)
{
	if(IS_Exception(e)) {
		KonohaStack *p = (sfp == NULL) ? kctx->esp : sfp - 1;
		KonohaStack *bottom = kctx->stack->stack;
		while(bottom < p) {
			if(p[0].calledMethod != NULL && isCalledMethod(kctx, p)) {
				kException_AddStackTrace(kctx, p+1, e);
				p[0].calledMethod = 0;
				//p = p[-1];
			}
			p--;
		}
	}
	KLIB KRuntime_raise(kctx, e->symbol, SoftwareFault, NULL, sfp);
}

/* ------------------------------------------------------------------------ */

//## void System.throw(Object e);
static KMETHOD System_throw(KonohaContext *kctx, KonohaStack *sfp)
{
	KUnsafeFieldSet(kctx->stack->ThrownException, sfp[1].asException);
	Kthrow(kctx, sfp, sfp[1].asException);
}

//## Exception System.getThrownException();
static KMETHOD System_getThrownException(KonohaContext *kctx, KonohaStack *sfp)
{
	//KonohaExceptionContext *ctx = KonohaContext_getExceptionContext(kctx);
	//KReturn(ctx->thrownException);
}

static KMETHOD Exception_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asObject);
}

///* ------------------------------------------------------------------------ */
//static kNode *Node_LookupTryOrCatchNodeNULL(KonohaContext *kctx, kNode *stmt)
//{
//	int i;
//	kArray *bka = stmt->parentNodeNULL->NodeList;
//	ksymbol_t trySym = KSymbol_("try");
//	ksymbol_t catchSym = KSymbol_("catch");
//	for(i = 0; kArray_size(bka); i++) {
//		kNode *s = bka->NodeItems[i];
//		if(s == stmt) {
//			break;
//		}
//	}
//
//	for(i = i-1; i >= 0; i--) {
//		kNode *s = bka->NodeItems[i];
//		if(s->syn && (s->syn->keyword == trySym || s->syn->keyword == catchSym)) {
//			return s;
//		}
//	}
//	return NULL;
//}

static KMETHOD Statement_try(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	DBG_P("try statement .. \n");
	int ret = false;
	//kNode *tryNode, *catchNode, *finallyNode;
	//tryNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_NodePattern, K_NULLBLOCK);
	//ret = SUGAR TypeCheckBlock(kctx, tryNode,   gma);
	//if(ret == false) {
	//	KReturnUnboxValue(ret);
	//}

	//catchNode   = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_("catch"),   K_NULLBLOCK);
	//finallyNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_("finally"), K_NULLBLOCK);
	//ret = SUGAR TypeCheckBlock(kctx, tryNode,   gma);
	//ret = SUGAR TypeCheckBlock(kctx, catchNode, gma);
	//if(ret == false) {
	//	KReturnUnboxValue(ret);
	//}
	//if(finallyNode) {
	//	ret = SUGAR TypeCheckBlock(kctx, finallyNode, gma);
	//}
	//if(ret) {
	//	kNode_Type(kctx, stmt, TRY);
	//}
	KReturnUnboxValue(ret);
}

static KMETHOD Statement_catch(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	DBG_P("catch statement .. \n");
	int ret = false;

	//// check "catch(...)"
	////ret = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_NodePattern, ns, KClass_Exception, 0);

	//kNode *catchNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_NodePattern, K_NULLBLOCK);
	//kNode *parentNode = Node_LookupTryOrCatchNodeNULL(kctx, stmt);

	//if(catchNode != K_NULLBLOCK && parentNode != NULL) {
	//	ret = SUGAR TypeCheckBlock(kctx, catchNode, gma);
	//	kNode *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_NodePattern, K_NULLNODE);
	//	KLIB kObjectProto_SetObject(kctx, parentNode, KSymbol_NodePattern, KType_Exception, expr);
	//	KLIB kObjectProto_SetObject(kctx, parentNode, KSymbol_("catch"), KType_Node, stmt);
	//	kNode_Type(kctx, stmt, KNode_Done, KType_void);
	//} else {
	//	kNode_Message(kctx, stmt, ErrTag, "upper stmt is not try/catch");
	//	KReturnUnboxValue(false);
	//}
	KReturnUnboxValue(ret);
}

static KMETHOD Statement_finally(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	DBG_P("finally statement .. \n");
	int ret = false;
	//kNode *finallyNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_NodePattern, K_NULLBLOCK);

	//if(finallyNode != K_NULLBLOCK) {
	//	kNode *tryNode = Node_LookupTryOrCatchNodeNULL(kctx, stmt);
	//	if(tryNode != NULL) {
	//		ret = SUGAR TypeCheckBlock(kctx, finallyNode, gma);
	//		KLIB kObjectProto_SetObject(kctx, tryNode, KSymbol_("finally"), KType_Node, finallyNode);
	//		kNode_Type(kctx, stmt, KNode_Done, KType_void);
	//	}
	//}

	KReturnUnboxValue(ret);
}


static kbool_t Exception_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Exception_new), KType_Exception,  KType_Exception, KMethodName_("new"), 0, _Public|_Hidden, _F(System_throw), KType_void,  KType_System, KMethodName_("throw"), 1, KType_Exception, KFieldName_("e"),
		_Public|_Hidden, _F(System_getThrownException), KType_Exception, KType_System, KMethodName_("getThrownException"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("try"), SYNFLAG_CTypeFunc, 0, 0, {SUGAR patternParseFunc}, {SUGARFUNC Statement_try}},
		{ KSymbol_("catch"), SYNFLAG_CTypeFunc, 0, 0, {SUGAR patternParseFunc}, {SUGARFUNC Statement_catch}},
		{ KSymbol_("finally"), SYNFLAG_CTypeFunc, 0, 0, {SUGAR patternParseFunc}, {SUGARFUNC Statement_finally}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);

	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("try")), "\"try\" $Node [ \"catch\" \"(\" $Type $Symbol \")\" catch: $Node ] [ \"finally\" finally: $Node ]", 0, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("catch")), "\"catch\" \"(\" $Type $Symbol \")\" $Node", 0, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("finally")), "\"finally\" $Node", 0, trace);

	return true;
}

static kbool_t Exception_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Exception_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "Exception", "0.0");
	d.PackupNameSpace   = Exception_PackupNameSpace;
	d.ExportNameSpace   = Exception_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
