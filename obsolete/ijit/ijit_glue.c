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
extern "C" {
#endif

#define MOD_jit  40/*TODO*/
#define kjitmod ((kjitmod_t *)kctx->mod[MOD_jit])
#define kmodjit ((kmodjit_t *)kctx->modshare[MOD_jit])
#define GenCodeMtd (kmodjit)->genCode
#define GenCodeDefault (kmodjit)->defaultCodeGen

typedef void (*FgenCode)(KonohaContext *kctx, kMethod *mtd, kBlock *bk);
typedef struct {
	KonohaModule h;
	kMethod *genCode;
	kArray  *constPool;
	kArray  *global_value;
	FgenCode defaultCodeGen;
	KHashMap *jitcache;
	kfileline_t uline;
	KonohaClass *cPointer;
} kmodjit_t;

typedef struct {
	KonohaModuleContext h;
} kjitmod_t;

static void kmodjit_Setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	(void)kctx;(void)def;(void)newctx;
}

static void val_Reftrace(KonohaContext *kctx, KHashMapEntry *p, void *thunk)
{
	KObjectVisitor *visitor = (KObjectVisitor *) thunk;
	BEGIN_REFTRACE(1);
	KRefTrace(p->ObjectValue);
	END_REFTRACE();
}

static void kmodjit_Reftrace(KonohaContext *kctx, struct KonohaModule *baseh, KObjectVisitor *visitor)
{
	kmodjit_t *mod = (kmodjit_t *) baseh;
	BEGIN_REFTRACE(3);
	KRefTrace(mod->genCode);
	KRefTrace(mod->global_value);
	KRefTrace(mod->constPool);
	END_REFTRACE();
	KLIB KHashMap_DoEach(kctx, mod->jitcache, (void *) visitor, val_Reftrace);
}

static void kmodjit_Free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	kmodjit_t *modshare = (kmodjit_t *) baseh;
	KLIB KHashMap_Free(kctx, modshare->jitcache, NULL);
	KFree(baseh, sizeof(kmodjit_t));
}

static void check_stack_size(KonohaContext *kctx, kArray *stack, int n)
{
	while(n >= kArray_size(stack)) {
		KLIB kArray_Add(kctx, stack, K_NULL);
	}
}

static void set_value(KonohaContext *kctx, int index, kObject *o)
{
	kArray  *g = kmodjit->global_value;
	KFieldSet(g, g->ObjectItems[index], o);
}

static kObject *get_value(KonohaContext *kctx, int index)
{
	kArray  *g = kmodjit->global_value;
	return g->ObjectItems[index];
}

//## void System.setUline(int uline);
static KMETHOD System_setUline(KonohaContext *kctx, KonohaStack *sfp)
{
	kmodjit->uline = sfp[1].intValue;
	KReturnVoid();
}
//## void System.getUline();
static KMETHOD System_getUline(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kmodjit->uline);
}

static KMETHOD Expr_getCons(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr = (kExpr *) sfp[0].asObject;
	KReturn(expr->cons);
}

static KMETHOD Expr_getSingle(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr = (kExpr *) sfp[0].asObject;
	DBG_ASSERT(IS_Expr(expr->single));
	KReturn(expr->single);
}

static kArray *get_stack(KonohaContext *kctx, kArray *g)
{
	if(!g->ObjectItems[0]) {
		KFieldSet(g, g->ObjectItems[0], ((kObject *)new_(Array, 0)));
	}
	return (kArray *)g->ObjectItems[0];
}

//## Value System.getValue(int index);
static KMETHOD System_getValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(kctx, g);
	int index = sfp[1].intValue;
	KReturn(stack->ObjectItems[index]);
}

//## void  System.setValue(int index, Value v);
static KMETHOD System_setValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(kctx, g);
	int index = sfp[1].intValue;
	check_stack_size(kctx, stack, index);
	kObject *o = sfp[2].asObject;
	KFieldSet(stack, stack->ObjectItems[index], o);
	KReturnVoid();
}

//## void  System.clearValue();
static KMETHOD System_clearValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(kctx, g);
	KLIB kArray_Clear(kctx, stack, 0);
}

//## Module System.getModule();
static KMETHOD System_getModule(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray  *g = kmodjit->global_value;
	kObject *o = g->ObjectItems[1] ? g->ObjectItems[1] : K_NULL;
	KReturn(o);
}

//## void System.setModule(Module m);
static KMETHOD System_setModule(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 1, sfp[1].asObject);
	KReturnVoid();
}

//## ExecutionEngine System.getExecutionEngine();
static KMETHOD System_getExecutionEngine(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = get_value(kctx, 2);
	KReturn(o);
}

//## void System.setExecutionEngine(ExecutionEngine ee);
static KMETHOD System_setExecutionEngine(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 2, sfp[1].asObject);
	KReturnVoid();
}

//## IRBuilder System.getBuilder();
static KMETHOD System_getBuilder(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = get_value(kctx, 3);
	KReturn(o);
}

//## void System.setBuilder(IRBuilder b);
static KMETHOD System_setBuilder(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 3, sfp[1].asObject);
	KReturnVoid();
}

//## Function System.getFunction();
static KMETHOD System_getFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = get_value(kctx, 4);
	KReturn(o);
}

//## void System.setFunction(Function f);
static KMETHOD System_setFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 4, sfp[1].asObject);
	KReturnVoid();
}

//## Value System.getCTX();
static KMETHOD System_getCTX(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 5));
}

//## void  System.setCTX(Value v);
static KMETHOD System_setCTX(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 5, sfp[1].asObject);
	KReturnVoid();
}

//## Value System.getSFP();
static KMETHOD System_getSFP(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 6));
}

//## void  System.setSFP(Value v);
static KMETHOD System_setSFP(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 6, sfp[1].asObject);
	KReturnVoid();
}

//## Value System.getRetVal();
static KMETHOD System_getRetVal(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 7));
}

//## void  System.setRetVal(Value v);
static KMETHOD System_setRetVal(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 7, sfp[1].asObject);
	KReturnVoid();
}

//## void System.addConstPool(o);
static KMETHOD System_AddConstPool(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[1].asObject;
	kArray  *a = kmodjit->constPool;
	KLIB kArray_Add(kctx, a, o);
	KReturnVoid();
}

static uintptr_t jitcache_hash(kMethod *mtd)
{
	ktypeattr_t cid = mtd->typeId;
	kmethodn_t mn = mtd->mn;
	return (cid << sizeof(short)*8) | mn;
}

static kObject *jitcache_get(KonohaContext *kctx, kMethod *mtd)
{
	uintptr_t hcode = jitcache_hash(mtd);
	KHashMap *map = kmodjit->jitcache;
	KHashMapEntry *e = KLIB KHashMap_get(kctx, map, hcode);
	if(e) {
		return (kObject *) e->unboxValue;
	} else {
		return K_NULL;
	}
}

static void jitcache_set(KonohaContext *kctx, kMethod *mtd, kObject *f)
{
	uintptr_t hcode = jitcache_hash(mtd);
	KHashMap *map = kmodjit->jitcache;
	KHashMapEntry *newe = KLIB KHashMap_newEntry(kctx, map, hcode);
	newe->unboxValue = (uintptr_t) f;
}

//## Function System.getJITCache(Method mtd);
static KMETHOD System_getJITCache(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[1].asMethod;
	kObject *o   = jitcache_get(kctx, mtd);
	KReturn(o);
}

//## void     System.setJITCache(Method mtd, Function f);
static KMETHOD System_setJITCache(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[1].asMethod;
	kObject *o   = sfp[2].asObject;
	jitcache_set(kctx, mtd, o);
	KReturnVoid();
}

//## void System.setLLVMTYPE_HOBJECT(Type type);
static KMETHOD System_setTyHObject(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 8, sfp[1].asObject);
	KReturnVoid();
}

//## void System.setLLVMTYPE_OBJECT(Type type);
static KMETHOD System_setTyObject(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 9, sfp[1].asObject);
	KReturnVoid();
}

//## void System.setLLVMTYPE_FUNCTION(Type type);
static KMETHOD System_setTyFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 10, sfp[1].asObject);
	KReturnVoid();
}

//## void System.setLLVMTYPE_FUNCTIONP(Type type);
static KMETHOD System_setTyFunctionP(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 11, sfp[1].asObject);
	KReturnVoid();
}

//## void System.setLLVMTYPE_METHOD(Type type);
static KMETHOD System_setTyMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 12, sfp[1].asObject);
	KReturnVoid();
}

//## void System.setLLVMTYPE_CONTEXT(Type type);
static KMETHOD System_setTyContext(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 13, sfp[1].asObject);
	KReturnVoid();
}

//## void System.setLLVMTYPE_STACK(Type type);
static KMETHOD System_setTyStack(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 14, sfp[1].asObject);
	KReturnVoid();
}

//## Type System.getLLVMTYPE_HOBJECT();
static KMETHOD System_getTyHObject(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 8));
}

//## Type System.getLLVMTYPE_OBJECT();
static KMETHOD System_getTyObject(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 9));
}

//## Type System.getLLVMTYPE_FUNCTION();
static KMETHOD System_getTyFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 10));
}

//## Type System.getLLVMTYPE_FUNCTIONP();
static KMETHOD System_getTyFunctionP(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 11));
}

//## Type System.getLLVMTYPE_METHOD();
static KMETHOD System_getTyMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 12));
}

//## Type System.getLLVMTYPE_CONTEXT();
static KMETHOD System_getTyContext(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 13));
}

//## Type System.getLLVMTYPE_STACK();
static KMETHOD System_getTyStack(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(get_value(kctx, 14));
}


//## int bk.getEspIndex();
static KMETHOD Block_getEspIndex(KonohaContext *kctx, KonohaStack *sfp)
{
	kBlock *bk = (kBlock *) sfp[0].asObject;
	KReturnUnboxValue(bk->esp->index);
}

//## Array Block.getBlocks();
static KMETHOD Block_getBlocks(KonohaContext *kctx, KonohaStack *sfp)
{
	kBlock *bk = (kBlock *) sfp[0].asObject;
	KReturn(bk->StmtList);
}

//## int Array.getSize();
static KMETHOD Array_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kArray_size(sfp[0].asArray));
}

//## int Stmt.getUline();
static KMETHOD Stmt_getUline(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt = (kStmt *) sfp[0].asObject;
	KReturnUnboxValue(stmt->uline);
}
//## boolean Stmt.hasSyntax();
static KMETHOD Stmt_hasSyntax(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt = (kStmt *) sfp[0].asObject;
	KReturnUnboxValue(stmt->syn != NULL);
}
//## Object Stmt.getObjectNULL(int id);
static KMETHOD Stmt_getObjectNULL(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt = (kStmt *) sfp[0].asObject;
	kObject *o = kStmt_GetObjectNULL(kctx, stmt, sfp[1].intValue);
	if(!o) {
		o = K_NULL;
	}
	KReturn(o);
}

//## int Object.unbox()
static KMETHOD Object_unbox(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[0].asObject;
	KReturnUnboxValue(kObject_Unbox(o));
}


//FIXME TODO stupid down cast
static KMETHOD Object_toStmt(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

kObject *boxing_jit(KonohaContext *kctx, ktypeattr_t cid, uintptr_t data)
{
	return KLIB new_kObjectDontUseThis(kctx, KClass_(cid), data);
}

// --------------------------------------------------------------------------
static kArray *create_array(KonohaContext *kctx, KonohaStack *sfp, int n)
{
	int i;
	kArray *a = new_(Array, 0);
	for (i = 1; i <= n; ++i) {
		KLIB kArray_Add(kctx, a, sfp[i].asObject);
	}
	return a;
}

// --------------------------------------------------------------------------
//## static Array Array.new1(Object o1, Object o2, Object o3);
static KMETHOD Array_new1(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(create_array(kctx, sfp, 1));
}

// --------------------------------------------------------------------------
//## static Array Array.new2(Object o1, Object o2);
static KMETHOD Array_new2(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(create_array(kctx, sfp, 2));
}

// --------------------------------------------------------------------------
//## static Array Array.new3(Object o1, Object o2, Object o3);
static KMETHOD Array_new3(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(create_array(kctx, sfp, 3));
}

// --------------------------------------------------------------------------
//## static Array Array.newN(int n);
static KMETHOD Array_newN(KonohaContext *kctx, KonohaStack *sfp)
{
	int i, n = sfp[1].intValue;
	kArray *a = new_(Array, (uintptr_t)n);
	for (i = 0; i < n; ++i) {
		KLIB kArray_Add(kctx, a, sfp[i].asObject);
	}
	KReturn(a);
}

// --------------------------------------------------------------------------
//## Array Array.getO();
static KMETHOD Array_getO(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = KCheckIndex(kctx, sfp[1].intValue, kArray_size(a), sfp[K_RTNIDX].calledFileLine);
	KReturn(a->ObjectItems[n]);
}
// --------------------------------------------------------------------------
//## void Array.setO(int n, Object o);
static KMETHOD Array_setO(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = KCheckIndex(kctx, sfp[1].intValue, kArray_size(a), sfp[K_RTNIDX].calledFileLine);
	KFieldSet(a, a->ObjectItems[n], sfp[2].asObject);
	KReturnVoid();
}

// Array Array.erase(int n);
static KMETHOD Array_erase(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *src = sfp[0].asArray;
	size_t n = KCheckIndex(kctx, sfp[1].intValue, kArray_size(src), sfp[K_RTNIDX].calledFileLine);
	size_t asize = kArray_size(src);
	size_t i, j = 0;
	kArray *dst = new_(Array, (asize-1));
	for (i = 0; i < asize; ++i) {
		if(i != n) {
			KFieldSet(dst, dst->ObjectItems[j], src->ObjectItems[i]);
			++j;
		}
	}
	KReturn(dst);
}

//## Int Method.getCid();
static KMETHOD Method_getCid(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	KReturnUnboxValue((mtd)->typeId);
}

//## Int Method.getParamSize();
static KMETHOD Method_getParamSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	KReturnUnboxValue(kMethod_GetParam(mtd)->psize);
}

//## Param Method.getParam(int n);
static KMETHOD Method_getParam(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	kParam *pa = kMethod_GetParam(mtd);
	KReturn(pa);
}

//## int Param.getType(int n);
static KMETHOD Param_getType(KonohaContext *kctx, KonohaStack *sfp)
{
	kParam *pa = (kParam *) sfp[0].asObject;
	int n = sfp[1].intValue;
	KReturnUnboxValue(pa->paramtypeItems[n].attrTypeId);
}

//## Int Method.getReturnType();
static KMETHOD Method_getReturnType(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	assert(IS_Method(mtd));
	KReturnUnboxValue(kMethod_GetParam(mtd)->rtype);
}

//## String mtd.getFname();
static KMETHOD Method_getFname(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KLIB KBuffer_printf(kctx, &wb, "%s%s", MethodName_Fmt2(mtd->mn));
	kString *fname = KLIB new_kString(kctx, KLIB KBuffer_text(kctx, &wb, 0), KBuffer_bytesize(&wb), StringPolicy_POOL);
	KReturn(fname);
}

//## String mtd.getCname();
static KMETHOD Method_getCname(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	ktypeattr_t cid = mtd->typeId;
	const char *cname = KType_t(cid);
	KReturn(KLIB new_kString(kctx, cname, strlen(cname), 0));
}

//## Object System.getNULL(kctx, int type);
static KMETHOD System_getNULL(KonohaContext *kctx, KonohaStack *sfp)
{
	ktypeattr_t cid = sfp[1].intValue;
	kObject *o = KLIB Knull(kctx, KClass_(cid));
	KReturn(o);
}

//## boolean Method.isStatic();
static KMETHOD Method_isStatic_(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	kbool_t b = kMethod_Is(Static, mtd);
	KReturnUnboxValue(b);
}

//## boolean Method.isVirtual();
static KMETHOD Method_isVirtual_(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	kbool_t b = kMethod_Is(Virtual, mtd);
	KReturnUnboxValue(b);
}

/****************************************************************/
static uintptr_t *get_Addr(void *addr, intptr_t offset, intptr_t datasize)
{
	return (uintptr_t *)(((char *)addr)+offset);
}
#define PTR_SIZE (sizeof(void *)*8)
//## int Pointer.get(int addr, int offset, int sizeof);
static KMETHOD Pointer_get(KonohaContext *kctx, KonohaStack *sfp)
{
	intptr_t size   = sfp[3].intValue;
	uintptr_t *p = get_Addr((void *)sfp[1].intValue, sfp[2].intValue, size);
	assert(size <= PTR_SIZE);
	KReturnUnboxValue(*p & (~0UL >> (PTR_SIZE - size)));
}

//## void Pointer.set(int addr, int offset, int sizeof, int data);
static KMETHOD Pointer_set(KonohaContext *kctx, KonohaStack *sfp)
{
	intptr_t size   = sfp[3].intValue;
	uintptr_t data  = sfp[4].intValue;
	uintptr_t *p = get_Addr((void *)sfp[1].intValue, sfp[2].intValue, sfp[3].intValue);
	uintptr_t mask = (~0UL >> (PTR_SIZE - size));
	*p = (data & mask) | (*p & ~mask);
	assert(size <= PTR_SIZE);
	KReturnVoid();
}

//## int Object.getAddr();
static KMETHOD Object_getAddr(KonohaContext *kctx, KonohaStack *sfp)
{
	uintptr_t p = (uintptr_t)sfp[0].asObject;
	KReturnUnboxValue(p);
}

//## Int Object.getCid();
static KMETHOD Object_getCid(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue((sfp[0].asObject)->h.ct->typeId);
}

//## var Pointer.asObject(int addr);
static KMETHOD Pointer_toObject(KonohaContext *kctx, KonohaStack *sfp)
{
	uintptr_t addr = (uintptr_t) sfp[1].intValue;
	kObject *o = (kObject *)addr;
	KReturn(o);
}

////FIXME TODO stupid down cast
//static KMETHOD Object_toExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	(void)kctx;
//	KReturn(sfp[0].asObject);
//}

/****************************************************************/
static void _kMethod_GenCode(KonohaContext *kctx, kMethod *mtd, kBlock *bk)
{
	DBG_P("START CODE GENERATION..");
	BEGIN_UnusedStack(lsfp, 8);

	KUnsafeFieldSet(lsfp[K_CALLDELTA+1].asMethod, mtd);
	KUnsafeFieldSet(lsfp[K_CALLDELTA+2].asObject, (kObject *)bk);
	KCALL(lsfp, 0, GenCodeMtd, 2, K_NULL);
	END_UnusedStack();
}

static kbool_t ijit_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("sugar", trace);
	KRequirePackage("konoha.float", trace);
	KRequirePackage("llvm", trace);
	KRequirePackage("konoha.assign", trace);
	KRequirePackage("konoha.null", trace);
	KRequirePackage("konoha.string", trace);
	kmodjit_t *base  = (kmodjit_t *)KCalloc_UNTRACE(sizeof(kmodjit_t), 1);
	base->h.name     = "ijit";
	base->h.setup    = kmodjit_Setup;
	base->h.reftrace = kmodjit_Reftrace;
	base->h.free     = kmodjit_Free;
	base->defaultCodeGen = kctx->klib->kMethod_GenCode;
	base->jitcache = KLIB KHashMap_Init(kctx, 0);
	KUnsafeFieldInit(base->global_value, new_(Array, 18));
	KUnsafeFieldInit(base->constPool, new_(Array, 0));

	typedef struct kPointer {
		KonohaObjectHeader h;
	} kPointer;
	static KDEFINE_CLASS PointerDef = {
		STRUCTNAME(Pointer)
	};
	base->cPointer = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &PointerDef, trace);

	//FIXME
	//KDEFINE_INT_CONST IntData[] = {
	//	{"PTRSIZE", KType_int, sizeof(void *)},
	//	{NULL},
	//};
	//KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(IntData), trace);

	KLIB KonohaRuntime_setModule(kctx, MOD_jit, &base->h, trace);
	return true;
}

static kbool_t ijit_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	/* Array[Expr] */
	kparamtype_t P_ExprArray[] = {{KType_Expr}};
	int KType_ExprArray = (KLIB KonohaClass_Generics(kctx, KClass_Array, KType_void, 1, P_ExprArray))->typeId;

	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KType_System, MN_("genCode"), 0, MPOL_FIRST);
	KUnsafeFieldInit(kmodjit->genCode, mtd);
#define KType_Pointer kmodjit->cPointer->typeId
#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)
#define KType_O  KType_Object
	int FN_x = FN_("x");
	int FN_y = FN_("y");
	int FN_z = FN_("z");
	int FN_w = FN_("w");

	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(System_setUline), KType_void, KType_System, MN_("setUline"), 1, KType_int, FN_x,
		_Public|_Static, _F(System_getUline), KType_int,  KType_System, MN_("getUline"), 0,
		_Public|_Static|_Coercion, _F(System_getValue), KType_Object, KType_System, MN_("getValue"), 1, KType_int, FN_x,
		_Public|_Static|_Coercion, _F(System_setValue), KType_void  , KType_System, MN_("setValue"), 2, KType_int, FN_x, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_clearValue), KType_void  , KType_System, MN_("clearValue"), 0,
		_Public|_Static|_Coercion, _F(System_getModule), KType_Object, KType_System, MN_("getModule"), 0,
		_Public|_Static|_Coercion, _F(System_setModule), KType_void  , KType_System, MN_("setModule"), 1, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getExecutionEngine), KType_Object, KType_System, MN_("getExecutionEngine"), 0,
		_Public|_Static|_Coercion, _F(System_setExecutionEngine), KType_void  , KType_System, MN_("setExecutionEngine"), 1, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getBuilder), KType_Object, KType_System, MN_("getBuilder"),0,
		_Public|_Static|_Coercion, _F(System_setBuilder), KType_void  , KType_System, MN_("setBuilder"),1, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getFunction), KType_Object, KType_System, MN_("getFunction"),0,
		_Public|_Static|_Coercion, _F(System_setFunction), KType_void  , KType_System, MN_("setFunction"),1, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getCTX), KType_Object, KType_System, MN_("getCTX"),0,
		_Public|_Static|_Coercion, _F(System_setCTX), KType_void  , KType_System, MN_("setCTX"),1, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getSFP), KType_Object, KType_System, MN_("getSFP"),0,
		_Public|_Static|_Coercion, _F(System_setSFP), KType_void  , KType_System, MN_("setSFP"),1, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getRetVal), KType_Object, KType_System, MN_("getRetVal"),0,
		_Public|_Static|_Coercion, _F(System_setRetVal), KType_void  , KType_System, MN_("setRetVal"),1, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getJITCache), KType_Object, KType_System, MN_("getJITCache"), 1, KType_Method, FN_x,
		_Public|_Static|_Coercion, _F(System_setJITCache), KType_void  , KType_System, MN_("setJITCache"), 2, KType_Method, FN_x, KType_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_setTyHObject),  KType_void, KType_System, MN_("setTyHObject"),  1, KType_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyObject),   KType_void, KType_System, MN_("setTyObject"),   1, KType_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyFunction), KType_void, KType_System, MN_("setTyFunction"), 1, KType_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyFunctionP),KType_void, KType_System, MN_("setTyFunctionP"),1, KType_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyMethod),   KType_void, KType_System, MN_("setTyMethod"),   1, KType_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyContext),  KType_void, KType_System, MN_("setTyContext"),  1, KType_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyStack),    KType_void, KType_System, MN_("setTyStack"),    1, KType_O, FN_x,
		_Public|_Static|_Coercion, _F(System_getTyHObject),  KType_O,    KType_System, MN_("getTyHObject"), 0,
		_Public|_Static|_Coercion, _F(System_getTyObject),   KType_O,    KType_System, MN_("getTyObject"), 0,
		_Public|_Static|_Coercion, _F(System_getTyFunction), KType_O,    KType_System, MN_("getTyFunction"), 0,
		_Public|_Static|_Coercion, _F(System_getTyFunctionP),KType_O,    KType_System, MN_("getTyFunctionP"), 0,
		_Public|_Static|_Coercion, _F(System_getTyMethod),   KType_O,    KType_System, MN_("getTyMethod"), 0,
		_Public|_Static|_Coercion, _F(System_getTyContext),  KType_O,    KType_System, MN_("getTyContext"), 0,
		_Public|_Static|_Coercion, _F(System_getTyStack),    KType_O,    KType_System, MN_("getTyStack"), 0,
		_Public, _F(Block_getEspIndex), KType_int, KType_Block, MN_("getEspIndex"), 0,
		_Public, _F(Block_getBlocks), KType_Array, KType_Block, MN_("getBlocks"), 0,
		_Public, _F(Array_getSize), KType_int, KType_Array, MN_("getSize"), 0,
		_Public, _F(Stmt_getUline), KType_int, KType_Stmt,  MN_("getUline"), 0,
		_Public, _F(Stmt_hasSyntax), KType_boolean, KType_Stmt,  MN_("hasSyntax"), 0,
		_Public, _F(Stmt_getObjectNULL), KType_O, KType_Stmt, MN_("getObjectNULL"), 1, KType_int, FN_x,
		_Public, _F(System_AddConstPool), KType_void, KType_System, MN_("addConstPool"), 1, KType_O, FN_x,
		_Public, _F(Object_unbox), KType_int, KType_O, MN_("unbox"), 0,
		_Public|_Static, _F(Array_new1), KType_Array, KType_Array, MN_("new1"), 1, KType_O, FN_x,
		_Public|_Static, _F(Array_new2), KType_Array, KType_Array, MN_("new2"), 2, KType_O, FN_x, KType_O, FN_y,
		_Public|_Static, _F(Array_new3), KType_Array, KType_Array, MN_("new3"), 3, KType_O, FN_x, KType_O, FN_y, KType_O, FN_z,
		_Public|_Static, _F(Array_newN), KType_Array, KType_Array, MN_("newN"), 1, KType_int, FN_x,
		_Public, _F(Array_getO), KType_Object, KType_Array, MN_("get"), 1, KType_int, FN_x,
		_Public, _F(Array_setO), KType_void,   KType_Array, MN_("set"), 2, KType_int, FN_x, KType_O, FN_y,
		_Public, _F(Array_erase), KType_Array, KType_Array, MN_("erase"), 1, KType_int, FN_x,
		_Public, _F(Method_getParamSize), KType_int, KType_Method, MN_("getParamSize"), 0,
		_Public, _F(Method_getParam), KType_Param, KType_Method, MN_("getParam"), 0,
		_Public, _F(Param_getType), KType_int, KType_Param, MN_("getType"), 1, KType_int, FN_x,
		_Public, _F(Method_getReturnType), KType_int, KType_Method, MN_("getReturnType"), 0,
		_Public, _F(Method_getCname), KType_String, KType_Method, MN_("getCname"), 0,
		_Public, _F(Method_getFname), KType_String, KType_Method, MN_("getFname"), 0,
		_Public|_Static, _F(System_getNULL), KType_Object, KType_System, MN_("getNULL"), 1, KType_int, FN_x,
		_Public, _F(Method_isStatic_), KType_boolean, KType_Method, MN_("isStatic"), 0,
		_Public, _F(Method_isVirtual_), KType_boolean, KType_Method, MN_("isVirtual"), 0,
		_Public|_Coercion, _F(Object_toStmt), KType_Stmt, KType_Object, MethodName_To(KType_Stmt), 0,
		_Public|_Coercion, _F(Object_toExpr), KType_Expr, KType_Object, MethodName_To(KType_Expr), 0,
		_Public, _F(Expr_getSingle), KType_Expr, KType_Expr, MN_("getSingle"), 0,
		_Public, _F(Expr_getCons), KType_ExprArray, KType_Expr, MN_("getCons"), 0,


		_Public|_Static, _F(Pointer_get), KType_int,  KType_System, MN_("getPointer"),3, KType_int, FN_x, KType_int, FN_y, KType_int, FN_z,
		_Public|_Static, _F(Pointer_set), KType_void, KType_System, MN_("setPointer"),4, KType_int, FN_x, KType_int, FN_y, KType_int, FN_z,KType_int, FN_w,
		_Public|_Coercion, _F(Object_getAddr), KType_int, KType_O, MN_("getAddr"), 0,
		_Public, _F(Object_getCid), KType_int, KType_O, MN_("getCid"), 0,
		_Public, _F(Method_getCid), KType_int, KType_Method, MN_("getCid"), 0,
		//_Public|_Coercion, _F(Object_toStmt), KType_Stmt, KType_Object, MethodName_To(KType_Stmt), 0,

#define TO(T) _Public|_Static, _F(Pointer_toObject), KType_##T, KType_System, MN_("convertTo" # T), 1, KType_int, FN_x
		TO(Array),
		TO(Object),
		TO(Method),
		TO(String),

		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	KonohaLibVar *l = (KonohaLibVar *)kctx->klib;
	l->kMethod_GenCode = GenCodeDefault;
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
	l->kMethod_GenCode = _kMethod_GenCode;

	return true;
}

KDEFINE_PACKAGE* ijit_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("ijit", "1.0"),
		.PackupNameSpace    = ijit_PackupNameSpace,
		.ExportNameSpace   = ijit_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
