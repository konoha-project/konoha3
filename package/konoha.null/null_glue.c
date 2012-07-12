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

#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>

// --------------------------------------------------------------------------
//## Boolean Object.isNull();
static KMETHOD Object_isNull(CTX, ksfp_t *sfp _RIX)
{
	kObject *o = sfp[0].o;
	RETURNb_(IS_NULL(o));
}

//## Boolean Object.isNotNull();
static KMETHOD Object_isNotNull(CTX, ksfp_t *sfp _RIX)
{
	kObject *o = sfp[0].o;
	RETURNb_(!IS_NULL(o));
}

// --------------------------------------------------------------------------

#define _F(F) (intptr_t)(F)
static kbool_t null_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	intptr_t MethodData[] = {
		kMethod_Public, _F(Object_isNull), TY_Boolean, TY_Object, MN_("isNull"), 0,
		kMethod_Public, _F(Object_isNotNull), TY_Boolean, TY_Object, MN_("isNotNull"), 0,
		DEND,
	};
	kNameSpace_loadMethodData(ks, MethodData);

	return true;
}

static kbool_t null_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static KMETHOD ExprTyCheck_null(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	DBG_P("typing null as %s", TY_t(reqty));
	if(reqty == TY_var) reqty = TY_Object;
	RETURN_(kExpr_setVariable(expr, NULL, reqty, 0, gma));
}

static kbool_t null_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
{
	USING_SUGAR;
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .kw = SYM_("null"), _TERM, ExprTyCheck_(null), },
		{ .kw = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(_ctx, ks, SYNTAX);
	return true;
}

static kbool_t null_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* null_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("null", "1.0"),
		.initPackage = null_initPackage,
		.setupPackage = null_setupPackage,
		.initNameSpace = null_initNameSpace,
		.setupNameSpace = null_setupNameSpace,
	};
	return &d;
}

