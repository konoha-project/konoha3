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
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C"{
#endif

//typedef struct kMapVar kMap;
#define kMap struct kMapVar

struct kMapVar {
	KonohaObjectHeader h;
	KHashMap *map;
};

#define Map_isUnboxData(o)    (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define Map_setUnboxData(o,b) TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local1,b)

static void kMap_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMap *map = (kMap *)o;
	map->map = KLIB Kmap_init(kctx, 17);
	if(TY_isUnbox(O_p0(map))) {
		Map_setUnboxData(map, true);
	}
}

static void MapUnboxEntry_reftrace(KonohaContext *kctx, KHashMapEntry *p, void *thunk)
{
	KObjectVisitor *visitor = (KObjectVisitor *) thunk;
	BEGIN_REFTRACE(1);
	KREFTRACEv(p->StringKey);
	END_REFTRACE();
}

static void MapObjectEntry_reftrace(KonohaContext *kctx, KHashMapEntry *p, void *thunk)
{
	KObjectVisitor *visitor = (KObjectVisitor *) thunk;
	BEGIN_REFTRACE(2);
	KREFTRACEn(p->StringKey);
	KREFTRACEv(p->ObjectValue);
	END_REFTRACE();
}

static void kMap_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kMap *map = (kMap *)o;
	if(TY_isUnbox(O_p0(map))) {
		KLIB Kmap_each(kctx, map->map, (void *)visitor, MapUnboxEntry_reftrace);
	}
	else {
		KLIB Kmap_each(kctx, map->map, (void *)visitor, MapObjectEntry_reftrace);
	}
}

static void kMap_free(KonohaContext *kctx, kObject *o)
{
	kMap *map = (kMap *)o;
	if(map->map != NULL) {
		KLIB Kmap_free(kctx, map->map, NULL);
	}
}

//static void kMap_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KGrowingBuffer *wb)
//{
//	// TODO
//}

/* ------------------------------------------------------------------------ */
/* method */

static uintptr_t String_hashCode(KonohaContext *kctx, kString *s)
{
	return strhash(S_text(s), S_size(s));  // FIXME: slow

}

static KHashMapEntry* kMap_getEntry(KonohaContext *kctx, kMap *m, kString *key, kbool_t isNewIfNULL)
{
	uintptr_t hcode = String_hashCode(kctx, key);
	const char *tkey = S_text(key);
	size_t tlen = S_size(key);
	KHashMapEntry *e = KLIB Kmap_get(kctx, m->map, hcode);
	while(e != NULL) {
		if(e->hcode == hcode && tlen == S_size(e->StringKey) && strncmp(S_text(e->StringKey), tkey, tlen) == 0) {
			return e;
		}
		e = e->next;
	}
	if(isNewIfNULL) {
		e = KLIB Kmap_newEntry(kctx, m->map, hcode);
		KUnsafeFieldInit(e->StringKey, key);
		if(!Map_isUnboxData(m)) {
			KUnsafeFieldInit(e->ObjectValue, K_NULL);
		}
		return e;
	}
	return NULL;
}

//## method Boolean Map.has(T1 key);
static KMETHOD Map_has(KonohaContext *kctx, KonohaStack *sfp)
{
	kMap *m = (kMap *)sfp[0].asObject;
	KHashMapEntry *e = kMap_getEntry(kctx, m, sfp[1].asString, false/*new_if_NULL*/);
	KReturnUnboxValue((e != NULL));
}

//## T0 Map.get(String key);
static KMETHOD Map_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kMap *m = (kMap *)sfp[0].asObject;
	KHashMapEntry *e = kMap_getEntry(kctx, m, sfp[1].asString, false/*new_if_NULL*/);
	if(Map_isUnboxData(m)) {
		uintptr_t u = (e == NULL) ? 0 : e->unboxValue;
		KReturnUnboxValue(u);
	}
	else if(e != NULL) {
		KReturn(e->ObjectValue);
	}
	KReturnDefaultValue();
}

//## method void Map.set(String key, T0 value);
static KMETHOD Map_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kMap *m = (kMap *)sfp[0].asObject;
	KHashMapEntry *e = kMap_getEntry(kctx, m, sfp[1].asString, true/*new_if_NULL*/);
	if(Map_isUnboxData(m)) {
		e->unboxValue = sfp[2].unboxValue;
	}
	else {
		KFieldSet(m, e->ObjectValue, sfp[2].asObject);
	}
	KReturnVoid();
}

//## method void Map.remove(String key);
static KMETHOD Map_remove(KonohaContext *kctx, KonohaStack *sfp)
{
	kMap *m = (kMap *)sfp[0].asObject;
	KHashMapEntry *e = kMap_getEntry(kctx, m, sfp[1].asString, false/*new_if_NULL*/);
	if(e != NULL) {
		KLIB Kmap_remove(m->map, e);
	}
	KReturnVoid();
}

static void MapEntry_appendKey(KonohaContext *kctx, KHashMapEntry *p, void *thunk)
{
	kArray *a = (kArray *)thunk;
	KLIB kArray_add(kctx, a, p->StringKey);
}

//## T0[] Map.keys();
static KMETHOD Map_keys(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kMap *m = (kMap *)sfp[0].asObject;
	KonohaClass *cArray = CT_p0(kctx, CT_Array, O_p0(m));
	kArray *a = (kArray *)(KLIB new_kObject(kctx, _GcStack, cArray, m->map->size));
	KLIB Kmap_each(kctx, m->map, (void *)a, MapEntry_appendKey);
	KReturnWith(a, RESET_GCSTACK());
}

//## Map<T> Map<T>.new();
static KMETHOD Map_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asObject);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _F(F)     (intptr_t)(F)

#define TY_Map cMap->typeId

static kbool_t map_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	kparamtype_t cparam = {TY_Object};
	KDEFINE_CLASS defMap = {0};
	SETSTRUCTNAME(defMap, Map);
	defMap.cflag     = kClass_Final;
	defMap.cparamsize = 1;
	defMap.cParamItems = &cparam;
	defMap.init      = kMap_init;
	defMap.reftrace  = kMap_reftrace;
	defMap.free      = kMap_free;

	KonohaClass *cMap = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defMap, trace);
	int FN_key = MN_("key");
	int TY_Array0 = CT_p0(kctx, CT_Array, TY_0)->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Map_new), TY_Map, TY_Map, MN_("new"), 0,
		_Public|_Im|_Const, _F(Map_has), TY_boolean, TY_Map, MN_("has"), 1, TY_String, FN_key,
		_Public|_Im|_Const, _F(Map_get), TY_0, TY_Map, MN_("get"), 1, TY_String, FN_key,
		_Public, _F(Map_set), TY_void, TY_Map, MN_("set"), 2, TY_String, FN_key, TY_0, FN_("value"),
		_Public, _F(Map_remove), TY_void, TY_Map, MN_("remove"), 1, TY_String, FN_key,
		_Public|_Im|_Const, _F(Map_keys), TY_Array0, TY_Map, MN_("keys"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

/* ----------------------------------------------------------------------- */

static KMETHOD TypeCheck_MapLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *termToken = expr->termToken;
	if(Expr_isTerm(expr) && IS_Token(termToken)) {
		DBG_P("termToken='%s'", S_text(termToken->text));

	}
}

static kbool_t map_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KW_BlockPattern, SugarFunc_TypeCheck, new_SugarFunc(ns, TypeCheck_MapLiteral));
	return true;
}

static kbool_t map_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	map_defineMethod(kctx, ns, trace);
	map_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t map_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* map_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "map", "1.0");
	d.PackupNameSpace    = map_PackupNameSpace;
	d.ExportNameSpace   = map_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
