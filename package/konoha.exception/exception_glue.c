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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/float.h>

typedef kushort_t kfault_t;
typedef const struct kExceptionVar kException;
typedef struct kExceptionVar kExceptionVar;

struct kExceptionVar {
	KonohaObjectHeader h;
	kshortflag_t flag;   kfault_t faultId;
	kfileline_t  uline;
	kString     *message;
	kArray      *stackTraceList;
};

// Module

#define CFLAG_Exception                            kClass_Final

#define KonohaContext_getExceptionModule(kctx)           ((KonohaExceptionModule*)kctx->modshare[MOD_exception])
#define KonohaContext_getExceptionContext(kctx)          ((KonohaExceptionContext*)kctx->modlocal[MOD_exception])
#define CT_Exception         KonohaContext_getExceptionModule(kctx)->cException
#define TY_Exception         KonohaContext_getExceptionModule(kctx)->cException->typeId
#define IS_Exception(e)      (O_ct(e) == CT_Exception)

typedef struct {
	KonohaModule  h;
	KonohaClass *cException;
	//
} KonohaExceptionModule;

typedef struct {
	KonohaModuleContext h;
	kException        *thrownException;
	//
} KonohaExceptionContext;

/* ------------------------------------------------------------------------ */

static void kException_addStackTrace(KonohaContext *kctx, KonohaStack *sfp, kException *e)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&kctx->stack->cwb, &wb);
	kfileline_t uline = sfp[K_RTNIDX].uline;
	if(uline > 0) {
		const char *file = FileId_t(uline);
		KLIB Kwb_printf(kctx, &wb, "(%s:%d) %s.%s%s" , PLATAPI shortFilePath(file), (kushort_t)uline, Method_t(mtd));
	}
//	int i = 0, psize = Method_paramsize(mtd);
//	kParam *pa = Method_param(mtd);
//	KonohaClass *thisClass = O_cid(sfp[0]);
//	for(i = 0; i < psize; i++) {
//		pa->paramtypeItems[0].ty;
//		if(i > 0) {
//			knh_putc(ctx, cwb->w, ',');
//		}
//		knh_write_fn(ctx, cwb->w, p->fn);
//		knh_putc(ctx, cwb->w, '=');
//		knh_write_sfp(ctx, cwb->w, type, &sfp[i+1], FMT_line);
//	}
	const char *msg = KLIB Kwb_top(kctx, &wb, 1);
	KLIB kArray_add(kctx, e->stackTraceList, KLIB new_kString(kctx, msg, strlen(msg), 0));
//	if((mtd)->mn != MN_LAMBDA) {
//		knh_uline_t uline = knh_stack_uline(ctx, sfp);
//		knh_write_uline(ctx, cwb->w, uline);
//		knh_write_type(ctx, cwb->w, (mtd)->cid);
//		knh_putc(ctx, cwb->w, '.');
//		knh_write_mn(ctx, cwb->w, (mtd)->mn);
//		knh_putc(ctx, cwb->w, '(');
//		knh_putc(ctx, cwb->w, ')');
//		if(DP(e)->tracesNULL == NULL) {
//			KNH_INITv(DP(e)->tracesNULL, new_Array(ctx, CLASS_String, 0));
//		}
//		knh_Array_add(ctx, DP(e)->tracesNULL, knh_cwb_newString(ctx, cwb));
//	}
}

static kbool_t isCalledMethod(KonohaContext *kctx, KonohaStack *sfp)
{
//	kMethod *mtd = sfp[0].mtdNC;
//	if(knh_isObject(kctx, mtd) && IS_Method(mtd)) {
//		//DBG_P("FOUND mtdNC: shift=%d, pc=%d", sfp[-2].shift, sfp[-1].pc);
//		return true;
//	}
	return false;
}

static void Kthrow(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaExceptionContext *ctx = KonohaContext_getExceptionContext(kctx);
	kException *e = ctx->thrownException;
	if(IS_Exception(e)) {
		KonohaStack *p = (sfp == NULL) ? kctx->esp : sfp - 1;
		KonohaStack *bottom = kctx->stack->jump_bottom;
		while(bottom < p) {
			if(p[0].mtdNC != NULL && isCalledMethod(kctx, p)) {
				kException_addStackTrace(kctx, p+1, e);
				p[0].mtdNC = 0;
				//p = p[-1];
			}
			p--;
		}
	}
	//KLIB KonohaRuntime_raise(kctx, 1);
}

/* ------------------------------------------------------------------------ */

//## void System.throw(Exception e);
static KMETHOD System_throw(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaExceptionContext *ctx = KonohaContext_getExceptionContext(kctx);
	KSETv_AND_WRITE_BARRIER(NULL, ctx->thrownException, sfp[1].asException, GC_NO_WRITE_BARRIER);
	Kthrow(kctx, sfp);
}

//## Exception System.getThrownException();
static KMETHOD System_getThrownException(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaExceptionContext *ctx = KonohaContext_getExceptionContext(kctx);
	RETURN_(ctx->thrownException);
}

static KMETHOD Exception_new(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(sfp[0].o);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Hidden   kMethod_Hidden
#define _F(F)     (intptr_t)(F)

// --------------------------------------------------------------------------

static void Exception_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kExceptionVar *e = (kExceptionVar*)o;
	e->flag = 0;
	e->faultId = 0;
	e->uline = 0;
	KINITv(e->message, TS_EMPTY);
	KINITv(e->stackTraceList, K_EMPTYARRAY);
}

static void Exception_reftrace(KonohaContext *kctx, kObject *o)
{
	BEGIN_REFTRACE(2);
	kExceptionVar *e = (kExceptionVar*)o;
	KREFTRACEv(e->message);
	KREFTRACEv(e->stackTraceList);
	END_REFTRACE();

}

static void Exception_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	KLIB Kwb_printf(kctx, wb, "%s", S_text(v[pos].asException->message));
}

static void kModuleException_setup(KonohaContext *kctx, KonohaModule *def, int newctx)
{

}

static void kModuleException_reftrace(KonohaContext *kctx, KonohaModule *baseh)
{
}

static void kModuleException_free(KonohaContext *kctx, KonohaModule *baseh)
{
	KFREE(baseh, sizeof(KonohaExceptionModule));
}

static kbool_t exception_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KonohaExceptionModule *mod = (KonohaExceptionModule*)KCALLOC(sizeof(KonohaExceptionModule), 1);
	mod->h.name     = "exception";
	mod->h.setup    = kModuleException_setup;
	mod->h.reftrace = kModuleException_reftrace;
	mod->h.free     = kModuleException_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_exception, &mod->h, pline);
	KDEFINE_CLASS defException = {
		STRUCTNAME(Exception),
		.cflag = CFLAG_Exception,
		.init  = Exception_init,
		.reftrace = Exception_reftrace,
		.p     = Exception_p,
	};
	mod->cException = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defException, pline);

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Exception_new), TY_Exception,  TY_Exception, MN_("new"), 0, _Public|_Hidden, _F(System_throw), TY_void,  TY_System, MN_("throw"), 1, TY_Exception, FN_("e"),
		_Public|_Hidden, _F(System_getThrownException), TY_Exception, TY_System, MN_("getThrownException"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t exception_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}


//
//static KMETHOD ParseExpr_BRACKET(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_ParseExpr(stmt, tokenException, s, c, e);
//	DBG_P("parse bracket!!");
//	kToken *tk = tokenException->tokenItems[c];
//	if(s == c) { // TODO
//		kExpr *expr = SUGAR kStmt_parseExpr(kctx, stmt, tk->subTokenList, 0, kException_size(tk->subTokenList));
//		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, expr, tokenException, c+1, e));
//	}
//	else {
//		kExpr *lexpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenException, s, c);
//		if(lexpr == K_NULLEXPR) {
//			RETURN_(lexpr);
//		}
//		if(lexpr->syn->keyword == KW_new) {  // new int[100]
//			kExpr_setsyn(lexpr, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall));
//			lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kException_size(tk->subTokenList), 0/*allowEmpty*/);
//		}
//		else {   // X[1] => get X 1
//			kTokenVar *tkN = GCSAFE_new(TokenVar, 0);
//			tkN->keyword = MN_toGETTER(0);
//			tkN->uline = tk->uline;
//			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
//			lexpr  = SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, tkN, lexpr);
//			lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kException_size(tk->subTokenList), 1/*allowEmpty*/);
//		}
//		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, lexpr, tokenException, c+1, e));
//	}
//}

static KMETHOD StmtTyCheck_try(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	DBG_P("try statement .. \n");
	int ret = false;
	kBlock *tryBlock, *catchBlock, *finallyBlock;
	tryBlock     = SUGAR kStmt_getBlock(kctx, stmt, NULL, KW_BlockPattern, K_NULLBLOCK);
	if (SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_Exception, 0)) {
		catchBlock   = SUGAR kStmt_getBlock(kctx, stmt, NULL, SYM_("catch"),   K_NULLBLOCK);
		finallyBlock = SUGAR kStmt_getBlock(kctx, stmt, NULL, SYM_("finally"), K_NULLBLOCK);
		ret = SUGAR kBlock_tyCheckAll(kctx, tryBlock,   gma);
		if (ret == false) {
			RETURNb_(ret);
		}
		ret = SUGAR kBlock_tyCheckAll(kctx, catchBlock, gma);
		if (ret == false) {
			RETURNb_(ret);
		}
		if (finallyBlock) {
			ret = SUGAR kBlock_tyCheckAll(kctx, finallyBlock, gma);
		}
	}
	if(ret) {
		kStmt_typed(stmt, TRY);
	}
	RETURNb_(ret);
}

static kbool_t exception_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("try"), TopStmtTyCheck_(try), StmtTyCheck_(try), .rule = "\"try\" $Block [ \"catch\" \"(\" $Expr \")\" catch: $Block ] [ \"finally\" finally: $Block ]",},
//		{ .keyword = SYM_("[]"), .flag = SYNFLAG_ExprPostfixOp2, ParseExpr_(BRACKET), .precedence_op2 = 16, },  //KW_BracketGroup
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t exception_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* exception_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("exception", "1.0"),
		.initPackage = exception_initPackage,
		.setupPackage = exception_setupPackage,
		.initNameSpace = exception_initNameSpace,
		.setupNameSpace = exception_setupNameSpace,
	};
	return &d;
}
