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

#define MOD_jit  40/*TODO*/
#define kjitmod ((kjitmod_t*)_ctx->mod[MOD_jit])
#define kmodjit ((kmodjit_t*)_ctx->modshare[MOD_jit])
#define GenCodeMtd (kmodjit)->genCode
#define GenCodeDefault (kmodjit)->defaultCodeGen

typedef void (*FgenCode)(CTX, kMethod *mtd, kBlock *bk);
typedef struct {
	kmodshare_t h;
	kMethod *genCode;
	kArray  *constPool;
	kArray  *global_value;
	FgenCode defaultCodeGen;
	kmap_t *jitcache;
	kline_t uline;
	kclass_t *cPointer;
} kmodjit_t;

typedef struct {
	kmodlocal_t h;
} kjitmod_t;

static void kmodjit_setup(CTX, struct kmodshare_t *def, int newctx)
{
	(void)_ctx;(void)def;(void)newctx;
}

extern struct _kObject **KONOHA_reftail(CTX, size_t size);

static void val_reftrace(CTX, kmape_t *p)
{
	BEGIN_REFTRACE(1);
	KREFTRACEv(p->ovalue);
	END_REFTRACE();
}

static void kmodjit_reftrace(CTX, struct kmodshare_t *baseh)
{
	kmodjit_t *mod = (kmodjit_t *) baseh;
	BEGIN_REFTRACE(3);
	KREFTRACEv(mod->genCode);
	KREFTRACEv(mod->global_value);
	KREFTRACEv(mod->constPool);
	END_REFTRACE();
	kmap_reftrace(mod->jitcache, val_reftrace);
}

static void kmodjit_free(CTX, struct kmodshare_t *baseh)
{
	kmodjit_t *modshare = (kmodjit_t*) baseh;
	kmap_free(modshare->jitcache, NULL);
	KFREE(baseh, sizeof(kmodjit_t));
}

static void check_stack_size(CTX, kArray *stack, int n)
{
	while (n >= kArray_size(stack)) {
		kArray_add(stack, K_NULL);
	}
}

static void set_value(CTX, int index, kObject *o)
{
	kArray  *g = kmodjit->global_value;
	KSETv(g->list[index], o);
}

static kObject *get_value(CTX, int index)
{
	kArray  *g = kmodjit->global_value;
	return g->list[index];
}

//## void System.setUline(int uline);
static KMETHOD System_setUline(CTX, ksfp_t *sfp _RIX)
{
	kmodjit->uline = sfp[1].ivalue;
	RETURNvoid_();
}
//## void System.getUline();
static KMETHOD System_getUline(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(kmodjit->uline);
}

static KMETHOD Expr_getSingle(CTX, ksfp_t *sfp _RIX)
{
	kExpr *expr = (kExpr *) sfp[0].o;
	DBG_ASSERT(IS_Expr(expr->single));
	RETURN_(expr->single);
}

static kArray *get_stack(CTX, kArray *g)
{
	if (!g->list[0]) {
		KSETv(g->list[0], ((kObject*)new_(Array, 0)));
	}
	return (kArray*)g->list[0];
}

//## Value System.getValue(int index);
static KMETHOD System_getValue(CTX, ksfp_t *sfp _RIX)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(_ctx, g);
	int index = sfp[1].ivalue;
	RETURN_(stack->list[index]);
}

//## void  System.setValue(int index, Value v);
static KMETHOD System_setValue(CTX, ksfp_t *sfp _RIX)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(_ctx, g);
	int index = sfp[1].ivalue;
	check_stack_size(_ctx, stack, index);
	kObject *o = sfp[2].o;
	KSETv(stack->list[index], o);
	RETURNvoid_();
}

//## void  System.clearValue();
static KMETHOD System_clearValue(CTX, ksfp_t *sfp _RIX)
{
	kArray *g = kmodjit->global_value;
	kArray *stack = get_stack(_ctx, g);
	kArray_clear(stack, 0);
}

//## Module System.getModule();
static KMETHOD System_getModule(CTX, ksfp_t *sfp _RIX)
{
	kArray  *g = kmodjit->global_value;
	kObject *o = g->list[1] ? g->list[1] : K_NULL;
	RETURN_(o);
}

//## void System.setModule(Module m);
static KMETHOD System_setModule(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 1, sfp[1].o);
	RETURNvoid_();
}

//## ExecutionEngine System.getExecutionEngine();
static KMETHOD System_getExecutionEngine(CTX, ksfp_t *sfp _RIX)
{
	kObject *o = get_value(_ctx, 2);
	RETURN_(o);
}

//## void System.setExecutionEngine(ExecutionEngine ee);
static KMETHOD System_setExecutionEngine(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 2, sfp[1].o);
	RETURNvoid_();
}

//## IRBuilder System.getBuilder();
static KMETHOD System_getBuilder(CTX, ksfp_t *sfp _RIX)
{
	kObject *o = get_value(_ctx, 3);
	RETURN_(o);
}

//## void System.setBuilder(IRBuilder b);
static KMETHOD System_setBuilder(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 3, sfp[1].o);
	RETURNvoid_();
}

//## Function System.getFunction();
static KMETHOD System_getFunction(CTX, ksfp_t *sfp _RIX)
{
	kObject *o = get_value(_ctx, 4);
	RETURN_(o);
}

//## void System.setFunction(Function f);
static KMETHOD System_setFunction(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 4, sfp[1].o);
	RETURNvoid_();
}

//## Value System.getCTX();
static KMETHOD System_getCTX(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 5));
}

//## void  System.setCTX(Value v);
static KMETHOD System_setCTX(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 5, sfp[1].o);
	RETURNvoid_();
}

//## Value System.getSFP();
static KMETHOD System_getSFP(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 6));
}

//## void  System.setSFP(Value v);
static KMETHOD System_setSFP(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 6, sfp[1].o);
	RETURNvoid_();
}

//## Value System.getRetVal();
static KMETHOD System_getRetVal(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 7));
}

//## void  System.setRetVal(Value v);
static KMETHOD System_setRetVal(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 7, sfp[1].o);
	RETURNvoid_();
}

//## void System.addConstPool(o);
static KMETHOD System_addConstPool(CTX, ksfp_t *sfp _RIX)
{
	kObject *o = sfp[1].o;
	kArray  *a = kmodjit->constPool;
	kArray_add(a, o);
	RETURNvoid_();
}

static uintptr_t jitcache_hash(kMethod *mtd)
{
	kcid_t cid = mtd->cid;
	kmethodn_t mn = mtd->mn;
	return (cid << sizeof(short)*8) | mn;
}

static kObject *jitcache_get(CTX, kMethod *mtd)
{
	uintptr_t hcode = jitcache_hash(mtd);
	kmap_t *map = kmodjit->jitcache;
	kmape_t *e = kmap_get(map, hcode);
	if (e) {
		return (kObject*) e->uvalue;
	} else {
		return K_NULL;
	}
}

static void jitcache_set(CTX, kMethod *mtd, kObject *f)
{
	uintptr_t hcode = jitcache_hash(mtd);
	kmap_t *map = kmodjit->jitcache;
	kmape_t *newe = kmap_newentry(map, hcode);
	newe->uvalue = (uintptr_t) f;
}

//## Function System.getJITCache(Method mtd);
static KMETHOD System_getJITCache(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[1].mtd;
	kObject *o   = jitcache_get(_ctx, mtd);
	RETURN_(o);
}

//## void     System.setJITCache(Method mtd, Function f);
static KMETHOD System_setJITCache(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[1].mtd;
	kObject *o   = sfp[2].o;
	jitcache_set(_ctx, mtd, o);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_HOBJECT(Type type);
static KMETHOD System_setTyHObject(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 8, sfp[1].o);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_OBJECT(Type type);
static KMETHOD System_setTyObject(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 9, sfp[1].o);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_FUNCTION(Type type);
static KMETHOD System_setTyFunction(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 10, sfp[1].o);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_FUNCTIONP(Type type);
static KMETHOD System_setTyFunctionP(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 11, sfp[1].o);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_METHOD(Type type);
static KMETHOD System_setTyMethod(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 12, sfp[1].o);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_CONTEXT(Type type);
static KMETHOD System_setTyContext(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 13, sfp[1].o);
	RETURNvoid_();
}

//## void System.setLLVMTYPE_STACK(Type type);
static KMETHOD System_setTyStack(CTX, ksfp_t *sfp _RIX)
{
	set_value(_ctx, 14, sfp[1].o);
	RETURNvoid_();
}

//## Type System.getLLVMTYPE_HOBJECT();
static KMETHOD System_getTyHObject(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 8));
}

//## Type System.getLLVMTYPE_OBJECT();
static KMETHOD System_getTyObject(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 9));
}

//## Type System.getLLVMTYPE_FUNCTION();
static KMETHOD System_getTyFunction(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 10));
}

//## Type System.getLLVMTYPE_FUNCTIONP();
static KMETHOD System_getTyFunctionP(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 11));
}

//## Type System.getLLVMTYPE_METHOD();
static KMETHOD System_getTyMethod(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 12));
}

//## Type System.getLLVMTYPE_CONTEXT();
static KMETHOD System_getTyContext(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 13));
}

//## Type System.getLLVMTYPE_STACK();
static KMETHOD System_getTyStack(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(get_value(_ctx, 14));
}


//## int bk.getEspIndex();
static KMETHOD Block_getEspIndex(CTX, ksfp_t *sfp _RIX)
{
	kBlock *bk = (kBlock *) sfp[0].o;
	RETURNi_(bk->esp->index);
}

//## Array Block.getBlocks();
static KMETHOD Block_getBlocks(CTX, ksfp_t *sfp _RIX)
{
	kBlock *bk = (kBlock *) sfp[0].o;
	RETURN_(bk->blocks);
}

//## int Array.getSize();
static KMETHOD Array_getSize(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(kArray_size(sfp[0].a));
}

//## int Stmt.getUline();
static KMETHOD Stmt_getUline(CTX, ksfp_t *sfp _RIX)
{
	kStmt *stmt = (kStmt*) sfp[0].o;
	RETURNi_(stmt->uline);
}
//## boolean Stmt.hasSyntax();
static KMETHOD Stmt_hasSyntax(CTX, ksfp_t *sfp _RIX)
{
	kStmt *stmt = (kStmt*) sfp[0].o;
	RETURNb_(stmt->syn != NULL);
}
//## Object Stmt.getObjectNULL(int id);
static KMETHOD Stmt_getObjectNULL(CTX, ksfp_t *sfp _RIX)
{
	kStmt *stmt = (kStmt*) sfp[0].o;
	kObject *o = kObject_getObjectNULL(stmt, sfp[1].ivalue);
	if (!o) {
		o = K_NULL;
	}
	RETURN_(o);
}

//## int Object.unbox()
static KMETHOD Object_unbox(CTX, ksfp_t *sfp _RIX)
{
	kObject *o = sfp[0].o;
	RETURNi_(O_unbox(o));
}


//FIXME TODO stupid down cast
static KMETHOD Object_toStmt(CTX, ksfp_t *sfp _RIX)
{
	(void)_ctx;
	RETURN_(sfp[0].o);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toExpr(CTX, ksfp_t *sfp _RIX)
{
	(void)_ctx;
	RETURN_(sfp[0].o);
}

kObject *boxing_jit(CTX, kcid_t cid, uintptr_t data)
{
	return new_kObject(CT_(cid), data);
}

// --------------------------------------------------------------------------
static kArray *create_array(CTX, ksfp_t *sfp, int n)
{
	int i;
	kArray *a = new_(Array, 0);
	for (i = 1; i <= n; ++i) {
		kArray_add(a, sfp[i].o);
	}
	return a;
}

// --------------------------------------------------------------------------
//## static Array Array.new1(Object o1, Object o2, Object o3);
static KMETHOD Array_new1(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(create_array(_ctx, sfp, 1));
}

// --------------------------------------------------------------------------
//## static Array Array.new2(Object o1, Object o2);
static KMETHOD Array_new2(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(create_array(_ctx, sfp, 2));
}

// --------------------------------------------------------------------------
//## static Array Array.new3(Object o1, Object o2, Object o3);
static KMETHOD Array_new3(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(create_array(_ctx, sfp, 3));
}

// --------------------------------------------------------------------------
//## static Array Array.newN(int n);
static KMETHOD Array_newN(CTX, ksfp_t *sfp _RIX)
{
	int i, n = sfp[1].ivalue;
	kArray *a = new_(Array, (uintptr_t)n);
	for (i = 0; i < n; ++i) {
		kArray_add(a, sfp[i].o);
	}
	RETURN_(a);
}

// --------------------------------------------------------------------------
//## Array Array.getO();
static KMETHOD Array_getO(CTX, ksfp_t *sfp _RIX)
{
	kArray *a = sfp[0].a;
	size_t n = check_index(_ctx, sfp[1].ivalue, kArray_size(a), sfp[K_RTNIDX].uline);
	RETURN_(a->list[n]);
}
// --------------------------------------------------------------------------
//## void Array.setO(int n, Object o);
static KMETHOD Array_setO(CTX, ksfp_t *sfp _RIX)
{
	kArray *a = sfp[0].a;
	size_t n = check_index(_ctx, sfp[1].ivalue, kArray_size(a), sfp[K_RTNIDX].uline);
	KSETv(a->list[n], sfp[2].o);
	RETURNvoid_();
}

// Array Array.erase(int n);
static KMETHOD Array_erase(CTX, ksfp_t *sfp _RIX)
{
	kArray *src = sfp[0].a;
	size_t n = check_index(_ctx, sfp[1].ivalue, kArray_size(src), sfp[K_RTNIDX].uline);
	size_t asize = kArray_size(src);
	size_t i, j = 0;
	kArray *dst = new_(Array, (asize-1));
	for (i = 0; i < asize; ++i) {
		if (i != n) {
			KSETv(dst->list[j], src->list[i]);
			++j;
		}
	}
	RETURN_(dst);
}

//## Int Method.getParamSize();
static KMETHOD Method_getParamSize(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[0].mtd;
	RETURNi_(kMethod_param(mtd)->psize);
}

//## Param Method.getParam(int n);
static KMETHOD Method_getParam(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[0].mtd;
	kParam *pa = kMethod_param(mtd);
	RETURN_(pa);
}

//## int Param.getType(int n);
static KMETHOD Param_getType(CTX, ksfp_t *sfp _RIX)
{
	kParam *pa = (kParam *) sfp[0].o;
	int n = sfp[1].ivalue;
	RETURNi_(pa->p[n].ty);
}

//## Int Method.getReturnType();
static KMETHOD Method_getReturnType(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[0].mtd;
	assert(IS_Method(mtd));
	RETURNi_(kMethod_param(mtd)->rtype);
}

//## String mtd.getFname();
static KMETHOD Method_getFname(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[0].mtd;
	kwb_t wb;
	kwb_init(&(_ctx->stack->cwb), &wb);
	kwb_printf(&wb, "%s%s", T_mn(mtd->mn));
	kString *fname = new_kString(kwb_top(&wb, 0), kwb_bytesize(&wb), SPOL_POOL);
	RETURN_(fname);
}

//## String mtd.getCname();
static KMETHOD Method_getCname(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[0].mtd;
	kcid_t cid = mtd->cid;
	const char *cname = TY_t(cid);
	RETURN_(new_kString(cname, strlen(cname), 0));
}

//## Object System.knull(int type);
static KMETHOD System_knull(CTX, ksfp_t *sfp _RIX)
{
	kcid_t cid = sfp[1].ivalue;
	kObject *o = knull(CT_(cid));
	RETURN_(o);
}

//## boolean Method.isStatic();
static KMETHOD Method_isStatic_(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[0].mtd;
	kbool_t b = kMethod_isStatic(mtd);
	RETURNb_(b);
}

//## boolean Method.isVirtual();
static KMETHOD Method_isVirtual_(CTX, ksfp_t *sfp _RIX)
{
	kMethod *mtd = sfp[0].mtd;
	kbool_t b = kMethod_isVirtual(mtd);
	RETURNb_(b);
}

/****************************************************************/
static uintptr_t *get_addr(void *addr, intptr_t offset, intptr_t datasize)
{
	return (uintptr_t*)(((char*)addr)+offset);
}
#define PTR_SIZE (sizeof(void*)*8)
//## int Pointer.get(int addr, int offset, int sizeof);
static KMETHOD Pointer_get(CTX, ksfp_t *sfp _RIX)
{
	intptr_t size   = sfp[3].ivalue;
	uintptr_t *p = get_addr((void*)sfp[1].ivalue, sfp[2].ivalue, size);
	assert(size <= PTR_SIZE);
	RETURNi_(*p & (~0UL >> (PTR_SIZE - size)));
}

//## void Pointer.set(int addr, int offset, int sizeof, int data);
static KMETHOD Pointer_set(CTX, ksfp_t *sfp _RIX)
{
	intptr_t size   = sfp[3].ivalue;
	uintptr_t data  = sfp[4].ivalue;
	uintptr_t *p = get_addr((void*)sfp[1].ivalue, sfp[2].ivalue, sfp[3].ivalue);
	uintptr_t mask = (~0UL >> (PTR_SIZE - size));
	*p = (data & mask) | (*p & ~mask);
	assert(size <= PTR_SIZE);
	RETURNvoid_();
}

//## int Object.getAddr();
static KMETHOD Object_getAddr(CTX, ksfp_t *sfp _RIX)
{
	uintptr_t p = (uintptr_t)sfp[0].o;
	RETURNi_(p);
}

//## var Pointer.toObject(int addr);
static KMETHOD Pointer_toObject(CTX, ksfp_t *sfp _RIX)
{
	uintptr_t addr = (uintptr_t) sfp[1].ivalue;
	kObject *o = (kObject*)addr;
	RETURN_(o);
}

////FIXME TODO stupid down cast
//static KMETHOD Object_toExpr(CTX, ksfp_t *sfp _RIX)
//{
//	(void)_ctx;
//	RETURN_(sfp[0].o);
//}

/****************************************************************/
static void KMethod_genCode(CTX, kMethod *mtd, kBlock *bk)
{
	DBG_P("START CODE GENERATION..");
	BEGIN_LOCAL(lsfp, 8);

	KSETv(lsfp[K_CALLDELTA+1].mtd, mtd);
	KSETv(lsfp[K_CALLDELTA+2].o, (kObject*)bk);
	KCALL(lsfp, 0, GenCodeMtd, 2, K_NULL);
	END_LOCAL();
}

static kbool_t ijit_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	KREQUIRE_PACKAGE("sugar", pline);
	KREQUIRE_PACKAGE("konoha.float", pline);
	KREQUIRE_PACKAGE("llvm", pline);
	KREQUIRE_PACKAGE("konoha.assignment", pline);
	KREQUIRE_PACKAGE("konoha.null", pline);
	KREQUIRE_PACKAGE("konoha.string", pline);
	kmodjit_t *base  = (kmodjit_t*)KCALLOC(sizeof(kmodjit_t), 1);
	base->h.name     = "ijit";
	base->h.setup    = kmodjit_setup;
	base->h.reftrace = kmodjit_reftrace;
	base->h.free     = kmodjit_free;
	base->defaultCodeGen = _ctx->lib2->KMethod_genCode;
	base->jitcache = kmap_init(0);
	KINITv(base->global_value, new_(Array, 18));
	KINITv(base->constPool, new_(Array, 0));

	typedef struct kPointer {
		kObjectHeader h;
	} kPointer;
	static KDEFINE_CLASS PointerDef = {
		STRUCTNAME(Pointer)
	};
	base->cPointer = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &PointerDef, pline);

	//FIXME
	//KDEFINE_INT_CONST IntData[] = {
	//	{"PTRSIZE", TY_Int, sizeof(void*)},
	//	{NULL},
	//};
	//kNameSpace_loadConstData(ks, IntData, pline);

	Konoha_setModule(MOD_jit, &base->h, pline);
	return true;
}

static kbool_t ijit_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	USING_SUGAR;

	kMethod *mtd = kNameSpace_getMethodNULL(ks, TY_System, MN_("genCode"));
	KINITv(kmodjit->genCode, mtd);
#define TY_Pointer kmodjit->cPointer->cid
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
		_Public|_Static, _F(System_setUline), TY_void, TY_System, MN_("setUline"), 1, TY_Int, FN_x,
		_Public|_Static, _F(System_getUline), TY_Int,  TY_System, MN_("getUline"), 0,
		_Public|_Static|_Coercion, _F(System_getValue), TY_Object, TY_System, MN_("getValue"), 1, TY_Int, FN_x,
		_Public|_Static|_Coercion, _F(System_setValue), TY_void  , TY_System, MN_("setValue"), 2, TY_Int, FN_x, TY_Object, FN_y,
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
		_Public, _F(Block_getEspIndex), TY_Int, TY_Block, MN_("getEspIndex"), 0,
		_Public, _F(Block_getBlocks), TY_Array, TY_Block, MN_("getBlocks"), 0,
		_Public, _F(Array_getSize), TY_Int, TY_Array, MN_("getSize"), 0,
		_Public, _F(Stmt_getUline), TY_Int, TY_Stmt,  MN_("getUline"), 0,
		_Public, _F(Stmt_hasSyntax), TY_Boolean, TY_Stmt,  MN_("hasSyntax"), 0,
		_Public, _F(Stmt_getObjectNULL), TY_O, TY_Stmt, MN_("getObjectNULL"), 1, TY_Int, FN_x,
		_Public, _F(System_addConstPool), TY_void, TY_System, MN_("addConstPool"), 1, TY_O, FN_x,
		_Public, _F(Object_unbox), TY_Int, TY_O, MN_("unbox"), 0,
		_Public|_Static, _F(Array_new1), TY_Array, TY_Array, MN_("new1"), 1, TY_O, FN_x,
		_Public|_Static, _F(Array_new2), TY_Array, TY_Array, MN_("new2"), 2, TY_O, FN_x, TY_O, FN_y,
		_Public|_Static, _F(Array_new3), TY_Array, TY_Array, MN_("new3"), 3, TY_O, FN_x, TY_O, FN_y, TY_O, FN_z,
		_Public|_Static, _F(Array_newN), TY_Array, TY_Array, MN_("newN"), 1, TY_Int, FN_x,
		_Public, _F(Array_getO), TY_Object, TY_Array, MN_("get"), 1, TY_Int, FN_x,
		_Public, _F(Array_setO), TY_void,   TY_Array, MN_("set"), 2, TY_Int, FN_x, TY_O, FN_y,
		_Public, _F(Array_erase), TY_Array, TY_Array, MN_("erase"), 1, TY_Int, FN_x,
		_Public, _F(Method_getParamSize), TY_Int, TY_Method, MN_("getParamSize"), 0,
		_Public, _F(Method_getParam), TY_Param, TY_Method, MN_("getParam"), 0,
		_Public, _F(Param_getType), TY_Int, TY_Param, MN_("getType"), 1, TY_Int, FN_x,
		_Public, _F(Method_getReturnType), TY_Int, TY_Method, MN_("getReturnType"), 0,
		_Public, _F(Method_getCname), TY_String, TY_Method, MN_("getCname"), 0,
		_Public, _F(Method_getFname), TY_String, TY_Method, MN_("getFname"), 0,
		_Public|_Static, _F(System_knull), TY_Object, TY_System, MN_("knull"), 1, TY_Int, FN_x,
		_Public, _F(Method_isStatic_), TY_Boolean, TY_Method, MN_("isStatic"), 0,
		_Public, _F(Method_isVirtual_), TY_Boolean, TY_Method, MN_("isVirtual"), 0,
		_Public|_Coercion, _F(Object_toStmt), TY_Stmt, TY_Object, MN_to(TY_Stmt), 0,
		_Public|_Coercion, _F(Object_toExpr), TY_Expr, TY_Object, MN_to(TY_Expr), 0,
		_Public, _F(Expr_getSingle), TY_Expr, TY_Expr, MN_("getSingle"), 0,


		_Public|_Static, _F(Pointer_get), TY_Int,  TY_System, MN_("getPointer"),3, TY_Int, FN_x, TY_Int, FN_y, TY_Int, FN_z,
		_Public|_Static, _F(Pointer_set), TY_void, TY_System, MN_("setPointer"),4, TY_Int, FN_x, TY_Int, FN_y, TY_Int, FN_z,TY_Int, FN_w,
		_Public|_Coercion, _F(Object_getAddr), TY_Int, TY_O, MN_("getAddr"), 0,
		//_Public|_Coercion, _F(Object_toStmt), TY_Stmt, TY_Object, MN_to(TY_Stmt), 0,

#define TO(T) _Public|_Static, _F(Pointer_toObject), TY_##T, TY_System, MN_("convertTo" # T), 1, TY_Int, FN_x
		TO(Array),
		TO(Object),
		TO(Method),
		TO(String),

		DEND,
	};
	kNameSpace_loadMethodData(ks, MethodData);

	struct _klib2 *l = (struct _klib2*)_ctx->lib2;
	l->KMethod_genCode = GenCodeDefault;
	kNameSpace_syncMethods();
	l->KMethod_genCode = KMethod_genCode;
	//KSET_KLIB(Method_genCode, pline);

	return true;
}

static kbool_t ijit_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t ijit_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* ijit_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("ijit", "1.0"),
		.initPackage = ijit_initPackage,
		.setupPackage = ijit_setupPackage,
		.initNameSpace = ijit_initNameSpace,
		.setupNameSpace = ijit_setupNameSpace,
	};
	return &d;
}
