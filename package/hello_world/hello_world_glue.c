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
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Person {
	KonohaObjectHeader h;
	kString *name;
	kint_t   age;
} kPerson;

static void Person_init(KonohaContext *kctx, kObject *o, void *conf)
{
	/* This function is called when generating the instance of this class.
	 * Moreover, it is called when generating the Null object of this class. */
	struct Person *p = (struct Person *) o;
	p->name = KNULL(String);
	p->age  = 0;
}

static void Person_free(KonohaContext *kctx, kObject *o)
{
	/* This function is called at the time of object destruction. 
	 * It is not necessary to destruct the field of the object which GC has managed. */

	/* Do something
	 * struct Person *p = (struct Person *) o;
	 */
}

static void Person_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	/* This function is called when serializing the object. */
	struct Person *p = (struct Person *) v[pos].o;
	KLIB Kwb_write(kctx, wb, S_text(p->name), S_size(p->name));
	KLIB Kwb_write(kctx, wb, ",", 1);
	KLIB Kwb_printf(kctx, wb, KINT_FMT, p->age);
}

static void Person_reftrace(KonohaContext *kctx, kObject *o)
{
	/* Garbage collector (GC) cannot recognize in which position of the field
	 * an object exists. The function tells to GC which object should be traced. */
	struct Person *p = (struct Person *) o;
	BEGIN_REFTRACE(1/* == field size */);
	/* If p->some_field is Nullable, please use 
	 * KREFTRACEn() macro instead of KREFTRACEv(). */
	KREFTRACEv(p->name);
	/* It is not necessary to trace p->age field,
	 * because p->age is not an Object */
	END_REFTRACE();
}

//## Person Person.new(String name, int age);
static KMETHOD Person_new(KonohaContext *kctx, KonohaStack *sfp)
{
	/* You do not need to allocate object because
	 * object is allocated by Runtime. */
	struct Person *p = (struct Person *) sfp[0].asObject;
	kString *name = sfp[1].asString;
	kint_t   age  = sfp[2].intValue;
	/* If you want to determine the type of the return value,
	 * please check O_ct(sfp[K_RTNIDX].o) . */
	KSETv(p, p->name, name);
	p->age = age;
	RETURN_(p);
}

//## String Person.say();
static KMETHOD Person_say(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Person *p = (struct Person *) sfp[0].asObject;
	kString *name = p->name;
	/* When you want to operate with a raw string, please use S_text() macro
	 * to acquire the pointer of a raw string. */
	const char *text = S_text(name);
	char *buf = (char *)alloca(16 + S_size(name));
	sprintf(buf, "hello , %s!", text);
	RETURN_(KLIB new_kString(kctx, buf, strlen(buf), SPOL_TEXT));
}

/* You can attach the following annotations to each methods. */
#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _Static   kMethod_Static

#define _F(F)     (intptr_t)(F)

static kbool_t HelloWorld_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	KDEFINE_CLASS defPerson = {0};
	SETSTRUCTNAME(defPerson, Person);
	defPerson.cflag     = kClass_Final;
	defPerson.init      = Person_init;
	defPerson.p         = Person_p;
	defPerson.reftrace  = Person_reftrace;
	defPerson.free      = Person_free;
	KonohaClass *PersonClass = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defPerson, pline);

	/* You can define methods with the following procedures. */
	int TY_Person = PersonClass->typeId;
	int FN_x = FN_("x");
	int FN_y = FN_("y");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Person_new), TY_Person, TY_Person, MN_("new"), 2, TY_String, FN_x, TY_int, FN_y,
		_Public, _F(Person_say), TY_String, TY_Person, MN_("say"), 0,
		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	/* You can define constant variable with the following procedures. */
	KDEFINE_INT_CONST IntData[] = {
		{"NARUTO_AGE", TY_int, 18},
		{} /* <= sentinel */
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);
	return true;
}

static kbool_t HelloWorld_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t HelloWorld_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t HelloWorld_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* hello_world_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "hello_world", "1.0");
	d.initPackage    = HelloWorld_initPackage;
	d.setupPackage   = HelloWorld_setupPackage;
	d.initNameSpace  = HelloWorld_initNameSpace;
	d.setupNameSpace = HelloWorld_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
