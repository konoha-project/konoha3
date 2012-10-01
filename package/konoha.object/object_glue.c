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

#ifdef __cplusplus
extern "C"{
#endif

// Object.getTypeId()

static KMETHOD Object_getTypeId(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(O_ct(sfp[0].asObject)->typeId);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Hidden   kMethod_Hidden
#define _Imm      kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t object_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("konoha.subtype", pline);
	KDEFINE_INT_CONST ClassData[] = {   // add Object as available
		{"Object", TY_TYPE, (uintptr_t)CT_(TY_Object)},
		{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(ClassData), 0);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const, _F(Object_getTypeId), TY_int, TY_Object, MN_("getTypeId"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t object_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

//static KMETHOD ExprTyCheck_Getter(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_ExprTyCheck(stmt, expr, gma, reqty);
//	kToken *tkN = expr->cons->tokenItems[0];
//	ksymbol_t fn = tkN->resolvedSymbol;
//	kExpr *self = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
//	kNameSpace *ns = Stmt_nameSpace(stmt);
//	if(self != K_NULLEXPR) {
//		kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, self->ty, MN_toGETTER(fn), 0, MPOL_GETTER);
//		if(mtd != NULL) {
//			KSETv(expr->cons, expr->cons->methodItems[0], mtd);
//			RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, reqty));
//		}
//		SUGAR kStmt_printMessage2(kctx, stmt, tkN, ErrTag, "undefined field: %s", S_text(tkN->text));
//	}
//	RETURN_(K_NULLEXPR);
//}

// ----------------------------------------------------------------------------

static kbool_t object_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KImportPackage(ns, "konoha.subtype", pline);
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ .keyword = SYM_("."), ExprTyCheck_(Getter) },
//		{ .keyword = KW_END, },
//	};
//	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t object_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* object_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "object", "1.0");
	d.initPackage    = object_initPackage;
	d.setupPackage   = object_setupPackage;
	d.initNameSpace  = object_initNameSpace;
	d.setupNameSpace = object_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
