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
#define kjitmod ((kjitmod_t*)kctx->mod[MOD_jit])
#define kmodjit ((kmodjit_t*)kctx->modshare[MOD_jit])
#define GenCodeMtd (kmodjit)->genCode
#define GenCodeDefault (kmodjit)->defaultCodeGen

typedef void (*FgenCode)(KonohaContext *kctx, kMethod *mtd, kBlock *bk);
typedef struct {
	KonohaModule h;
	kMethod *genCode;
	kArray  *constPool;
	kArray  *global_value;
	FgenCode defaultCodeGen;
	KUtilsHashMap *jitcache;
	kfileline_t uline;
	KonohaClass *cPointer;
} kmodjit_t;

typedef struct {
	KonohaModuleContext h;
} kjitmod_t;

static void kmodjit_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	(void)kctx;(void)def;(void)newctx;
}

static void val_reftrace(KonohaContext *kctx, KUtilsHashMapEntry *p)
{
	BEGIN_REFTRACE(1);
	KREFTRACEv(p->objectValue);
	END_REFTRACE();
}

static void kmodjit_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
	kmodjit_t *mod = (kmodjit_t *) baseh;
	BEGIN_REFTRACE(3);
	KREFTRACEv(mod->genCode);
	KREFTRACEv(mod->global_value);
	KREFTRACEv(mod->constPool);
	END_REFTRACE();
	KLIB Kmap_each(kctx, mod->jitcache, val_reftrace);
}

static void kmodjit_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	kmodjit_t *modshare = (kmodjit_t*) baseh;
	KLIB Kmap_free(kctx, modshare->jitcache, NULL);
	KFREE(baseh, sizeof(kmodjit_t));
}

static void check_stack_size(KonohaContext *kctx, kArray *stack, int n)
{
	while (n >= kArray_size(stack)) {
		KLIB kArray_add(kctx, stack, K_NULL);
	}
}

static void set_value(KonohaContext *kctx, int index, kObject *o)
{
	kArray  *g = kmodjit->global_value;
	KSETv(g, g->objectItems[index], o);
}

static kObject *get_value(KonohaContext *kctx, int index)
{
	kArray  *g = kmodjit->global_value;
	return g->objectItems[index];
}

//## void System.setUline(int uline);
static KMETHOD System_setUline(KonohaContext *kctx, KonohaStack *sfp)
{
	kmodjit->uline = sfp[1].intValue;
	RETURNvoid_();
}
//## void System.getUline();
static KMETHOD System_getUline(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(kmodjit->uline);
}

static KMETHOD Expr_getCons(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr = (kExpr *) sfp[0].asObject;
	RETURN_(expr->cons);
}

static KMETHOD Expr_getSingle(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr = (kExpr *) sfp[0].asObject;
	DBG_ASSERT(IS_Expr(expr->single));
	RETURN_(expr->single);
}

static kArray *get_stack(KonohaContext *kctx, kArray *g)
{
	if (!g->objectItems[0]) {
		KSETv(g, g->objectItems[0], ((kObject*)new_(Array, 0)));
	}
	return (kArray*)g->objectItems[0];
}

//## Value System.getValue(int index);
static KMETHOD System_getValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(kctx, g);
	int index = sfp[1].intValue;
	RETURN_(stack->objectItems[index]);
}

//## void  System.setValue(int index, Value v);
static KMETHOD System_setValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(kctx, g);
	int index = sfp[1].intValue;
	check_stack_size(kctx, stack, index);
	kObject *o = sfp[2].o;
	KSETv(stack, stack->objectItems[index], o);
	RETURNvoid_();
}

//## void  System.clearValue();
static KMETHOD System_clearValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(kctx, g);
	KLIB kArray_clear(kctx, stack, 0);
}

//## Module System.getModule();
static KMETHOD System_getModule(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray  *g = kmodjit->global_value;
	kObject *o = g->objectItems[1] ? g->objectItems[1] : K_NULL;
	RETURN_(o);
}

//## void System.setModule(Module m);
static KMETHOD System_setModule(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 1, sfp[1].asObject);
	RETURNvoid_();
}

//## ExecutionEngine System.getExecutionEngine();
static KMETHOD System_getExecutionEngine(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = get_value(kctx, 2);
	RETURN_(o);
}

//## void System.setExecutionEngine(ExecutionEngine ee);
static KMETHOD System_setExecutionEngine(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 2, sfp[1].asObject);
	RETURNvoid_();
}

//## IRBuilder System.getBuilder();
static KMETHOD System_getBuilder(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = get_value(kctx, 3);
	RETURN_(o);
}

//## void System.setBuilder(IRBuilder b);
static KMETHOD System_setBuilder(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 3, sfp[1].asObject);
	RETURNvoid_();
}

//## Function System.getFunction();
static KMETHOD System_getFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = get_value(kctx, 4);
	RETURN_(o);
}

//## void System.setFunction(Function f);
static KMETHOD System_setFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 4, sfp[1].asObject);
	RETURNvoid_();
}

//## Value System.getCTX();
static KMETHOD System_getCTX(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 5));
}

//## void  System.setCTX(Value v);
static KMETHOD System_setCTX(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 5, sfp[1].asObject);
	RETURNvoid_();
}

//## Value System.getSFP();
static KMETHOD System_getSFP(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 6));
}

//## void  System.setSFP(Value v);
static KMETHOD System_setSFP(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 6, sfp[1].asObject);
	RETURNvoid_();
}

//## Value System.getRetVal();
static KMETHOD System_getRetVal(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 7));
}

//## void  System.setRetVal(Value v);
static KMETHOD System_setRetVal(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 7, sfp[1].asObject);
	RETURNvoid_();
}

//## void System.addConstPool(o);
static KMETHOD System_addConstPool(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[1].asObject;
	kArray  *a = kmodjit->constPool;
	KLIB kArray_add(kctx, a, o);
	RETURNvoid_();
}

static uintptr_t jitcache_hash(kMethod *mtd)
{
	ktype_t cid = mtd->typeId;
	kmethodn_t mn = mtd->mn;
	return (cid << sizeof(short)*8) | mn;
}

static kObject *jitcache_get(KonohaContext *kctx, kMethod *mtd)
{
	uintptr_t hcode = jitcache_hash(mtd);
	KUtilsHashMap *map = kmodjit->jitcache;
	KUtilsHashMapEntry *e = KLIB Kmap_get(kctx, map, hcode);
	if (e) {
		return (kObject*) e->unboxValue;
	} else {
		return K_NULL;
	}
}

static void jitcache_set(KonohaContext *kctx, kMethod *mtd, kObject *f)
{
	uintptr_t hcode = jitcache_hash(mtd);
	KUtilsHashMap *map = kmodjit->jitcache;
	KUtilsHashMapEntry *newe = KLIB Kmap_newEntry(kctx, map, hcode);
	newe->unboxValue = (uintptr_t) f;
}

//## Function System.getJITCache(Method mtd);
static KMETHOD System_getJITCache(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[1].asMethod;
	kObject *o   = jitcache_get(kctx, mtd);
	RETURN_(o);
}

//## void     System.setJITCache(Method mtd, Function f);
static KMETHOD System_setJITCache(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[1].asMethod;
	kObject *o   = sfp[2].asObject;
	jitcache_set(kctx, mtd, o);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_HOBJECT(Type type);
static KMETHOD System_setTyHObject(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 8, sfp[1].asObject);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_OBJECT(Type type);
static KMETHOD System_setTyObject(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 9, sfp[1].asObject);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_FUNCTION(Type type);
static KMETHOD System_setTyFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 10, sfp[1].asObject);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_FUNCTIONP(Type type);
static KMETHOD System_setTyFunctionP(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 11, sfp[1].asObject);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_METHOD(Type type);
static KMETHOD System_setTyMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 12, sfp[1].asObject);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_CONTEXT(Type type);
static KMETHOD System_setTyContext(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 13, sfp[1].asObject);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_STACK(Type type);
static KMETHOD System_setTyStack(KonohaContext *kctx, KonohaStack *sfp)
{
	set_value(kctx, 14, sfp[1].asObject);
	RETURNvoid_();
}

//## Type System.getLLVMTYPE_HOBJECT();
static KMETHOD System_getTyHObject(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 8));
}

//## Type System.getLLVMTYPE_OBJECT();
static KMETHOD System_getTyObject(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 9));
}

//## Type System.getLLVMTYPE_FUNCTION();
static KMETHOD System_getTyFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 10));
}

//## Type System.getLLVMTYPE_FUNCTIONP();
static KMETHOD System_getTyFunctionP(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 11));
}

//## Type System.getLLVMTYPE_METHOD();
static KMETHOD System_getTyMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 12));
}

//## Type System.getLLVMTYPE_CONTEXT();
static KMETHOD System_getTyContext(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 13));
}

//## Type System.getLLVMTYPE_STACK();
static KMETHOD System_getTyStack(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(get_value(kctx, 14));
}


//## int bk.getEspIndex();
static KMETHOD Block_getEspIndex(KonohaContext *kctx, KonohaStack *sfp)
{
	kBlock *bk = (kBlock *) sfp[0].asObject;
	RETURNi_(bk->esp->index);
}

//## Array Block.getBlocks();
static KMETHOD Block_getBlocks(KonohaContext *kctx, KonohaStack *sfp)
{
	kBlock *bk = (kBlock *) sfp[0].asObject;
	RETURN_(bk->stmtList);
}

//## int Array.getSize();
static KMETHOD Array_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(kArray_size(sfp[0].asArray));
}

//## int Stmt.getUline();
static KMETHOD Stmt_getUline(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt = (kStmt*) sfp[0].asObject;
	RETURNi_(stmt->uline);
}
//## boolean Stmt.hasSyntax();
static KMETHOD Stmt_hasSyntax(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt = (kStmt*) sfp[0].asObject;
	RETURNb_(stmt->syn != NULL);
}
//## Object Stmt.getObjectNULL(int id);
static KMETHOD Stmt_getObjectNULL(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt = (kStmt*) sfp[0].asObject;
	kObject *o = kStmt_getObjectNULL(kctx, stmt, sfp[1].intValue);
	if (!o) {
		o = K_NULL;
	}
	RETURN_(o);
}

//## int Object.unbox()
static KMETHOD Object_unbox(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[0].asObject;
	RETURNi_(O_unbox(o));
}


//FIXME TODO stupid down cast
static KMETHOD Object_toStmt(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

kObject *boxing_jit(KonohaContext *kctx, ktype_t cid, uintptr_t data)
{
	return KLIB new_kObject(kctx, CT_(cid), data);
}

// --------------------------------------------------------------------------
static kArray *create_array(KonohaContext *kctx, KonohaStack *sfp, int n)
{
	int i;
	kArray *a = new_(Array, 0);
	for (i = 1; i <= n; ++i) {
		KLIB kArray_add(kctx, a, sfp[i].o);
	}
	return a;
}

// --------------------------------------------------------------------------
//## static Array Array.new1(Object o1, Object o2, Object o3);
static KMETHOD Array_new1(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(create_array(kctx, sfp, 1));
}

// --------------------------------------------------------------------------
//## static Array Array.new2(Object o1, Object o2);
static KMETHOD Array_new2(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(create_array(kctx, sfp, 2));
}

// --------------------------------------------------------------------------
//## static Array Array.new3(Object o1, Object o2, Object o3);
static KMETHOD Array_new3(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(create_array(kctx, sfp, 3));
}

// --------------------------------------------------------------------------
//## static Array Array.newN(int n);
static KMETHOD Array_newN(KonohaContext *kctx, KonohaStack *sfp)
{
	int i, n = sfp[1].intValue;
	kArray *a = new_(Array, (uintptr_t)n);
	for (i = 0; i < n; ++i) {
		KLIB kArray_add(kctx, a, sfp[i].o);
	}
	RETURN_(a);
}

// --------------------------------------------------------------------------
//## Array Array.getO();
static KMETHOD Array_getO(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = check_index(kctx, sfp[1].intValue, kArray_size(a), sfp[K_RTNIDX].uline);
	RETURN_(a->objectItems[n]);
}
// --------------------------------------------------------------------------
//## void Array.setO(int n, Object o);
static KMETHOD Array_setO(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = check_index(kctx, sfp[1].intValue, kArray_size(a), sfp[K_RTNIDX].uline);
	KSETv(a, a->objectItems[n], sfp[2].o);
	RETURNvoid_();
}

// Array Array.erase(int n);
static KMETHOD Array_erase(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *src = sfp[0].asArray;
	size_t n = check_index(kctx, sfp[1].intValue, kArray_size(src), sfp[K_RTNIDX].uline);
	size_t asize = kArray_size(src);
	size_t i, j = 0;
	kArray *dst = new_(Array, (asize-1));
	for (i = 0; i < asize; ++i) {
		if (i != n) {
			KSETv(dst, dst->objectItems[j], src->objectItems[i]);
			++j;
		}
	}
	RETURN_(dst);
}

//## Int Method.getCid();
static KMETHOD Method_getCid(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	RETURNi_((mtd)->typeId);
}

//## Int Method.getParamSize();
static KMETHOD Method_getParamSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	RETURNi_(Method_param(mtd)->psize);
}

//## Param Method.getParam(int n);
static KMETHOD Method_getParam(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	kParam *pa = Method_param(mtd);
	RETURN_(pa);
}

//## int Param.getType(int n);
static KMETHOD Param_getType(KonohaContext *kctx, KonohaStack *sfp)
{
	kParam *pa = (kParam *) sfp[0].asObject;
	int n = sfp[1].intValue;
	RETURNi_(pa->paramtypeItems[n].ty);
}

//## Int Method.getReturnType();
static KMETHOD Method_getReturnType(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	assert(IS_Method(mtd));
	RETURNi_(Method_param(mtd)->rtype);
}

//## String mtd.getFname();
static KMETHOD Method_getFname(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	KLIB Kwb_printf(kctx, &wb, "%s%s", T_mn(mtd->mn));
	kString *fname = KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), StringPolicy_POOL);
	RETURN_(fname);
}

//## String mtd.getCname();
static KMETHOD Method_getCname(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	ktype_t cid = mtd->typeId;
	const char *cname = TY_t(cid);
	RETURN_(KLIB new_kString(kctx, cname, strlen(cname), 0));
}

//## Object System.getNULL(kctx, int type);
static KMETHOD System_getNULL(KonohaContext *kctx, KonohaStack *sfp)
{
	ktype_t cid = sfp[1].intValue;
	kObject *o = KLIB Knull(kctx, CT_(cid));
	RETURN_(o);
}

//## boolean Method.isStatic();
static KMETHOD Method_isStatic_(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	kbool_t b = kMethod_is(Static, mtd);
	RETURNb_(b);
}

//## boolean Method.isVirtual();
static KMETHOD Method_isVirtual_(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[0].asMethod;
	kbool_t b = kMethod_is(Virtual, mtd);
	RETURNb_(b);
}

/****************************************************************/
static uintptr_t *get_addr(void *addr, intptr_t offset, intptr_t datasize)
{
	return (uintptr_t*)(((char*)addr)+offset);
}
#define PTR_SIZE (sizeof(void*)*8)
//## int Pointer.get(int addr, int offset, int sizeof);
static KMETHOD Pointer_get(KonohaContext *kctx, KonohaStack *sfp)
{
	intptr_t size   = sfp[3].intValue;
	uintptr_t *p = get_addr((void*)sfp[1].intValue, sfp[2].intValue, size);
	assert(size <= PTR_SIZE);
	RETURNi_(*p & (~0UL >> (PTR_SIZE - size)));
}

//## void Pointer.set(int addr, int offset, int sizeof, int data);
static KMETHOD Pointer_set(KonohaContext *kctx, KonohaStack *sfp)
{
	intptr_t size   = sfp[3].intValue;
	uintptr_t data  = sfp[4].intValue;
	uintptr_t *p = get_addr((void*)sfp[1].intValue, sfp[2].intValue, sfp[3].intValue);
	uintptr_t mask = (~0UL >> (PTR_SIZE - size));
	*p = (data & mask) | (*p & ~mask);
	assert(size <= PTR_SIZE);
	RETURNvoid_();
}

//## int Object.getAddr();
static KMETHOD Object_getAddr(KonohaContext *kctx, KonohaStack *sfp)
{
	uintptr_t p = (uintptr_t)sfp[0].asObject;
	RETURNi_(p);
}

//## Int Object.getCid();
static KMETHOD Object_getCid(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_((sfp[0].asObject)->h.ct->typeId);
}

//## var Pointer.asObject(int addr);
static KMETHOD Pointer_toObject(KonohaContext *kctx, KonohaStack *sfp)
{
	uintptr_t addr = (uintptr_t) sfp[1].intValue;
	kObject *o = (kObject*)addr;
	RETURN_(o);
}

////FIXME TODO stupid down cast
//static KMETHOD Object_toExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	(void)kctx;
//	RETURN_(sfp[0].asObject);
//}

/****************************************************************/
static void _kMethod_genCode(KonohaContext *kctx, kMethod *mtd, kBlock *bk)
{
	DBG_P("START CODE GENERATION..");
	BEGIN_LOCAL(lsfp, 8);

	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].asMethod, mtd, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].asObject, (kObject*)bk, GC_NO_WRITE_BARRIER);
	KCALL(lsfp, 0, GenCodeMtd, 2, K_NULL);
	END_LOCAL();
}

static kbool_t ijit_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("sugar", pline);
	KRequirePackage("konoha.float", pline);
	KRequirePackage("llvm", pline);
	KRequirePackage("konoha.assign", pline);
	KRequirePackage("konoha.null", pline);
	KRequirePackage("konoha.string", pline);
	kmodjit_t *base  = (kmodjit_t*)KCALLOC(sizeof(kmodjit_t), 1);
	base->h.name     = "ijit";
	base->h.setup    = kmodjit_setup;
	base->h.reftrace = kmodjit_reftrace;
	base->h.free     = kmodjit_free;
	base->defaultCodeGen = kctx->klib->kMethod_genCode;
	base->jitcache = KLIB Kmap_init(kctx, 0);
	KINITv(base->global_value, new_(Array, 18));
	KINITv(base->constPool, new_(Array, 0));

	typedef struct kPointer {
		KonohaObjectHeader h;
	} kPointer;
	static KDEFINE_CLASS PointerDef = {
		STRUCTNAME(Pointer)
	};
	base->cPointer = KLIB kNameSpace_defineClass(kctx, ns, NULL, &PointerDef, pline);

	//FIXME
	//KDEFINE_INT_CONST IntData[] = {
	//	{"PTRSIZE", TY_int, sizeof(void*)},
	//	{NULL},
	//};
	//KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);

	KLIB KonohaRuntime_setModule(kctx, MOD_jit, &base->h, pline);
	return true;
}

static kbool_t ijit_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	/* Array[Expr] */
	kparamtype_t P_ExprArray[] = {{TY_Expr}};
	int TY_ExprArray = (KLIB KonohaClass_Generics(kctx, CT_Array, TY_void, 1, P_ExprArray))->typeId;

	kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_System, MN_("genCode"), 0, MPOL_FIRST);
	KINITv(kmodjit->genCode, mtd);
#define TY_Pointer kmodjit->cPointer->typeId
#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)
#define TY_O  TY_Object
	int FN_x = FN_("x");
	int FN_y = FN_("y");
	int FN_z = FN_("z");
	int FN_w = FN_("w");

	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(System_setUline), TY_void, TY_System, MN_("setUline"), 1, TY_int, FN_x,
		_Public|_Static, _F(System_getUline), TY_int,  TY_System, MN_("getUline"), 0,
		_Public|_Static|_Coercion, _F(System_getValue), TY_Object, TY_System, MN_("getValue"), 1, TY_int, FN_x,
		_Public|_Static|_Coercion, _F(System_setValue), TY_void  , TY_System, MN_("setValue"), 2, TY_int, FN_x, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_clearValue), TY_void  , TY_System, MN_("clearValue"), 0,
		_Public|_Static|_Coercion, _F(System_getModule), TY_Object, TY_System, MN_("getModule"), 0,
		_Public|_Static|_Coercion, _F(System_setModule), TY_void  , TY_System, MN_("setModule"), 1, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getExecutionEngine), TY_Object, TY_System, MN_("getExecutionEngine"), 0,
		_Public|_Static|_Coercion, _F(System_setExecutionEngine), TY_void  , TY_System, MN_("setExecutionEngine"), 1, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getBuilder), TY_Object, TY_System, MN_("getBuilder"),0,
		_Public|_Static|_Coercion, _F(System_setBuilder), TY_void  , TY_System, MN_("setBuilder"),1, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getFunction), TY_Object, TY_System, MN_("getFunction"),0,
		_Public|_Static|_Coercion, _F(System_setFunction), TY_void  , TY_System, MN_("setFunction"),1, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getCTX), TY_Object, TY_System, MN_("getCTX"),0,
		_Public|_Static|_Coercion, _F(System_setCTX), TY_void  , TY_System, MN_("setCTX"),1, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getSFP), TY_Object, TY_System, MN_("getSFP"),0,
		_Public|_Static|_Coercion, _F(System_setSFP), TY_void  , TY_System, MN_("setSFP"),1, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getRetVal), TY_Object, TY_System, MN_("getRetVal"),0,
		_Public|_Static|_Coercion, _F(System_setRetVal), TY_void  , TY_System, MN_("setRetVal"),1, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_getJITCache), TY_Object, TY_System, MN_("getJITCache"), 1, TY_Method, FN_x,
		_Public|_Static|_Coercion, _F(System_setJITCache), TY_void  , TY_System, MN_("setJITCache"), 2, TY_Method, FN_x, TY_Object, FN_y,
		_Public|_Static|_Coercion, _F(System_setTyHObject),  TY_void, TY_System, MN_("setTyHObject"),  1, TY_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyObject),   TY_void, TY_System, MN_("setTyObject"),   1, TY_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyFunction), TY_void, TY_System, MN_("setTyFunction"), 1, TY_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyFunctionP),TY_void, TY_System, MN_("setTyFunctionP"),1, TY_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyMethod),   TY_void, TY_System, MN_("setTyMethod"),   1, TY_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyContext),  TY_void, TY_System, MN_("setTyContext"),  1, TY_O, FN_x,
		_Public|_Static|_Coercion, _F(System_setTyStack),    TY_void, TY_System, MN_("setTyStack"),    1, TY_O, FN_x,
		_Public|_Static|_Coercion, _F(System_getTyHObject),  TY_O,    TY_System, MN_("getTyHObject"), 0,
		_Public|_Static|_Coercion, _F(System_getTyObject),   TY_O,    TY_System, MN_("getTyObject"), 0,
		_Public|_Static|_Coercion, _F(System_getTyFunction), TY_O,    TY_System, MN_("getTyFunction"), 0,
		_Public|_Static|_Coercion, _F(System_getTyFunctionP),TY_O,    TY_System, MN_("getTyFunctionP"), 0,
		_Public|_Static|_Coercion, _F(System_getTyMethod),   TY_O,    TY_System, MN_("getTyMethod"), 0,
		_Public|_Static|_Coercion, _F(System_getTyContext),  TY_O,    TY_System, MN_("getTyContext"), 0,
		_Public|_Static|_Coercion, _F(System_getTyStack),    TY_O,    TY_System, MN_("getTyStack"), 0,
		_Public, _F(Block_getEspIndex), TY_int, TY_Block, MN_("getEspIndex"), 0,
		_Public, _F(Block_getBlocks), TY_Array, TY_Block, MN_("getBlocks"), 0,
		_Public, _F(Array_getSize), TY_int, TY_Array, MN_("getSize"), 0,
		_Public, _F(Stmt_getUline), TY_int, TY_Stmt,  MN_("getUline"), 0,
		_Public, _F(Stmt_hasSyntax), TY_boolean, TY_Stmt,  MN_("hasSyntax"), 0,
		_Public, _F(Stmt_getObjectNULL), TY_O, TY_Stmt, MN_("getObjectNULL"), 1, TY_int, FN_x,
		_Public, _F(System_addConstPool), TY_void, TY_System, MN_("addConstPool"), 1, TY_O, FN_x,
		_Public, _F(Object_unbox), TY_int, TY_O, MN_("unbox"), 0,
		_Public|_Static, _F(Array_new1), TY_Array, TY_Array, MN_("new1"), 1, TY_O, FN_x,
		_Public|_Static, _F(Array_new2), TY_Array, TY_Array, MN_("new2"), 2, TY_O, FN_x, TY_O, FN_y,
		_Public|_Static, _F(Array_new3), TY_Array, TY_Array, MN_("new3"), 3, TY_O, FN_x, TY_O, FN_y, TY_O, FN_z,
		_Public|_Static, _F(Array_newN), TY_Array, TY_Array, MN_("newN"), 1, TY_int, FN_x,
		_Public, _F(Array_getO), TY_Object, TY_Array, MN_("get"), 1, TY_int, FN_x,
		_Public, _F(Array_setO), TY_void,   TY_Array, MN_("set"), 2, TY_int, FN_x, TY_O, FN_y,
		_Public, _F(Array_erase), TY_Array, TY_Array, MN_("erase"), 1, TY_int, FN_x,
		_Public, _F(Method_getParamSize), TY_int, TY_Method, MN_("getParamSize"), 0,
		_Public, _F(Method_getParam), TY_Param, TY_Method, MN_("getParam"), 0,
		_Public, _F(Param_getType), TY_int, TY_Param, MN_("getType"), 1, TY_int, FN_x,
		_Public, _F(Method_getReturnType), TY_int, TY_Method, MN_("getReturnType"), 0,
		_Public, _F(Method_getCname), TY_String, TY_Method, MN_("getCname"), 0,
		_Public, _F(Method_getFname), TY_String, TY_Method, MN_("getFname"), 0,
		_Public|_Static, _F(System_getNULL), TY_Object, TY_System, MN_("getNULL"), 1, TY_int, FN_x,
		_Public, _F(Method_isStatic_), TY_boolean, TY_Method, MN_("isStatic"), 0,
		_Public, _F(Method_isVirtual_), TY_boolean, TY_Method, MN_("isVirtual"), 0,
		_Public|_Coercion, _F(Object_toStmt), TY_Stmt, TY_Object, MN_to(TY_Stmt), 0,
		_Public|_Coercion, _F(Object_toExpr), TY_Expr, TY_Object, MN_to(TY_Expr), 0,
		_Public, _F(Expr_getSingle), TY_Expr, TY_Expr, MN_("getSingle"), 0,
		_Public, _F(Expr_getCons), TY_ExprArray, TY_Expr, MN_("getCons"), 0,


		_Public|_Static, _F(Pointer_get), TY_int,  TY_System, MN_("getPointer"),3, TY_int, FN_x, TY_int, FN_y, TY_int, FN_z,
		_Public|_Static, _F(Pointer_set), TY_void, TY_System, MN_("setPointer"),4, TY_int, FN_x, TY_int, FN_y, TY_int, FN_z,TY_int, FN_w,
		_Public|_Coercion, _F(Object_getAddr), TY_int, TY_O, MN_("getAddr"), 0,
		_Public, _F(Object_getCid), TY_int, TY_O, MN_("getCid"), 0,
		_Public, _F(Method_getCid), TY_int, TY_Method, MN_("getCid"), 0,
		//_Public|_Coercion, _F(Object_toStmt), TY_Stmt, TY_Object, MN_to(TY_Stmt), 0,

#define TO(T) _Public|_Static, _F(Pointer_toObject), TY_##T, TY_System, MN_("convertTo" # T), 1, TY_int, FN_x
		TO(Array),
		TO(Object),
		TO(Method),
		TO(String),

		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	KonohaLibVar *l = (KonohaLibVar*)kctx->klib;
	l->kMethod_genCode = GenCodeDefault;
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
	l->kMethod_genCode = _kMethod_genCode;

	return true;
}

static kbool_t ijit_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t ijit_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* ijit_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("ijit", "1.0"),
		.initPackage    = ijit_initPackage,
		.setupPackage   = ijit_setupPackage,
		.initNameSpace  = ijit_initNameSpace,
		.setupNameSpace = ijit_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
