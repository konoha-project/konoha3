///****************************************************************************
// * Copyright (c) 2012, the Konoha project authors. All rights reserved.
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions are met:
// *
// *  * Redistributions of source code must retain the above copyright notice,
// *    this list of conditions and the following disclaimer.
// *  * Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the distribution.
// *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ***************************************************************************/
//
//#include <minikonoha/minikonoha.h>
//#include <minikonoha/sugar.h>
//#include <minikonoha/konoha_common.h>
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//typedef kushort_t kfault_t;
//typedef const struct kExceptionVar kException;
//typedef struct kExceptionVar kExceptionVar;
//
//struct kExceptionVar {
//	kObjectHeader h;
//	kshortflag_t flag;   kfault_t faultId;
//	kfileline_t  uline;
//	kString     *message;
//	kArray      *StackTraceList;
//};
//
//// Module
//
//#define KClassFlag_Exception                            KClassFlag_Final
//
//#define KonohaContext_getExceptionModule(kctx)           ((KonohaExceptionModule *)kctx->modshare[MOD_exception])
//#define KonohaContext_getExceptionContext(kctx)          ((KonohaExceptionContext *)kctx->modlocal[MOD_exception])
//#define KClass_Exception         KonohaContext_getExceptionModule(kctx)->cException
//#define KType_Exception         KonohaContext_getExceptionModule(kctx)->cException->typeId
//#define IS_Exception(e)      (kObject_class(e) == KClass_Exception)
//
//typedef struct {
//	KRuntimeModule  h;
//	KClass *cException;
//	//
//} KonohaExceptionModule;
//
//typedef struct {
//	KContextModule h;
//	kException        *thrownException;
//	//
//} KonohaExceptionContext;
//
///* ------------------------------------------------------------------------ */
//
//static void kException_AddStackTrace(KonohaContext *kctx, KonohaStack *sfp, kException *e)
//{
//	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
//	KBuffer wb;
//	KLIB KBuffer_Init(&kctx->stack->cwb, &wb);
//	kfileline_t uline = sfp[K_RTNIDX].calledFileLine;
//	if(uline > 0) {
//		const char *file = KFileLine_textFileName(uline);
//		KLIB KBuffer_printf(kctx, &wb, "(%s:%d) %s.%s%s" , PLATAPI shortFilePath(file), (kushort_t)uline, kMethod_Fmt3(mtd));
//	}
////	int i = 0, psize = kMethod_ParamSize(mtd);
////	kParam *pa = kMethod_GetParam(mtd);
////	KClass *thisClass = O_cid(sfp[0]);
////	for(i = 0; i < psize; i++) {
////		pa->paramtypeItems[0].attrTypeId;
////		if(i > 0) {
////			knh_putc(ctx, cwb->w, ',');
////		}
////		knh_Write_fn(ctx, cwb->w, p->fn);
////		knh_putc(ctx, cwb->w, '=');
////		knh_Write_sfp(ctx, cwb->w, type, &sfp[i+1], FMT_line);
////	}
//	const char *msg = KLIB KBuffer_text(kctx, &wb, EnsureZero);
//	KLIB new_kString(kctx, e->StackTraceList, msg, strlen(msg), 0);
////	if((mtd)->mn != MN_LAMBDA) {
////		knh_uline_t uline = knh_stack_uline(ctx, sfp);
////		knh_Write_uline(ctx, cwb->w, uline);
////		knh_Write_type(ctx, cwb->w, (mtd)->cid);
////		knh_putc(ctx, cwb->w, '.');
////		knh_Write_mn(ctx, cwb->w, (mtd)->mn);
////		knh_putc(ctx, cwb->w, '(');
////		knh_putc(ctx, cwb->w, ')');
////		if(DP(e)->tracesNULL == NULL) {
////			KNH_INITv(DP(e)->tracesNULL, new_Array(ctx, CLASS_String, 0));
////		}
////		knh_Array_Add(ctx, DP(e)->tracesNULL, knh_cwb_newString(ctx, cwb));
////	}
//}
//
//static kbool_t isCalledMethod(KonohaContext *kctx, KonohaStack *sfp)
//{
////	kMethod *mtd = sfp[0].calledMethod;
////	if(knh_isObject(kctx, mtd) && IS_Method(mtd)) {
////		//DBG_P("FOUND calledMethod: shift=%d, pc=%d", sfp[-2].shift, sfp[-1].pc);
////		return true;
////	}
//	return false;
//}
//
//static void Kthrow(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KonohaExceptionContext *ctx = KonohaContext_getExceptionContext(kctx);
//	kException *e = ctx->thrownException;
//	if(IS_Exception(e)) {
//		KonohaStack *p = (sfp == NULL) ? kctx->esp : sfp - 1;
//		KonohaStack *bottom = kctx->stack->jump_bottom;
//		while(bottom < p) {
//			if(p[0].calledMethod != NULL && isCalledMethod(kctx, p)) {
//				kException_AddStackTrace(kctx, p+1, e);
//				p[0].calledMethod = 0;
//				//p = p[-1];
//			}
//			p--;
//		}
//	}
//	//KLIB KRuntime_raise(kctx, 1);
//}
//
///* ------------------------------------------------------------------------ */
//
////## void System.throw(Exception e);
//static KMETHOD System_throw(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KonohaExceptionContext *ctx = KonohaContext_getExceptionContext(kctx);
//	KUnsafeFieldSet(ctx->thrownException, sfp[1].asException);
//	Kthrow(kctx, sfp);
//}
//
////## Exception System.getThrownException();
//static KMETHOD System_getThrownException(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KonohaExceptionContext *ctx = KonohaContext_getExceptionContext(kctx);
//	KReturn(ctx->thrownException);
//}
//
//static KMETHOD Exception_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturn(sfp[0].asObject);
//}
//
//// --------------------------------------------------------------------------
//
//#define _Public   kMethod_Public
//#define _Const    kMethod_Const
//#define _Coercion kMethod_Coercion
//#define _Hidden   kMethod_Hidden
//#define _F(F)     (intptr_t)(F)
//
//// --------------------------------------------------------------------------
//
//static void Exception_Init(KonohaContext *kctx, kObject *o, void *conf)
//{
//	kExceptionVar *e = (kExceptionVar *)o;
//	e->flag = 0;
//	e->faultId = 0;
//	e->uline = 0;
//	KFieldInit(e, e->message, TS_EMPTY);
//	KFieldInit(e, e->StackTraceList, K_EMPTYARRAY);
//}
//
//static void Exception_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
//{
//	BEGIN_REFTRACE(2);
//	kExceptionVar *e = (kExceptionVar *)o;
//	KRefTrace(e->message);
//	KRefTrace(e->StackTraceList);
//	END_REFTRACE();
//
//}
//
//static void Exception_p(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
//{
//	KLIB KBuffer_printf(kctx, wb, "%s", kString_text(v[pos].asException->message));
//}
//
//static void kModuleException_Setup(KonohaContext *kctx, KRuntimeModule *def, int newctx)
//{
//
//}
//
//static void kModuleException_Reftrace(KonohaContext *kctx, KRuntimeModule *baseh, KObjectVisitor *visitor)
//{
//}
//
//static void kModuleException_Free(KonohaContext *kctx, KRuntimeModule *baseh)
//{
//	KFree(baseh, sizeof(KonohaExceptionModule));
//}
//
//static kbool_t exception_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int argc, const char **args, KTraceInfo *trace)
//{
//	KonohaExceptionModule *mod = (KonohaExceptionModule *)KCalloc_UNTRACE(sizeof(KonohaExceptionModule), 1);
//	mod->h.name     = "exception";
//	mod->h.setup    = kModuleException_Setup;
//	mod->h.reftrace = kModuleException_Reftrace;
//	mod->h.free     = kModuleException_Free;
//	KLIB KRuntime_SetModule(kctx, MOD_exception, &mod->h, trace);
//	KDEFINE_CLASS defException = {
//		STRUCTNAME(Exception),
//		.cflag = KClassFlag_Exception,
//		.init  = Exception_Init,
//		.reftrace = Exception_Reftrace,
//		.p     = Exception_p,
//	};
//	mod->cException = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defException, trace);
//
//	KDEFINE_METHOD MethodData[] = {
//		_Public, _F(Exception_new), KType_Exception,  KType_Exception, KKMethodName_("new"), 0, _Public|_Hidden, _F(System_throw), KType_void,  KType_System, KKMethodName_("throw"), 1, KType_Exception, KFieldName_("e"),
//		_Public|_Hidden, _F(System_getThrownException), KType_Exception, KType_System, KKMethodName_("getThrownException"), 0,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//	return true;
//}
//
//static kbool_t exception_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
//{
//	return true;
//}
//
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
//
//static KMETHOD Statement_try(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	DBG_P("try statement .. \n");
//	int ret = false;
//	kNode *tryNode, *catchNode, *finallyNode;
//	tryNode     = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_NodePattern, K_NULLBLOCK);
//	ret = SUGAR TypeCheckBlock(kctx, tryNode,   gma);
//	if(ret == false) {
//		KReturnUnboxValue(ret);
//	}
//
//	catchNode   = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_("catch"),   K_NULLBLOCK);
//	finallyNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_("finally"), K_NULLBLOCK);
//	ret = SUGAR TypeCheckBlock(kctx, tryNode,   gma);
//	ret = SUGAR TypeCheckBlock(kctx, catchNode, gma);
//	if(ret == false) {
//		KReturnUnboxValue(ret);
//	}
//	if(finallyNode) {
//		ret = SUGAR TypeCheckBlock(kctx, finallyNode, gma);
//	}
//	if(ret) {
//		kNode_Type(kctx, stmt, TRY);
//	}
//	KReturnUnboxValue(ret);
//}
//
//static KMETHOD Statement_catch(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	DBG_P("catch statement .. \n");
//	int ret = false;
//
//	// check "catch(...)"
//	//ret = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_NodePattern, gma, KClass_Exception, 0);
//
//	kNode *catchNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_NodePattern, K_NULLBLOCK);
//	kNode *parentNode = Node_LookupTryOrCatchNodeNULL(kctx, stmt);
//
//	if(catchNode != K_NULLBLOCK && parentNode != NULL) {
//		ret = SUGAR TypeCheckBlock(kctx, catchNode, gma);
//		kNode *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_NodePattern, K_NULLNODE);
//		KLIB kObjectProto_SetObject(kctx, parentNode, KSymbol_NodePattern, KType_Exception, expr);
//		KLIB kObjectProto_SetObject(kctx, parentNode, KSymbol_("catch"), KType_Node, stmt);
//		kNode_Type(kctx, stmt, KNode_Done, KType_void);
//	} else {
//		kNode_Message(kctx, stmt, ErrTag, "upper stmt is not try/catch");
//		KReturnUnboxValue(false);
//	}
//	KReturnUnboxValue(ret);
//}
//
//static KMETHOD Statement_finally(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	DBG_P("finally statement .. \n");
//	int ret = false;
//	kNode *finallyNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_NodePattern, K_NULLBLOCK);
//
//	if(finallyNode != K_NULLBLOCK) {
//		kNode *tryNode = Node_LookupTryOrCatchNodeNULL(kctx, stmt);
//		if(tryNode != NULL) {
//			ret = SUGAR TypeCheckBlock(kctx, finallyNode, gma);
//			KLIB kObjectProto_SetObject(kctx, tryNode, KSymbol_("finally"), KType_Node, finallyNode);
//			kNode_Type(kctx, stmt, KNode_Done, KType_void);
//		}
//	}
//
//	KReturnUnboxValue(ret);
//}
//
//static kbool_t exception_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ .keyword = KSymbol_("try"), Statement_(try), .rule = "\"try\" $Node [ \"catch\" \"(\" $Type $Symbol \")\" catch: $Node ] [ \"finally\" finally: $Node ]",},
//		{ .keyword = KSymbol_("catch"), Statement_(catch), .rule = "\"catch\" \"(\" $Type $Symbol \")\" $Node",},
//		{ .keyword = KSymbol_("finally"), Statement_(finally), .rule = "\"finally\" $Node ",},
//		{ .keyword = KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	return true;
//}
//
//static kbool_t exception_SetupNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
//{
//	return true;
//}
//
//KDEFINE_PACKAGE *exception_Init(void)
//{
//	static KDEFINE_PACKAGE d = {
//		KPACKNAME("konoha", "1.0"),
//		.PackupNameSpace    = exception_PackupNameSpace,
//		.ExportNameSpace   = exception_ExportNameSpace,
//	};
//	return &d;
//}
//
//#ifdef __cplusplus
//} /* extern "C" */
//#endif
