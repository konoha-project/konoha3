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

#ifndef MECAB_GLUE_H_
#define MECAB_GLUE_H_

#include <stdbool.h>
#include <stdio.h>
#include <mecab.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct KonohaContextVar *ctx;

struct _kTagger {
	KonohaObjectHeader h;
	mecab_t *mecab;
} kTagger;

struct _kDictionary {
	KonohaObjectHeader h;
	mecab_dictionary_info_t *d;
} kDictionary;

struct _kMecabNode {
	KonohaObjectHeader h;
	const mecab_node_t *node;
} kMecabNode;

/* ------------------------------------------------------------------------ */

static void Tagger_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kTagger *mecab = (struct _kTagger *)o;
	int argc = 1;          // dummy
	char* dummy = "dummy"; // dummy
	char** argv = &dummy;  // dummy
	mecab->mecab = mecab_new(argc, argv);
	if (!mecab->mecab) {
		fprintf (stderr, "Exception:%s\n", mecab_strerror (mecab->mecab)); \
	}
}

static void Tagger_free(KonohaContext *kctx, kObject *o)
{
	struct _kTagger *mecab = (struct _kTagger *)o;
	mecab->mecab = (mecab_t*)((uintptr_t)mecab->mecab & ~(0xf)); // why is it need?
	mecab_destroy(mecab->mecab);
}

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define CT_Tagger     cTagger
#define TY_Tagger     cTagger->classId
#define IS_Tagger(O)  ((O)->h.ct == CT_Tagger)

#define CT_MecabNode     cMecabNode
#define TY_MecabNode     cMecabNode->classId
#define IS_MecabNode(O)  ((O)->h.ct == CT_MecabNode)

#define _KVi(T)  #T, TY_Int, T

/* ------------------------------------------------------------------------ */
/* [API methods] */
/* Tagger class  */

#define CHECK(mecab, eval) if (! eval) { \
		fprintf (stderr, "Exception:%s\n", mecab_strerror (mecab)); \
		mecab_destroy(mecab); \
	}

//#define getMecab(O) ((mecab_t *)((kTagger*)(O)->mecab))

// Tagger Tagger.new();
static KMETHOD Tagger_new (KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0));
}

// String Tagger.parse(String input)
static KMETHOD Tagger_parse(KonohaContext *kctx, KonohaStack *sfp)
{
	mecab_t * mecab = ((struct _kTagger*)(sfp[0].o))->mecab;
	const char *input = S_text(sfp[1].s);
	const char* result = mecab_sparse_tostr(mecab, input);
	//fprintf(stderr, "result='\n%s' @ parse\n", result);
	RETURN_(KLIB new_kString(kctx, result, strlen(result), 0));
}

// String Tagger.NBestParse(int n, String input)
static KMETHOD Tagger_NBestParse(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger*)sfp[0].o;
	kint_t ival = sfp[1].intValue;
	const char *input = S_text(sfp[2].s);
	const char* result = mecab_nbest_sparse_tostr(mecab->mecab, ival, input);
	//fprintf(stderr, "result='%s' @ NBestParse\n", result);
	RETURN_(KLIB new_kString(kctx, result, strlen(result), 0));
}

// Boolean Tagger.NBestInit(String input)
static KMETHOD Tagger_NBestInit(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger*)sfp[0].o;
	const char *input = S_text(sfp[1].s);
	RETURNb_(mecab_nbest_init(mecab->mecab, input));
}

// String Tagger.NBestNext()
static KMETHOD Tagger_NBestNext(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger*)sfp[0].o;
	const char* next = mecab_nbest_next_tostr(mecab->mecab);
	//fprintf(stderr, "next='%s' @ NBestNext\n", next);
	RETURN_(KLIB new_kString(kctx, next, strlen(next), 0));
}

// MecabNode Tagger.ParseToNode(String input)
static KMETHOD Tagger_parseToNode(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger*)sfp[0].o;
	const char *input = S_text(sfp[1].s);
	const mecab_node_t* node = mecab_sparse_tonode(mecab->mecab, input);
	struct _kMecabNode* ret = (struct _kMecabNode*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	ret->node = node;
	RETURN_(ret);
}

// void Tagger.destory()
static KMETHOD Tagger_destroy(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kTagger *mecab = (struct _kTagger*)sfp[0].o;
	mecab_destroy(mecab->mecab);
}

/* ------------------------------------------------------------------------ */
/* MecabNode class */

static void MecabNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
}

static void MecabNode_free(KonohaContext *kctx, kObject *o)
{
}

// MecabNode MecabNode.next()
static KMETHOD MecabNode_next(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	mecab_node_t* next = node->node->next;
	struct _kMecabNode* ret;
	if (next != NULL) {
		ret = (struct _kMecabNode*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
		ret->node = next;
	}
	else {
		ret = NULL;
	}
	RETURN_(ret);
}

// MecabNode MecabNode.prev()
static KMETHOD MecabNode_prev(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	mecab_node_t* prev = node->node->prev;
	struct _kMecabNode* ret;
	if (node != NULL) {
		ret = (struct _kMecabNode*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
		ret->node = prev;
	}
	else {
		ret = NULL;
	}
	RETURN_(ret);
}

// MecabNode MecabNode.enext()
static KMETHOD MecabNode_enext(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	mecab_node_t* enext = node->node->enext;
	struct _kMecabNode* ret;
	if (node != NULL) {
		ret = (struct _kMecabNode*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
		ret->node = enext;
	}
	else {
		ret = NULL;
	}
	RETURN_(ret);
}

// MecabNode MecabNode.bnext()
static KMETHOD MecabNode_bnext(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	mecab_node_t* bnext = node->node->bnext;
	struct _kMecabNode* ret;
	if (node != NULL) {
		ret = (struct _kMecabNode*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
		ret->node = bnext;
	}
	else {
		ret = NULL;
	}
	RETURN_(ret);
}

// String MecabNode.getSurface()
static KMETHOD MecabNode_getSurface(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	const char* ret = node->node->surface;
	RETURN_(KLIB new_kString(kctx, ret, strlen(ret), 0));
}

// String MecabNode.getFeature()
static KMETHOD MecabNode_getFeature(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	const char* ret = node->node->feature;
	RETURN_(KLIB new_kString(kctx, ret, strlen(ret), 0));
}

// int MecabNode.getLength()
static KMETHOD MecabNode_getLength(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	unsigned int ret = node->node->length;
	RETURNi_(ret);
}

// int MecabNode.getRLength()
static KMETHOD MecabNode_getRLength(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	unsigned int ret = node->node->rlength;
	RETURNi_(ret);
}

// int MecabNode.getID()
static KMETHOD MecabNode_getID(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	unsigned int ret = node->node->id;
	RETURNi_(ret);
}

// int MecabNode.getRCAttr()
static KMETHOD MecabNode_getRCAttr(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	short ret = node->node->rcAttr;
	RETURNi_(ret);
}

// int MecabNode.getLCAttr()
static KMETHOD MecabNode_getLCAttr(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	short ret = node->node->lcAttr;
	RETURNi_(ret);
}

// int MecabNode.getCharType()
static KMETHOD MecabNode_getCharType(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	unsigned int ret = node->node->char_type;
	RETURNi_(ret);
}

// int MecabNode.getStat()
static KMETHOD MecabNode_getStat(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	unsigned int ret = node->node->stat;
	RETURNi_(ret);
}

// Boolean MecabNode.isBest()
static KMETHOD MecabNode_isBest(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	unsigned int ret = node->node->isbest;
	RETURNb_(ret);
}

//// float MecabNode.alpha()
//static KMETHOD MecabNode_alpha(KonohaContext *kctx, KonohaStack *sfp)
//{
//	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
//	float ret = node->node->alpha;
//	RETURNf_(ret);
//}
//
//// float MecabNode.beta()
//static KMETHOD MecabNode_beta(KonohaContext *kctx, KonohaStack *sfp)
//{
//	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
//	float ret = node->node->beta;
//	RETURNf_(ret);
//}
//
//// float MecabNode.prob()
//static KMETHOD MecabNode_prob(KonohaContext *kctx, KonohaStack *sfp)
//{
//	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
//	float ret = node->node->prob;
//	RETURNf_(ret);
//}

// int MecabNode.wcost()
static KMETHOD MecabNode_wcost(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	short ret = node->node->wcost;
	RETURNi_(ret);
}

// int MecabNode.cost()
static KMETHOD MecabNode_cost(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kMecabNode *node = (struct _kMecabNode*)sfp[0].o;
	long ret = node->node->cost;
	RETURNi_(ret);
}

/* ------------------------------------------------------------------------ */

static kbool_t mecab_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	fprintf(stderr, "hoge\n");
	ctx = (struct KonohaContextVar *)kctx;

	static KDEFINE_CLASS defTagger = {
		STRUCTNAME(Tagger),
		.cflag = kClass_Final,
		.init = Tagger_init,
		.free = Tagger_free,
	};

	static KDEFINE_CLASS defMecabNode = {
		STRUCTNAME(MecabNode),
		.cflag = kClass_Final,
		.init = MecabNode_init,
		.free = MecabNode_free,
	};

	KonohaClass *cTagger = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defTagger, pline);
	KonohaClass *cMecabNode = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defMecabNode, pline);

	intptr_t MethodData[] = {
		_Public|_Const, _F(Tagger_new),         TY_Tagger,  TY_Tagger, MN_("new"),   0,
		_Public|_Const, _F(Tagger_parse),       TY_String,  TY_Tagger, MN_("parse"), 1, TY_String, FN_("input"),
		_Public|_Const, _F(Tagger_NBestParse),  TY_String,  TY_Tagger, MN_("NBestParse"), 2, TY_Int, FN_("n"), TY_String, FN_("input"),
		_Public|_Const, _F(Tagger_NBestInit),   TY_String,  TY_Tagger, MN_("NBestInit"), 1, TY_String, FN_("input"),
		_Public|_Const, _F(Tagger_NBestNext),   TY_String,  TY_Tagger, MN_("NBestNext"), 0,
		_Public|_Const, _F(Tagger_parseToNode), TY_MecabNode,  TY_Tagger, MN_("parseToNode"), 1, TY_String, FN_("input"),
		_Public|_Const, _F(Tagger_destroy),     TY_void,    TY_Tagger, MN_("destroy"), 0,

		_Public|_Const, _F(MecabNode_next),        TY_MecabNode,  TY_MecabNode, MN_("next"), 0,
		_Public|_Const, _F(MecabNode_prev),        TY_MecabNode,  TY_MecabNode, MN_("prev"), 0,
		_Public|_Const, _F(MecabNode_enext),       TY_MecabNode,  TY_MecabNode, MN_("enext"), 0,
		_Public|_Const, _F(MecabNode_bnext),       TY_MecabNode,  TY_MecabNode, MN_("bnext"), 0,
		_Public|_Const, _F(MecabNode_getSurface),  TY_String,  TY_MecabNode, MN_("getSurface"), 0,
		_Public|_Const, _F(MecabNode_getFeature),  TY_String,  TY_MecabNode, MN_("getFeature"), 0,
		_Public|_Const, _F(MecabNode_getLength),   TY_Int,  TY_MecabNode, MN_("getLength"), 0,
		_Public|_Const, _F(MecabNode_getRLength),  TY_Int,  TY_MecabNode, MN_("getRLength"), 0,
		_Public|_Const, _F(MecabNode_getID),       TY_Int,  TY_MecabNode, MN_("getID"), 0,
		_Public|_Const, _F(MecabNode_getRCAttr),   TY_Int,  TY_MecabNode, MN_("getRCAttr"), 0,
		_Public|_Const, _F(MecabNode_getLCAttr),   TY_Int,  TY_MecabNode, MN_("getLCAttr"), 0,
		_Public|_Const, _F(MecabNode_getCharType), TY_Int,  TY_MecabNode, MN_("getCharType"), 0,
		_Public|_Const, _F(MecabNode_getStat),     TY_Int,  TY_MecabNode, MN_("getStat"), 0,
		_Public|_Const, _F(MecabNode_isBest),      TY_Boolean,  TY_MecabNode, MN_("isBest"), 0,
		//_Public|_Const, _F(MecabNode_alpha),       TY_Float,  TY_MecabNode, MN_("alpha"), 0,
		//_Public|_Const, _F(MecabNode_beta),        TY_Float,  TY_MecabNode, MN_("beta"), 0,
		//_Public|_Const, _F(MecabNode_prob),        TY_Float,  TY_MecabNode, MN_("prob"), 0,
		_Public|_Const, _F(MecabNode_wcost),       TY_Int,  TY_MecabNode, MN_("wcost"), 0,
		_Public|_Const, _F(MecabNode_cost),        TY_Int,  TY_MecabNode, MN_("cost"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	KDEFINE_INT_CONST IntData[] = {
			{_KVi(MECAB_NOR_NODE)},
			{_KVi(MECAB_UNK_NODE)},
			{_KVi(MECAB_BOS_NODE)},
			{_KVi(MECAB_EOS_NODE)},
			{}
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);
	return true;
}

static kbool_t mecab_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t mecab_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t mecab_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

/* ======================================================================== */

#endif /* MECAB_GLUE_H_ */

#ifdef __cplusplus
}
#endif
