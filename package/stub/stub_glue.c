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

#include<konoha2/konoha2.h>
#include<konoha2/sugar.h>

// Expr Expr.tyCheckStub(Gamma gma, int reqtyid);
//static KMETHOD ExprTyCheck_stub(CTX, ksfp_t *sfp _RIX)
//{
//	VAR_ExprTyCheck(stmt, expr, gma, reqty);
//	DBG_P("stub: size=%d", kArray_size(expr->consNUL));
//	RETURN_(K_NULLEXPR);
//}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static	kbool_t stub_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
//	KDEFINE_METHOD MethodData[] = {
//		_Public, _F(Stmt_setBuild), TY_void, TY_Stmt, MN_("setBuild"), 1, TY_Int, FN_buildid,
//		_Public, _F(Stmt_getBlock), TY_Block, TY_Stmt, MN_("getBlock"), 2, TY_String, FN_key, TY_Block, FN_defval,
//		_Public, _F(Stmt_tyCheckExpr), TY_Boolean, TY_Stmt, MN_("tyCheckExpr"), 4, TY_String, FN_key, TY_Gamma, FN_gma, TY_Int, FN_typeid, TY_Int, FN_pol,
//		_Public, _F(Block_tyCheckAll), TY_Boolean, TY_Block, MN_("tyCheckAll"), 1, TY_Gamma, FN_gma,
//		_Public, _F(NameSpace_defineSyntaxRule), TY_void, TY_NameSpace, MN_("defineSyntaxRule"),   2, TY_String, FN_key, TY_String, FN_("rule"),
//		_Public, _F(NameSpace_defineStmtTyCheck), TY_void, TY_NameSpace, MN_("defineStmtTyCheck"), 2, TY_String, FN_key, TY_String, FN_methodname,
//		DEND,
//	};
//	kNameSpace_loadMethodData(NULL, MethodData);
	return true;
}

static kbool_t stub_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t stub_initNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
//	USING_SUGAR;
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ TOKEN("float"), .type = TY_Float, },
//		{ TOKEN("double"), .type = TY_Float, },
//		{ TOKEN("$FLOAT"), .kw = KW_TK(TK_FLOAT), .ExprTyCheck = ExprTyCheck_FLOAT, },
//		{ .kw = KW_END, },
//	};
//	SUGAR NameSpace_defineSyntax(_ctx, ks, SYNTAX);
	return true;
}

static kbool_t stub_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* stub_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("stub", "1.0"),
		.initPackage = stub_initPackage,
		.setupPackage = stub_setupPackage,
		.initNameSpace = stub_initNameSpace,
		.setupNameSpace = stub_setupNameSpace,
	};
	return &d;
}
