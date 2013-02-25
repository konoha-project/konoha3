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

#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* @Object */

// @SmartReturn Object Object.to()
static KMETHOD Object_to(KonohaContext *kctx, KonohaStack *sfp)
{
	KClass *selfClass = kObject_class(sfp[0].asObject), *targetClass = KGetReturnType(sfp);
	if(selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass)) {
		sfp[K_RTNIDX].unboxValue = kObject_Unbox(sfp[0].asObject);
		KReturnField(sfp[0].asObject);
	}
	else {
		kNameSpace *ns = KGetLexicalNameSpace(sfp);
		DBG_ASSERT(IS_NameSpace(ns));
		kMethod *mtd = KLIB kNameSpace_GetCoercionMethodNULL(kctx, ns, selfClass, targetClass);
//		DBG_P("BEFORE >>>>>>>>>>> %lld\n", sfp[0].unboxValue);
		sfp[0].unboxValue = kObject_Unbox(sfp[0].asObject);
//		DBG_P("AFTER >>>>>>>>>>> %lld\n", sfp[0].unboxValue);
		if(mtd != NULL && sfp[K_MTDIDX].calledMethod != mtd /* to avoid infinite loop */) {
			sfp[K_MTDIDX].calledMethod = mtd;
			mtd->invokeKMethodFunc(kctx, sfp);
			return;
		}
	}
	kObject *returnValue = KLIB Knull(kctx, targetClass);
	sfp[K_RTNIDX].unboxValue = kObject_Unbox(returnValue);
	KReturnField(returnValue);
}

//## String Object.toString();
static KMETHOD Object_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *self = sfp[0].asObject;
	if(IS_String(self)) {
		KReturnField(self);
	}
	else {
		kNameSpace *ns = KGetLexicalNameSpace(sfp);
		DBG_ASSERT(IS_NameSpace(ns));
		kMethod *mtd = KLIB kNameSpace_GetCoercionMethodNULL(kctx, ns, kObject_class(self), KClass_String);
//		DBG_P("BEFORE >>>>>>>>>>> %s %lld\n", KType_text(kObject_typeId(self)), sfp[0].unboxValue);
		sfp[0].unboxValue = kObject_Unbox(self);
//		DBG_P("AFTER >>>>>>>>>>> %lld\n", sfp[0].unboxValue);
		if(mtd != NULL && sfp[K_MTDIDX].calledMethod != mtd /* to avoid infinite loop */) {
			sfp[K_MTDIDX].calledMethod = mtd;
			mtd->invokeKMethodFunc(kctx, sfp);
			return;
		}
	}
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	kObject_class(sfp[0].asObject)->format(kctx, sfp, 0, &wb);
	KReturn(KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer));
}

//## method Object Object.new();
static KMETHOD Object_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asObject);
}

//## @Const method boolean Object.opEQ(Object x);
static KMETHOD Object_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *self = sfp[0].asObject;
	kObject *that = sfp[1].asObject;
	KClass *ct = kObject_class(self);
	KReturnUnboxValue(ct->compareTo(kctx, self, that) == 0);
}

//## @Const method boolean Object.opNEQ(Object x);
static KMETHOD Object_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *self = sfp[0].asObject;
	kObject *that = sfp[1].asObject;
	KClass *ct = kObject_class(self);
	KReturnUnboxValue(ct->compareTo(kctx, self, that) != 0);
}

//## Boolean Object.isNull();
static KMETHOD Object_isNull(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[0].asObject;
	KReturnUnboxValue(IS_NULL(o));
}

//## Boolean Object.isNotNull();
static KMETHOD Object_isNotNull(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[0].asObject;
	KReturnUnboxValue(!IS_NULL(o));
}

/* String */

//## @Const method Boolean Boolean.toString();
static KMETHOD Boolean_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = (sfp[0].boolValue) ? KSymbol_GetString(kctx, KSymbol_("true")) : KSymbol_GetString(kctx, KSymbol_("false"));
	KReturn(s);
}

//## @Const method Boolean Boolean.opNOT();
static KMETHOD Boolean_opNOT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(!sfp[0].boolValue);
}

//## @Const method Boolean Boolean.opEQ(Boolean x);
static KMETHOD Boolean_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].boolValue == sfp[1].boolValue);
}

//## @Const method Int Int.opMINUS();
static KMETHOD Int_opMINUS(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(-(sfp[0].intValue));
}

//## @Const method Int Int.opADD(Int x);
static KMETHOD Int_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue + sfp[1].intValue);
}

//## @Const method Int Int.opSUB(Int x);
static KMETHOD Int_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue - sfp[1].intValue);
}

//## @Const method Int Int.opMUL(Int x);
static KMETHOD Int_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue * sfp[1].intValue);
}

//## @Const method Int Int.opDIV(Int x);
static KMETHOD Int_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t n = sfp[1].intValue;
	if(unlikely(n == 0)) {
		KMakeTrace(trace, sfp);
		KLIB KRuntime_raise(kctx, KException_("ZeroDivided"), SoftwareFault, NULL, trace->baseStack);
	}
	KReturnUnboxValue(sfp[0].intValue / n);
}

//## @Const method Int Int.opMOD(Int x);
static KMETHOD Int_opMOD(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t n = sfp[1].intValue;
	if(unlikely(n == 0)) {
		KMakeTrace(trace, sfp);
		KLIB KRuntime_raise(kctx, KException_("ZeroDivided"), SoftwareFault, NULL, trace->baseStack);
	}
	KReturnUnboxValue(sfp[0].intValue % n);
}

//## @Const method Boolean Int.opEQ(Int x);
static KMETHOD Int_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue == sfp[1].intValue);
}

//## @Const method Boolean Int.opNEQ(Int x);
static KMETHOD Int_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue != sfp[1].intValue);
}

//## @Const method Boolean Int.opLT(Int x);
static KMETHOD Int_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue < sfp[1].intValue);
}

//## @Const method Boolean Int.opLTE(Int x);
static KMETHOD Int_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue <= sfp[1].intValue);
}

//## @Const method Boolean Int.opGT(Int x);
static KMETHOD Int_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue > sfp[1].intValue);
}

//## @Const method Boolean Int.opGTE(Int x);
static KMETHOD Int_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue >= sfp[1].intValue);
}

//## @Const method String Int.toString();
static KMETHOD Int_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	char buf[40];
	PLATAPI snprintf_i(buf, sizeof(buf), "%ld", (intptr_t)sfp[0].intValue);
	KReturn(KLIB new_kString(kctx, OnStack, buf, strlen(buf), StringPolicy_ASCII));
}

//## @Const method String String.toInt();
static KMETHOD String_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue((kint_t)strtoll(kString_text(sfp[0].asString), NULL, 10));
}

//## @Const @Immutable method String String.opAdd(@Coercion String x);
static KMETHOD String_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *leftHandString = sfp[0].asString, *rightHandString = sfp[1].asString;
	int spol = (kString_Is(ASCII, leftHandString) && kString_Is(ASCII, rightHandString)) ? StringPolicy_ASCII : StringPolicy_UTF8;
	kString *s = KLIB new_kString(kctx, OnStack, NULL, kString_size(leftHandString)+kString_size(rightHandString), spol|StringPolicy_NOCOPY);
	memcpy(s->buf,  kString_text(leftHandString), kString_size(leftHandString));
	memcpy(s->buf + kString_size(leftHandString), kString_text(rightHandString), kString_size(rightHandString));
	KReturn(s);
}

//## @Const method Boolean String.equals(String s);
//## @Const method Boolean String.opEQ(String s);
static KMETHOD String_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	if(kString_size(s0) == kString_size(s1)) {
		KReturnUnboxValue(strncmp(kString_text(s0), kString_text(s1), kString_size(s0)) == 0);
	}
	KReturnUnboxValue(0);
}

static KMETHOD String_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	if(kString_size(s0) == kString_size(s1)) {
		KReturnUnboxValue(strncmp(kString_text(s0), kString_text(s1), kString_size(s0)) != 0);
	}
	KReturnUnboxValue(1);
}

//## This Func.new(Object self, Method mtd);
static KMETHOD Func_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kFuncVar *fo = (kFuncVar *)sfp[0].asFunc;
//	KFieldSet(fo, fo->self, sfp[1].asObject);
	KFieldSet(fo, fo->method,  sfp[2].asMethod);
	KReturn(fo);
}

//## @Hidden T0 Func.invoke();
static KMETHOD Func_invoke(KonohaContext *kctx, KonohaStack *sfp)
{
	kFunc* fo = sfp[0].asFunc;
	DBG_ASSERT(IS_Func(fo));
//	KUnsafeFieldSet(sfp[0].asObject, fo->self);

	KStackCallAgain(sfp, fo->method);
}

//## @Const @Static void System.assert(boolean x)
static KMETHOD NameSpace_assert(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t cond = sfp[1].boolValue;
	if(cond == false) {
		KMakeTrace(trace, sfp);
		((KonohaFactory *)kctx->platApi)->exitStatus = 1;  // just in case
		KLIB KRuntime_raise(kctx, KException_("Assertion"), SoftwareFault, NULL, trace->baseStack);
	}
}

// void NameSpace_AllowImplicitCoercion(boolean t)
static KMETHOD NameSpace_AllowImplicitCoercion(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpaceVar *ns = (kNameSpaceVar *)sfp[0].asNameSpace;
	kNameSpace_Set(ImplicitCoercion, ns, sfp[1].boolValue);
}

//## method void System.p(@Coercion String msg);
static KMETHOD System_p(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *text = (IS_NULL(sfp[1].asString)) ? K_NULLTEXT : kString_text(sfp[1].asString);
	PLATAPI ConsoleModule.ReportUserMessage(kctx, DebugTag, sfp[K_RTNIDX].calledFileLine, text, true/*isNewLine*/);
}

//## method void System.gc();
static KMETHOD System_gc(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	PLATAPI GCModule.ScheduleGC(kctx, trace);
}

// --------------------------------------------------------------------------

static void LoadDefaultMethod(KonohaContext *kctx, kNameSpace *ns)
{
	int FN_x = KFieldName_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Const|_Virtual, _F(Object_opEQ),  KType_Boolean, KType_Object, KMethodName_("=="), 1, KType_Object, FN_x,
		_Public|_Im|_Const|_Virtual, _F(Object_opNEQ), KType_Boolean, KType_Object, KMethodName_("!="), 1, KType_Object, FN_x,
		_Public|_Hidden|_Im|_Const|kMethod_SmartReturn|_Virtual, _F(Object_to), KType_Object, KType_Object, KMethodName_("to"), 0,
		_Public|_Im|_Const|_Virtual, _F(Object_toString), KType_String, KType_Object, KMethodName_To(KType_String), 0,
		_Public|_Im|_Final|_Const, _F(Object_isNull),   KType_Boolean, KType_Object, KMethodName_("IsNull"), 0,
		_Public|_Im|_Final|_Const, _F(Object_isNotNull), KType_Boolean, KType_Object, KMethodName_("IsNotNull"), 0,
		_Public|_Virtual, _F(Object_new), KType_Object, KType_Object, KMethodName_("new"), 0,
		_Public|_Im|_Const, _F(Boolean_toString), KType_String, KType_Boolean, KMethodName_To(KType_String), 0,
		_Public|_Im|_Const, _F(Boolean_opNOT), KType_Boolean, KType_Boolean, KMethodName_("!"), 0,
		_Public|_Im|_Const, _F(Boolean_opEQ), KType_Boolean, KType_Boolean, KMethodName_("=="), 1, KType_Boolean, FN_x,
		_Public|_Im|_Const, _F(Int_opNEQ), KType_Boolean, KType_Boolean, KMethodName_("!="), 1, KType_Boolean, FN_x,
		_Public|_Im|_Const, _F(Int_opMINUS), KType_Int, KType_Int, KMethodName_("-"), 0,
		_Public|_Im|_Const, _F(Int_opADD), KType_Int, KType_Int, KMethodName_("+"), 1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opSUB), KType_Int, KType_Int, KMethodName_("-"), 1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opMUL), KType_Int, KType_Int, KMethodName_("*"), 1, KType_Int, FN_x,
		/* opDIV and opMOD raise zero divided exception. Don't set _Const */
		_Public|_Im, _F(Int_opDIV), KType_Int, KType_Int, KMethodName_("/"), 1, KType_Int, FN_x,
		_Public|_Im, _F(Int_opMOD), KType_Int, KType_Int, KMethodName_("%"), 1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opEQ),  KType_Boolean, KType_Int, KMethodName_("=="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opNEQ), KType_Boolean, KType_Int, KMethodName_("!="), 1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opLT),  KType_Boolean, KType_Int, KMethodName_("<"),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opLTE), KType_Boolean, KType_Int, KMethodName_("<="), 1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opGT),  KType_Boolean, KType_Int, KMethodName_(">"),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opGTE), KType_Boolean, KType_Int, KMethodName_(">="), 1, KType_Int, FN_x,
		_Public|_Im|_Const,  _F(Int_toString), KType_String, KType_Int, KMethodName_To(KType_String), 0,
//		_Public|_Im|_Const|kMethod_SmartReturn|kMethod_Hidden, _F(Boolean_box), KType_Object, KType_Boolean, MN_box, 0,
//		_Public|_Im|_Const|kMethod_SmartReturn|kMethod_Hidden, _F(Int_box), KType_Object, KType_Int, MN_box, 0,
		_Public|_Im|_Const, _F(String_opEQ),  KType_Boolean, KType_String, KMethodName_("=="),  1, KType_String, FN_x ,
		_Public|_Im|_Const, _F(String_opNEQ), KType_Boolean, KType_String, KMethodName_("!="), 1, KType_String, FN_x ,
		_Public|_Im|_Const, _F(String_toInt), KType_Int, KType_String, KMethodName_To(KType_Int), 0,
		_Public|_Im|_Const, _F(String_opADD), KType_String, KType_String, KMethodName_("+"), 1, KType_String | KTypeAttr_Coercion, FN_x,
		_Public|_Const|_Hidden, _F(Func_new), KType_Func, KType_Func, MN_new, 2, KType_Object, FN_x, KType_Method, FN_x,
		_Public|kMethod_SmartReturn|_Hidden, _F(Func_invoke), KType_Object, KType_Func, KMethodName_("Invoke"), 0,
		_Static|_Public|_Im, _F(NameSpace_assert), KType_void, KType_NameSpace, KMethodName_("Assert"), 1, KType_Boolean, FN_x,
		_Public|_Const, _F(NameSpace_AllowImplicitCoercion), KType_void, KType_NameSpace, KMethodName_("AllowImplicitCoercion"), 1, KType_Boolean, KFieldName_("allow"),
		_Static|_Public|_Im, _F(System_p), KType_void, KType_System, KMethodName_("p"), 1, KType_String | KTypeAttr_Coercion, KFieldName_("s"),
		_Static|_Public|_Im, _F(System_gc), KType_void, KType_System, KMethodName_("gc"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, NULL);
}

#ifdef __cplusplus
}
#endif
