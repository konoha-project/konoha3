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

/* ************************************************************************ */

#define USING_SUGAR_AS_BUILTIN 1
#include<konoha2/sugar.h>

/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif

#include<konoha2/konoha2_local.h>

// global variable
int verbose_sugar = 0;

#include "perror.h"
#include "sugarclass.h"
#include "token.h"
#include "ast.h"
#include "tycheck.h"

#define PATTERN(T)  .kw = KW_##T##Pattern
#define TOKEN(T)  .kw = KW_##T

static void defineDefaultSyntax(CTX, kNameSpace *ks)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ TOKEN(ERR), .flag = SYNFLAG_StmtBreakExec, },
		{ PATTERN(Expr), .rule ="$expr", PatternMatch_(Expr), TopStmtTyCheck_(Expr), StmtTyCheck_(Expr),  },
		{ PATTERN(Symbol),  _TERM, PatternMatch_(Symbol),  ExprTyCheck_(Symbol),},
		{ PATTERN(Usymbol), _TERM, PatternMatch_(Usymbol), /* .rule = "$USYMBOL \"=\" $expr",*/ TopStmtTyCheck_(ConstDecl), ExprTyCheck_(Usymbol),},
		{ PATTERN(Text), _TERM, ExprTyCheck_(Text),},
		{ PATTERN(Int), _TERM, ExprTyCheck_(Int),},
		{ PATTERN(Float), _TERM, },
		{ PATTERN(Type), _TERM, PatternMatch_(Type), .rule = "$type $expr", StmtTyCheck_(TypeDecl), ExprTyCheck_(Type), },
		{ PATTERN(Parenthesis), .flag = SYNFLAG_ExprPostfixOp2, ParseExpr_(Parenthesis), .priority_op2 = 16, ExprTyCheck_(FuncStyleCall),}, //AST_PARENTHESIS
		{ PATTERN(Bracket),  },  //AST_BRACKET
		{ PATTERN(Brace),  }, // AST_BRACE
		{ PATTERN(Block), PatternMatch_(Block), ExprTyCheck_(Block), },
		{ PATTERN(Params), PatternMatch_(Params), TopStmtTyCheck_(ParamsDecl), ExprTyCheck_(MethodCall),},
		{ PATTERN(Toks), PatternMatch_(Toks), },
		{ TOKEN(DOT), ParseExpr_(DOT), .priority_op2 = 16, },
		{ TOKEN(DIV), _OP, .op2 = "opDIV", .priority_op2 = 32, },
		{ TOKEN(MOD), _OP, .op2 = "opMOD", .priority_op2 = 32, },
		{ TOKEN(MUL), _OP, .op2 = "opMUL", .priority_op2 = 32, },
		{ TOKEN(ADD), _OP, .op1 = "opPLUS", .op2 = "opADD", .priority_op2 = 64, },
		{ TOKEN(SUB), _OP, .op1 = "opMINUS", .op2 = "opSUB", .priority_op2 = 64, },
		{ TOKEN(LT), _OP, .op2 = "opLT", .priority_op2 = 256, },
		{ TOKEN(LTE), _OP, .op2 = "opLTE", .priority_op2 = 256, },
		{ TOKEN(GT), _OP, .op2 = "opGT", .priority_op2 = 256, },
		{ TOKEN(GTE), _OP, .op2 = "opGTE", .priority_op2 = 256, },
		{ TOKEN(EQ), _OP, .op2 = "opEQ", .priority_op2 = 512, },
		{ TOKEN(NEQ), _OP, .op2 = "opNEQ", .priority_op2 = 512, },
		{ TOKEN(AND), _OP, /*.op2 = ""unused*/ .priority_op2 = 1024, ExprTyCheck_(AND)},
		{ TOKEN(OR), _OP, /*.op2 = ""unused*/ .priority_op2 = 2048, ExprTyCheck_(OR)},
		{ TOKEN(NOT), _OP, .op1 = "opNOT", },
//		{ TOKEN(":"),  _OP,  .priority_op2 = 3072,},
		{ TOKEN(LET),  _OPLeft, /*.op2 = "*"*/ .priority_op2 = 4096, },
		{ TOKEN(COMMA), ParseExpr_(COMMA), .op2 = "*", .priority_op2 = 8192, /*.flag = SYNFLAG_ExprLeftJoinOP2,*/ },
		{ TOKEN(DOLLAR), ParseExpr_(DOLLAR), },
		{ TOKEN(void), .type = TY_void, .rule ="$type [$type \".\"] $SYMBOL $params [$block]", TopStmtTyCheck_(MethodDecl)},
		{ TOKEN(boolean), .type = TY_Boolean, },
		{ TOKEN(int),     .type = TY_Int, },
		{ TOKEN(true),  _TERM, ExprTyCheck_(true),},
		{ TOKEN(false),  _TERM, ExprTyCheck_(false),},
		{ TOKEN(if), .rule ="\"if\" \"(\" $expr \")\" $block [\"else\" else: $block]", TopStmtTyCheck_(if), StmtTyCheck_(if), },
		{ TOKEN(else), .rule = "\"else\" $block", TopStmtTyCheck_(else), StmtTyCheck_(else), },
		{ TOKEN(return), .rule ="\"return\" [$expr]", .flag = SYNFLAG_StmtBreakExec, StmtTyCheck_(return), },
		{ .kw = KW_END, },
	};
	NameSpace_defineSyntax(_ctx, ks, SYNTAX);
	struct _ksyntax *syn = (struct _ksyntax*)SYN_(ks, KW_void);
	syn->ty = TY_void; // it's not cool, but necessary
	syn = (struct _ksyntax*)SYN_(ks, KW_UsymbolPattern);
	KINITv(syn->syntaxRuleNULL, new_(TokenArray, 0));
	parseSyntaxRule(_ctx, "$USYMBOL \"=\" $expr", 0, syn->syntaxRuleNULL);
}

/* ------------------------------------------------------------------------ */
/* ctxsugar_t global functions */

static kstatus_t NameSpace_eval(CTX, kNameSpace *ks, const char *script, kline_t uline)
{
	kstatus_t result;
	kmodsugar->h.setup(_ctx, (kmodshare_t*)kmodsugar, 0/*lazy*/);
	{
		INIT_GCSTACK();
		kArray *tls = ctxsugar->tokens;
		size_t pos = kArray_size(tls);
		NameSpace_tokenize(_ctx, ks, script, uline, tls);
		kBlock *bk = new_Block(_ctx, ks, NULL, tls, pos, kArray_size(tls), ';');
		kArray_clear(tls, pos);
		result = Block_eval(_ctx, bk);
		RESET_GCSTACK();
	}
	return result;
}

kstatus_t MODSUGAR_eval(CTX, const char *script, kline_t uline)
{
	if(verbose_sugar) {
		DUMP_P("\n>>>----\n'%s'\n------\n", script);
	}
	kmodsugar->h.setup(_ctx, (kmodshare_t*)kmodsugar, 0/*lazy*/);
	return NameSpace_eval(_ctx, KNULL(NameSpace), script, uline);
}

/* ------------------------------------------------------------------------ */
/* [ctxsugar] */

static void ctxsugar_reftrace(CTX, struct kmodlocal_t *baseh)
{
	ctxsugar_t *base = (ctxsugar_t*)baseh;
	BEGIN_REFTRACE(7);
	KREFTRACEv(base->tokens);
	KREFTRACEv(base->errors);
	KREFTRACEv(base->gma);
	KREFTRACEv(base->lvarlst);
	KREFTRACEv(base->singleBlock);
	KREFTRACEv(base->definedMethods);
	END_REFTRACE();
}
static void ctxsugar_free(CTX, struct kmodlocal_t *baseh)
{
	ctxsugar_t *base = (ctxsugar_t*)baseh;
	KARRAY_FREE(&base->cwb);
	KFREE(base, sizeof(ctxsugar_t));
}

static void kmodsugar_setup(CTX, struct kmodshare_t *def, int newctx)
{
	if(!newctx && _ctx->modlocal[MOD_sugar] == NULL) {
		ctxsugar_t *base = (ctxsugar_t*)KCALLOC(sizeof(ctxsugar_t), 1);
		base->h.reftrace = ctxsugar_reftrace;
		base->h.free     = ctxsugar_free;
		KINITv(base->tokens, new_(TokenArray, K_PAGESIZE/sizeof(void*)));
		base->err_count = 0;
		KINITv(base->errors, new_(StringArray, 8));
		KINITv(base->lvarlst, new_(ExprArray, K_PAGESIZE/sizeof(void*)));
		KINITv(base->definedMethods, new_(MethodArray, 8));

		KINITv(base->gma, new_(Gamma, NULL));
		KINITv(base->singleBlock, new_(Block, NULL));
		kArray_add(base->singleBlock->blocks, K_NULL);
		KARRAY_INIT(&base->cwb, K_PAGESIZE);
		_ctx->modlocal[MOD_sugar] = (kmodlocal_t*)base;
	}
}

static void pack_reftrace(CTX, kmape_t *p)
{
	kpackage_t *pack = (kpackage_t*)p->uvalue;
	BEGIN_REFTRACE(1);
	KREFTRACEn(pack->ks);
	END_REFTRACE();
}

static void pack_free(CTX, void *p)
{
	KFREE(p, sizeof(kpackage_t));
}

static void kmodsugar_reftrace(CTX, struct kmodshare_t *baseh)
{
	kmodsugar_t *base = (kmodsugar_t*)baseh;
	kmap_reftrace(base->packageMapNO, pack_reftrace);
	BEGIN_REFTRACE(6);
	KREFTRACEv(base->packageList);
	KREFTRACEv(base->UndefinedParseExpr);
	KREFTRACEv(base->UndefinedStmtTyCheck);
	KREFTRACEv(base->UndefinedExprTyCheck);
	KREFTRACEv(base->ParseExpr_Term);
	KREFTRACEv(base->ParseExpr_Op);
	END_REFTRACE();
}

static void kmodsugar_free(CTX, struct kmodshare_t *baseh)
{
	kmodsugar_t *base = (kmodsugar_t*)baseh;
	kmap_free(base->packageMapNO, pack_free);
	KFREE(baseh, sizeof(kmodsugar_t));
}

void MODSUGAR_init(CTX, kcontext_t *ctx)
{
	kmodsugar_t *base = (kmodsugar_t*)KCALLOC(sizeof(kmodsugar_t), 1);
	base->h.name     = "sugar";
	base->h.setup    = kmodsugar_setup;
	base->h.reftrace = kmodsugar_reftrace;
	base->h.free     = kmodsugar_free;
	Konoha_setModule(MOD_sugar, (kmodshare_t*)base, 0);

	struct _klib2* l = (struct _klib2*)ctx->lib2;
	l->KS_getCT   = NameSpace_getCT;
	l->KS_loadMethodData = NameSpace_loadMethodData;
	l->KS_loadConstData  = NameSpace_loadConstData;
	l->KS_getMethodNULL  = NameSpace_getMethodNULL;
	l->KS_syncMethods    = NameSpace_syncMethods;

	KINITv(base->packageList, new_(Array, 8));
	base->packageMapNO = kmap_init(0);

	KDEFINE_CLASS defNameSpace = {
		STRUCTNAME(NameSpace),
		.init = NameSpace_init,
		.reftrace = NameSpace_reftrace,
		.free = NameSpace_free,
	};
	KDEFINE_CLASS defToken = {
		STRUCTNAME(Token),
		.init = Token_init,
		.reftrace = Token_reftrace,
	};
	KDEFINE_CLASS defExpr = {
		STRUCTNAME(Expr),
		.init = Expr_init,
		.reftrace = Expr_reftrace,
	};
	KDEFINE_CLASS defStmt = {
		STRUCTNAME(Stmt),
		.init = Stmt_init,
		.reftrace = Stmt_reftrace,
	};
	KDEFINE_CLASS defBlock = {
		STRUCTNAME(Block),
		.init = Block_init,
		.reftrace = Block_reftrace,
	};
	KDEFINE_CLASS defGamma = {
		STRUCTNAME(Gamma),
		.init = Gamma_init,
	};
	base->cNameSpace = Konoha_addClassDef(PN_sugar, PN_sugar, NULL, &defNameSpace, 0);
	base->cToken = Konoha_addClassDef(PN_sugar, PN_sugar, NULL, &defToken, 0);
	base->cExpr  = Konoha_addClassDef(PN_sugar, PN_sugar, NULL, &defExpr, 0);
	base->cStmt  = Konoha_addClassDef(PN_sugar, PN_sugar, NULL, &defStmt, 0);
	base->cBlock = Konoha_addClassDef(PN_sugar, PN_sugar, NULL, &defBlock, 0);
	base->cGamma = Konoha_addClassDef(PN_sugar, PN_sugar, NULL, &defGamma, 0);
	base->cTokenArray = CT_p0(_ctx, CT_Array, base->cToken->cid);

	knull(base->cNameSpace);
	knull(base->cToken);
	knull(base->cExpr);
	knull(base->cBlock);
	kmodsugar_setup(_ctx, &base->h, 0);

	KINITv(base->UndefinedParseExpr,   new_SugarFunc(UndefinedParseExpr));
	KINITv(base->UndefinedStmtTyCheck, new_SugarFunc(UndefinedStmtTyCheck));
	KINITv(base->UndefinedExprTyCheck, new_SugarFunc(UndefinedExprTyCheck));
	KINITv(base->ParseExpr_Op,   new_SugarFunc(ParseExpr_Op));
	KINITv(base->ParseExpr_Term, new_SugarFunc(ParseExpr_Term));

	defineDefaultSyntax(_ctx, KNULL(NameSpace));
	DBG_ASSERT(SYM_("$params") == KW_ParamsPattern);
	DBG_ASSERT(SYM_(".") == KW_DOT);
	DBG_ASSERT(SYM_(",") == KW_COMMA);
	DBG_ASSERT(SYM_("void") == KW_void);
	DBG_ASSERT(SYM_("return") == KW_return);
	DBG_ASSERT(SYM_("new") == KW_new);
	EXPORT_SUGAR(base);
}

// -------------------------------------------------------------------------

#ifndef EOF
#define EOF (-1)
#endif

static kline_t readquote(CTX, FILE_i *fp, kline_t line, kwb_t *wb, int quote)
{
	int ch, prev = quote;
	while((ch = PLAT fgetc_i(fp)) != EOF) {
		if(ch == '\r') continue;
		if(ch == '\n') line++;
		kwb_putc(wb, ch);  // SLOW
		if(ch == quote && prev != '\\') {
			return line;
		}
		prev = ch;
	}
	return line;
}

static kline_t readcomment(CTX, FILE_i *fp, kline_t line, kwb_t *wb)
{
	int ch, prev = 0, level = 1;
	while((ch = PLAT fgetc_i(fp)) != EOF) {
		if(ch == '\r') continue;
		if(ch == '\n') line++;
		kwb_putc(wb, ch);  // SLOW
		if(prev == '*' && ch == '/') level--;
		if(prev == '/' && ch == '*') level++;
		if(level == 0) return line;
		prev = ch;
	}
	return line;
}

static kline_t readchunk(CTX, FILE_i *fp, kline_t line, kwb_t *wb)
{
	int ch;
	int prev = 0, isBLOCK = 0;
	while((ch = PLAT fgetc_i(fp)) != EOF) {
		if(ch == '\r') continue;
		if(ch == '\n') line++;
		kwb_putc(wb, ch);  // SLOW
		if(prev == '/' && ch == '*') {
			line = readcomment(_ctx, fp, line, wb);
			continue;
		}
		if(ch == '\'' || ch == '"' || ch == '`') {
			line = readquote(_ctx, fp, line, wb, ch);
			continue;
		}
		if(isBLOCK != 1 && prev == '\n' && ch == '\n') {
			break;
		}
		if(prev == '{') {
			isBLOCK = 1;
		}
		if(prev == '\n' && ch == '}') {
			isBLOCK = 0;
		}
		prev = ch;
	}
	return line;
}

static int isemptychunk(const char *t, size_t len)
{
	size_t i;
	for(i = 0; i < len; i++) {
		if(!isspace(t[i])) return 1;
	}
	return 0;
}

static kstatus_t NameSpace_loadstream(CTX, kNameSpace *ns, FILE_i *fp, kline_t uline, kline_t pline)
{
	kstatus_t status = K_CONTINUE;
	kwb_t wb;
	char *p;
	kwb_init(&(_ctx->stack->cwb), &wb);
	while(! PLAT feof_i(fp)) {
		kline_t chunkheadline = uline;
		uline = readchunk(_ctx, fp, uline, &wb);
		const char *script = kwb_top(&wb, 1);
		size_t len = kwb_bytesize(&wb);
		if (len > 2 && script[0] == '#' && script[1] == '!') {
			if ((p = strstr(script, "konoha")) != 0) {
				p += 6;
				script = p;
			} else {
				//FIXME: its not konoha shell, need to exec??
				kreportf(ERR_, pline, "it may not konoha script: %s", SS_t(uline));
				status = K_FAILED;
				break;
			}
		}
		if(isemptychunk(script, len)) {
			status = MODSUGAR_eval(_ctx, script, /*len, */chunkheadline);
		}
		if(status != K_CONTINUE) break;
		kwb_free(&wb);
	}
	kwb_free(&wb);
	if(status != K_CONTINUE) {
		kreportf(DEBUG_, pline, "running script is failed: %s", SS_t(uline));
	}
	return status;
}

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

static kline_t uline_init(CTX, const char *path, size_t len, int line, int isreal)
{
	kline_t uline = line;
	if(isreal) {
		char buf[PATH_MAX];
		char *ptr = PLAT realpath_i(path, buf);
		uline |= kfileid((const char*)buf, strlen(ptr), 0, _NEWID);
		if(ptr != buf && ptr != NULL) {
			PLAT free_i(ptr);
		}
	}
	else {
		uline |= kfileid(path, len, 0, _NEWID);
	}
	return uline;
}

static kstatus_t NameSpace_loadscript(CTX, kNameSpace *ks, const char *path, size_t len, kline_t pline)
{
	kstatus_t status = K_BREAK;
//	if(path[0] == '-' && path[1] == 0) {
//		kline_t uline = FILEID_("<stdin>") | 1;
//		status = NameSpace_loadstream(_ctx, ks, stdin, uline, pline);
//	}
//	else {
		FILE_i *fp = PLAT fopen_i(path, "r");
		if(fp != NULL) {
			kline_t uline = uline_init(_ctx, path, len, 1, 1);
			status = NameSpace_loadstream(_ctx, ks, fp, uline, pline);
			PLAT fclose_i(fp);
		}
		else {
			kreportf(ERR_, pline, "script not found: %s", path);
		}
//	}
	return status;
}

kstatus_t MODSUGAR_loadscript(CTX, const char *path, size_t len, kline_t pline)
{
	if (ctxsugar == NULL) {
		kmodsugar->h.setup(_ctx, (kmodshare_t*)kmodsugar, 0/*lazy*/);
	}
	INIT_GCSTACK();
	kNameSpace *ns = new_(NameSpace, KNULL(NameSpace));
	PUSH_GCSTACK(ns);
	kstatus_t result = NameSpace_loadscript(_ctx, ns, path, len, pline);
	RESET_GCSTACK();
	return result;
}

// ---------------------------------------------------------------------------
// package

static const char* packname(const char *str)
{
	char *p = strrchr(str, '.');
	return (p == NULL) ? str : (const char*)p+1;
}

static KDEFINE_PACKAGE PKGDEFNULL = {
	.konoha_checksum = 0,
	.name = "*stub",
	.version = "0.0",
	.note = "this is stub",
	.initPackage = NULL,
	.setupPackage = NULL,
	.initNameSpace = NULL,
	.setupNameSpace = NULL,
	.konoha_revision = 0,
};

static KDEFINE_PACKAGE *NameSpace_openGlueHandler(CTX, kNameSpace *ks, char *pathbuf, size_t bufsiz, const char *pname, kline_t pline)
{
	char *p = strrchr(pathbuf, '.');
//	snprintf(p, bufsiz - (p  - pathbuf), "%s", K_OSDLLEXT);
	strncpy(p, K_OSDLLEXT, bufsiz - (p  - pathbuf));
	void *gluehdr = dlopen(pathbuf, CTX_isCompileOnly(_ctx) ? RTLD_NOW : RTLD_LAZY);  // FIXME
	if(gluehdr != NULL) {
		char funcbuf[80];
		PLAT snprintf_i(funcbuf, sizeof(funcbuf), "%s_init", packname(pname));
		Fpackageinit f = (Fpackageinit)dlsym(gluehdr, funcbuf);
		if(f != NULL) {
			KDEFINE_PACKAGE *packdef = f();
			return (packdef != NULL) ? packdef : &PKGDEFNULL;
		}
		else {
			kreportf(WARN_, pline, "package loader: %s has no %s function", pathbuf, funcbuf);
		}
	}
	else {
		kreportf(DEBUG_, pline, "package loader: %s has no glue library: %s", pname, pathbuf);
	}
	return &PKGDEFNULL;
}

static kNameSpace* new_NameSpace(CTX, kpack_t packdom, kpack_t packid)
{
	struct _kNameSpace *ks = new_W(NameSpace, KNULL(NameSpace));
	ks->packid = packid;
	ks->packdom = packid;
	return (kNameSpace*)ks;
}

static kpackage_t *loadPackageNULL(CTX, kpack_t packid, kline_t pline)
{
	char fbuf[256];
	const char *path = PLAT packagepath(fbuf, sizeof(fbuf), S_text(PN_s(packid)));
	FILE_i *fp = PLAT fopen_i(path, "r");
	kpackage_t *pack = NULL;
	if(fp != NULL) {
		INIT_GCSTACK();
		kNameSpace *ks = new_NameSpace(_ctx, packid, packid);
		PUSH_GCSTACK(ks);
		kline_t uline = uline_init(_ctx, path, strlen(path), 1, 1);
		KDEFINE_PACKAGE *packdef = NameSpace_openGlueHandler(_ctx, ks, fbuf, sizeof(fbuf), PN_t(packid), pline);
		if(packdef->initPackage != NULL) {
			packdef->initPackage(_ctx, ks, 0, NULL, pline);
		}
		if(NameSpace_loadstream(_ctx, ks, fp, uline, pline) == K_CONTINUE) {
			if(packdef->initPackage != NULL) {
				packdef->setupPackage(_ctx, ks, pline);
			}
			pack = (kpackage_t*)KCALLOC(sizeof(kpackage_t), 1);
			pack->packid = packid;
			KINITv(pack->ks, ks);
			pack->packdef = packdef;
			if(PLAT exportpath(fbuf, sizeof(fbuf), PN_t(packid)) != NULL) {
				pack->export_script = kfileid(fbuf, strlen(fbuf), 0, _NEWID) | 1;
			}
			return pack;
		}
		PLAT fclose_i(fp);
		RESET_GCSTACK();
	}
	else {
		kreportf(CRIT_, pline, "package not found: %s path=%s", PN_t(packid), path);
	}
	return NULL;
}

static kpackage_t *getPackageNULL(CTX, kpack_t packid, kline_t pline)
{
	kpackage_t *pack = (kpackage_t*)map_getu(_ctx, kmodsugar->packageMapNO, packid, uNULL);
	if(pack != NULL) return pack;
	pack = loadPackageNULL(_ctx, packid, pline);
	if(pack != NULL) {
		map_addu(_ctx, kmodsugar->packageMapNO, packid, (uintptr_t)pack);
	}
	return pack;
}

static void NameSpace_merge(CTX, kNameSpace *ks, kNameSpace *target, kline_t pline)
{
	if(target->packid != PN_konoha) {
		NameSpace_importClassName(_ctx, ks, target->packid, pline);
	}
	if(target->cl.bytesize > 0) {
		NameSpace_mergeConstData(_ctx, (struct _kNameSpace*)ks, target->cl.kvs, target->cl.bytesize/sizeof(kvs_t), pline);
	}
	size_t i;
	for(i = 0; i < kArray_size(target->methods); i++) {
		kMethod *mtd = target->methods->methods[i];
		if(kMethod_isPublic(mtd) && mtd->packid == target->packid) {
			kArray_add(ks->methods, mtd);
		}
	}
}

static kbool_t NameSpace_importPackage(CTX, kNameSpace *ks, const char *name, kline_t pline)
{
	kbool_t res = 0;
	kpack_t packid = kpack(name, strlen(name), 0, _NEWID);
	kpackage_t *pack = getPackageNULL(_ctx, packid, pline);
	if(pack != NULL) {
		res = 1;
		if(ks != NULL) {
			NameSpace_merge(_ctx, ks, pack->ks, pline);
			if(pack->packdef->initNameSpace != NULL) {
				res = pack->packdef->initNameSpace(_ctx, ks, pline);
			}
			if(res && pack->export_script != 0) {
				kString *fname = SS_s(pack->export_script);
				kline_t uline = pack->export_script | (kline_t)1;
				FILE_i *fp = PLAT fopen_i(S_text(fname), "r");
				if(fp != NULL) {
					res = (NameSpace_loadstream(_ctx, ks, fp, uline, pline) == K_CONTINUE);
					PLAT fclose_i(fp);
				}
				else {
					kreportf(WARN_, pline, "script not found: %s", S_text(fname));
					res = 0;
				}
			}
			if(res && pack->packdef->setupNameSpace != NULL) {
				res = pack->packdef->setupNameSpace(_ctx, ks, pline);
			}
		}
	}
	return res;
}

// boolean NameSpace.importPackage(String pkgname);
static KMETHOD NameSpace_importPackage_(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(NameSpace_importPackage(_ctx, sfp[0].ks, S_text(sfp[1].s), sfp[K_RTNIDX].uline));
}

// boolean NameSpace.loadScript(String path);
static KMETHOD NameSpace_loadScript_(CTX, ksfp_t *sfp _RIX)
{
	kline_t pline = sfp[K_RTNIDX].uline;
	FILE_i *fp = PLAT fopen_i(S_text(sfp[1].s), "r");
	if(fp != NULL) {
		kline_t uline = uline_init(_ctx, S_text(sfp[1].s), S_size(sfp[1].s), 1, 1);
		kstatus_t status = NameSpace_loadstream(_ctx, sfp[0].ks, fp, uline, 0);
		PLAT fclose_i(fp);
		RETURNb_(status == K_CONTINUE);
	}
	else {
		kreportf(ERR_, pline, "script not found: %s", S_text(sfp[1].s));
		RETURNb_(0);
	}
}

#define _Public kMethod_Public
#define _Static kMethod_Static
#define _F(F)   (intptr_t)(F)
#define TY_NameSpace  (CT_NameSpace)->cid
KDEFINE_PACKAGE* konoha_init(void);

void MODSUGAR_loadMethod(CTX)
{
	KDEFINE_METHOD MethodData[] = {
//		_Public, _F(NameSpace_importPackage_), TY_Boolean, TY_NameSpace, MN_("importPackage"), 1, TY_String, FN_pkgname,
		_Public, _F(NameSpace_importPackage_), TY_Boolean, TY_NameSpace, MN_("import"), 1, TY_String, FN_("name"),
		_Public, _F(NameSpace_loadScript_), TY_Boolean, TY_NameSpace, MN_("load"), 1, TY_String, FN_("path"),
		DEND,
	};
	kNameSpace_loadMethodData(NULL, MethodData);
	KSET_KLIB2(importPackage, NameSpace_importPackage, 0);
//#ifdef WITH_ECLIPSE
//	KDEFINE_PACKAGE *d = konoha_init();
//	d->initPackage(_ctx, KNULL(NameSpace), 0, NULL, 0);
//	d->setupPackage(_ctx, KNULL(NameSpace), 0);
//	d->initNameSpace(_ctx, KNULL(NameSpace), 0);
//	d->setupNameSpace(_ctx, KNULL(NameSpace), 0);
//#endif
}

#ifdef __cplusplus
}
#endif
