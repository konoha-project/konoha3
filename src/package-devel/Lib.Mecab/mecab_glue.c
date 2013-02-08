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

#include <mecab.h>
#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _kTagger {
	kObjectHeader h;
	mecab_t *mecab;
} kTagger;

struct _kDictionary {
	kObjectHeader h;
	mecab_dictionary_info_t *d;
} kDictionary;

struct _kMecabNode {
	kObjectHeader h;
	const mecab_node_t *node;
} kMecabNode;

/* ------------------------------------------------------------------------ */

static void Tagger_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kTagger *mecab = (struct _kTagger *)o;
	const char* dummy = ""; // dummy
	mecab->mecab = mecab_new2(dummy);
	DBG_ASSERT(mecab->mecab != NULL);
}

static void Tagger_Free(KonohaContext *kctx, kObject *o)
{
	struct _kTagger *mecab = (struct _kTagger *)o;
	mecab->mecab = (mecab_t *)((uintptr_t)mecab->mecab & ~(0xf)); // why is it need?
	mecab_destroy(mecab->mecab);
}

#define KType_Tagger     cTagger->typeId
#define KType_MecabNode  cMecabNode->typeId
#define KDefineConstInt(T)  #T, KType_Int, T

/* ------------------------------------------------------------------------ */
/* [API methods] */
/* Tagger class  */

// Tagger Tagger.new();
static KMETHOD Tagger_new (KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[K_RTNIDX].asObject);
}

// String Tagger.parse(String input)
static KMETHOD Tagger_Parse(KonohaContext *kctx, KonohaStack *sfp)
{
	mecab_t * mecab = ((struct _kTagger *)(sfp[0].asObject))->mecab;
	const char *input = kString_text(sfp[1].asString);
	const char* result = mecab_sparse_tostr(mecab, input);
	KReturn(KLIB new_kString(kctx, GcUnsafe, result, strlen(result), 0));
}

// String Tagger.NBestParse(int n, String input)
static KMETHOD Tagger_NBestParse(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger *)sfp[0].asObject;
	kint_t ival = sfp[1].intValue;
	const char *input = kString_text(sfp[2].asString);
	const char* result = mecab_nbest_sparse_tostr(mecab->mecab, ival, input);
	KReturn(KLIB new_kString(kctx, GcUnsafe, result, strlen(result), 0));
}

// Boolean Tagger.NBestInit(String input)
static KMETHOD Tagger_NBestInit(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger *)sfp[0].asObject;
	const char *input = kString_text(sfp[1].asString);
	KReturnUnboxValue(mecab_nbest_init(mecab->mecab, input));
}

// String Tagger.NBestNext()
static KMETHOD Tagger_NBestNext(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger *)sfp[0].asObject;
	const char* next = mecab_nbest_next_tostr(mecab->mecab);
	KReturn(KLIB new_kString(kctx, GcUnsafe, next, strlen(next), 0));
}

// MecabNode Tagger.ParseToNode(String input)
static KMETHOD Tagger_ParseToNode(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger *)sfp[0].asObject;
	const char *input = kString_text(sfp[1].asString);
	const mecab_node_t* node = mecab_sparse_tonode(mecab->mecab, input);
	struct _kMecabNode* ret = (struct _kMecabNode *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	ret->node = node;
	KReturn(ret);
}

// void Tagger.destory()
static KMETHOD Tagger_destroy(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger *)sfp[0].asObject;
	mecab_destroy(mecab->mecab);
}

/* ------------------------------------------------------------------------ */
/* MecabNode class */

static void MecabNode_Init(KonohaContext *kctx, kObject *o, void *conf)
{
}

static void MecabNode_Free(KonohaContext *kctx, kObject *o)
{
}

// MecabNode MecabNode.next()
static KMETHOD MecabNode_next(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	mecab_node_t* next = node->node->next;
	struct _kMecabNode* ret;
	if(next != NULL) {
		ret = (struct _kMecabNode *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
		ret->node = next;
		KReturn(ret);
	}
	else {
		KReturnDefaultValue();
	}
}

// MecabNode MecabNode.prev()
static KMETHOD MecabNode_prev(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	mecab_node_t* prev = node->node->prev;
	struct _kMecabNode* ret;
	if(node != NULL) {
		ret = (struct _kMecabNode *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
		ret->node = prev;
		KReturn(ret);
	}
	else {
		KReturnDefaultValue();
	}
}

// MecabNode MecabNode.enext()
static KMETHOD MecabNode_enext(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	mecab_node_t* enext = node->node->enext;
	struct _kMecabNode* ret;
	if(node != NULL) {
		ret = (struct _kMecabNode *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
		ret->node = enext;
		KReturn(ret);
	}
	else {
		KReturnDefaultValue();
	}
}

// MecabNode MecabNode.bnext()
static KMETHOD MecabNode_bnext(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	mecab_node_t* bnext = node->node->bnext;
	struct _kMecabNode* ret = NULL;
	if(node != NULL) {
		ret = (struct _kMecabNode *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
		ret->node = bnext;
	}
	KReturn(ret);
}

// String MecabNode.getSurface()
static KMETHOD MecabNode_getSurface(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	const char* ret = node->node->surface;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

// String MecabNode.getFeature()
static KMETHOD MecabNode_getFeature(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	const char* ret = node->node->feature;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

// int MecabNode.getLength()
static KMETHOD MecabNode_getLength(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	unsigned int ret = node->node->length;
	KReturnUnboxValue(ret);
}

// int MecabNode.getRLength()
static KMETHOD MecabNode_getRLength(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	unsigned int ret = node->node->rlength;
	KReturnUnboxValue(ret);
}

// int MecabNode.getRCAttr()
static KMETHOD MecabNode_getRCAttr(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	short ret = node->node->rcAttr;
	KReturnUnboxValue(ret);
}

// int MecabNode.getLCAttr()
static KMETHOD MecabNode_getLCAttr(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	short ret = node->node->lcAttr;
	KReturnUnboxValue(ret);
}

// int MecabNode.getCharType()
static KMETHOD MecabNode_getCharType(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	unsigned int ret = node->node->char_type;
	KReturnUnboxValue(ret);
}

// int MecabNode.getStat()
static KMETHOD MecabNode_getStat(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	unsigned int ret = node->node->stat;
	KReturnUnboxValue(ret);
}

// int MecabNode.getID()
static KMETHOD MecabNode_getID(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	unsigned int ret = node->node->id;
	KReturnUnboxValue(ret);
}

// Boolean MecabNode.isBest()
static KMETHOD MecabNode_isBest(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	unsigned int ret = node->node->isbest;
	KReturnUnboxValue(ret);
}

//// float MecabNode.alpha()
//static KMETHOD MecabNode_alpha(KonohaContext *kctx, KonohaStack *sfp)
//{
//	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
//	float ret = node->node->alpha;
//	KReturnFloatValue(ret);
//}
//
//// float MecabNode.beta()
//static KMETHOD MecabNode_beta(KonohaContext *kctx, KonohaStack *sfp)
//{
//	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
//	float ret = node->node->beta;
//	KReturnFloatValue(ret);
//}
//
//// float MecabNode.prob()
//static KMETHOD MecabNode_prob(KonohaContext *kctx, KonohaStack *sfp)
//{
//	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
//	float ret = node->node->prob;
//	KReturnFloatValue(ret);
//}

// int MecabNode.wcost()
static KMETHOD MecabNode_wcost(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	short ret = node->node->wcost;
	KReturnUnboxValue(ret);
}

// int MecabNode.cost()
static KMETHOD MecabNode_cost(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode *)sfp[0].asObject;
	long ret = node->node->cost;
	KReturnUnboxValue(ret);
}

/* ------------------------------------------------------------------------ */

static kbool_t mecab_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	static KDEFINE_CLASS TaggerDef = {
		STRUCTNAME(Tagger),
		.cflag = KClassFlag_Final,
		.init = Tagger_Init,
		.free = Tagger_Free,
	};

	static KDEFINE_CLASS MecabNodeDef = {
		STRUCTNAME(MecabNode),
		.cflag = KClassFlag_Final,
		.init = MecabNode_Init,
		.free = MecabNode_Free,
	};

	KClass *cTagger = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &TaggerDef, trace);
	KClass *cMecabNode = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MecabNodeDef, trace);

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const, _F(Tagger_new),         KType_Tagger,  KType_Tagger, KMethodName_("new"),   0,
		_Public|_Const, _F(Tagger_Parse),       KType_String,  KType_Tagger, KMethodName_("parse"), 1, KType_String, KFieldName_("input"),
		_Public|_Const, _F(Tagger_NBestParse),  KType_String,  KType_Tagger, KMethodName_("NBestParse"), 2, KType_Int, KFieldName_("n"), KType_String, KFieldName_("input"),
		_Public|_Const, _F(Tagger_NBestInit),   KType_String,  KType_Tagger, KMethodName_("NBestInit"), 1, KType_String, KFieldName_("input"),
		_Public|_Const, _F(Tagger_NBestNext),   KType_String,  KType_Tagger, KMethodName_("NBestNext"), 0,
		_Public|_Const, _F(Tagger_ParseToNode), KType_MecabNode,  KType_Tagger, KMethodName_("parseToNode"), 1, KType_String, KFieldName_("input"),
		_Public|_Const, _F(Tagger_destroy),     KType_void,    KType_Tagger, KMethodName_("destroy"), 0,

		_Public|_Const, _F(MecabNode_next),        KType_MecabNode,  KType_MecabNode, KMethodName_("next"), 0,
		_Public|_Const, _F(MecabNode_prev),        KType_MecabNode,  KType_MecabNode, KMethodName_("prev"), 0,
		_Public|_Const, _F(MecabNode_enext),       KType_MecabNode,  KType_MecabNode, KMethodName_("enext"), 0,
		_Public|_Const, _F(MecabNode_bnext),       KType_MecabNode,  KType_MecabNode, KMethodName_("bnext"), 0,
		_Public|_Const, _F(MecabNode_getSurface),  KType_String,  KType_MecabNode, KMethodName_("getSurface"), 0,
		_Public|_Const, _F(MecabNode_getFeature),  KType_String,  KType_MecabNode, KMethodName_("getFeature"), 0,
		_Public|_Const, _F(MecabNode_getLength),   KType_Int,  KType_MecabNode, KMethodName_("getLength"), 0,
		_Public|_Const, _F(MecabNode_getRLength),  KType_Int,  KType_MecabNode, KMethodName_("getRLength"), 0,
		_Public|_Const, _F(MecabNode_getRCAttr),   KType_Int,  KType_MecabNode, KMethodName_("getRCAttr"), 0,
		_Public|_Const, _F(MecabNode_getLCAttr),   KType_Int,  KType_MecabNode, KMethodName_("getLCAttr"), 0,
		_Public|_Const, _F(MecabNode_getCharType), KType_Int,  KType_MecabNode, KMethodName_("getCharType"), 0,
		_Public|_Const, _F(MecabNode_getStat),     KType_Int,  KType_MecabNode, KMethodName_("getStat"), 0,
		_Public|_Const, _F(MecabNode_getID),       KType_Int,  KType_MecabNode, KMethodName_("getID"), 0,
		_Public|_Const, _F(MecabNode_isBest),      KType_Boolean,  KType_MecabNode, KMethodName_("isBest"), 0,
		//_Public|_Const, _F(MecabNode_alpha),       KType_Float,  KType_MecabNode, KMethodName_("alpha"), 0,
		//_Public|_Const, _F(MecabNode_beta),        KType_Float,  KType_MecabNode, KMethodName_("beta"), 0,
		//_Public|_Const, _F(MecabNode_prob),        KType_Float,  KType_MecabNode, KMethodName_("prob"), 0,
		_Public|_Const, _F(MecabNode_wcost),       KType_Int,  KType_MecabNode, KMethodName_("wcost"), 0,
		_Public|_Const, _F(MecabNode_cost),        KType_Int,  KType_MecabNode, KMethodName_("cost"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	KDEFINE_INT_CONST IntData[] = {
			{KDefineConstInt(MECAB_NOR_NODE)},
			{KDefineConstInt(MECAB_UNK_NODE)},
			{KDefineConstInt(MECAB_BOS_NODE)},
			{KDefineConstInt(MECAB_EOS_NODE)},
			{}
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);
	return true;
}

static kbool_t mecab_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

/* ======================================================================== */

KDEFINE_PACKAGE *Mecab_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("mecab", "1.0"),
		.PackupNameSpace    = mecab_PackupNameSpace,
		.ExportNameSpace   = mecab_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
