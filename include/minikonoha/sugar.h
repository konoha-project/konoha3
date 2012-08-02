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

#ifndef SUGAR_H_
#define SUGAR_H_

/* ------------------------------------------------------------------------ */
/* sugar.h */

#ifndef __KERNEL__
#include <dlfcn.h>
#endif
#include "minikonoha.h"
#include "klib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KW_t(X)   SYM_PRE(X),SYM_t(X)

#define TokenType_ERR          KW_TokenPattern
#define TokenType_NONE         0
#define TokenType_INDENT       1
#define TokenType_SYMBOL       KW_SymbolPattern
#define TokenType_TEXT         KW_TextPattern
#define TokenType_INT          KW_NumberPattern
#define TokenType_FLOAT        KW_FloatPattern
#define TokenType_CODE         KW_BlockPattern

#define KW_END  ((ksymbol_t)-1)
#define KW_ERR  (((ksymbol_t)0)|0) /**/
#define KW_ExprPattern      (((ksymbol_t)1)|KW_PATTERN) /*$Expr*/
#define KW_SymbolPattern    (((ksymbol_t)2)|KW_PATTERN) /*$Symbol*/
#define KW_ConstPattern     (((ksymbol_t)3)|KW_PATTERN) /*$Const*/
#define KW_TextPattern      (((ksymbol_t)4)|KW_PATTERN) /*$Text*/
#define KW_NumberPattern    (((ksymbol_t)5)|KW_PATTERN) /*$Number*/
#define KW_FloatPattern     (((ksymbol_t)6)|KW_PATTERN) /*$Float*/
#define KW_TypePattern      (((ksymbol_t)7)|KW_PATTERN) /*$Type*/

#define KW_ParenthesisGroup (((ksymbol_t)8)) /*()*/
#define KW_BracketGroup     (((ksymbol_t)9)) /*[]*/
#define KW_BraceGroup       (((ksymbol_t)10)) /*{}*/
#define KW_TypeCastGroup    (((ksymbol_t)8)|KW_PATTERN) /*$()*/
#define KW_TypeParamGroup   (((ksymbol_t)9)|KW_PATTERN) /*$[]*/
#define KW_OptionalGroupGroup   (((ksymbol_t)9)|KW_ATMARK)  /*@[]*/
#define KW_BlockPattern    (((ksymbol_t)11)|KW_PATTERN) /*$Block*/
#define KW_ParamPattern    (((ksymbol_t)12)|KW_PATTERN) /*$Param*/
#define KW_TokenPattern    (((ksymbol_t)13)|KW_PATTERN) /*$Token*/

#define KW_StmtConstDecl   KW_ConstPattern
#define KW_StmtTypeDecl    KW_TypePattern
#define KW_ExprMethodCall  KW_ParamPattern
#define KW_StmtMethodDecl  KW_void

#define KW_DOT     14
#define KW_DIV     (1+KW_DOT)
#define KW_MOD     (2+KW_DOT)
#define KW_MUL     (3+KW_DOT)
#define KW_ADD     (4+KW_DOT)
#define KW_SUB     (5+KW_DOT)
#define KW_LT      (6+KW_DOT)
#define KW_LTE     (7+KW_DOT)
#define KW_GT      (8+KW_DOT)
#define KW_GTE     (9+KW_DOT)
#define KW_EQ      (10+KW_DOT)
#define KW_NEQ     (11+KW_DOT)
#define KW_AND     (12+KW_DOT)
#define KW_OR      (13+KW_DOT)
#define KW_NOT     (14+KW_DOT)
#define KW_LET     (15+KW_DOT)
#define KW_COMMA   (16+KW_DOT)
#define KW_DOLLAR  KW_PATTERN
#define KW_ATMARK  MN_Annotation

// #define KW_void (((ksymbol_t)32)|0) /*void*/

#define KW_void      31
#define KW_boolean   (1+KW_void)
#define KW_int       (2+KW_void)
#define KW_true      (3+KW_void)
#define KW_false     (4+KW_void)
#define KW_if        (5+KW_void)
#define KW_else      (6+KW_void)
#define KW_return    (7+KW_void)
// reserved
#define KW_new       (8+KW_void)
#define FN_this      FN_("this")


#define kflag_clear(flag)  (flag) = 0
#define K_CHECKSUM 1

#define KPACKNAME(N, V) \
	.name = N, .version = V, .konoha_checksum = K_CHECKSUM, .konoha_revision = K_REVISION

#define KPACKLIB(N, V) \
	.libname = N, .libversion = V

typedef enum {  Nope, isFirstTime } isFirstTime_t;

struct KonohaPackageHandlerVar {
	int konoha_checksum;
	const char *name;
	const char *version;
	const char *libname;
	const char *libversion;
	const char *note;
	kbool_t (*initPackage)   (KonohaContext *kctx, kNameSpace *, int, const char**, kfileline_t);
	kbool_t (*setupPackage)  (KonohaContext *kctx, kNameSpace *, isFirstTime_t, kfileline_t);
	kbool_t (*initNameSpace) (KonohaContext *kctx, kNameSpace *, kfileline_t);
	kbool_t (*setupNameSpace)(KonohaContext *kctx, kNameSpace *, kfileline_t);
	int konoha_revision;
};

typedef struct KonohaPackageVar KonohaPackage;

struct KonohaPackageVar {
	kpackage_t                   packageId;
	kNameSpace                  *packageNameSpace;
	KonohaPackageHandler        *packageHandler;
	kfileline_t                  exportScriptUri;
};

// Tokenizer

#define KCHAR_MAX  41
typedef struct TokenizerEnv TokenizerEnv;
typedef int (*TokenizeFunc)(KonohaContext *, kTokenVar *, TokenizerEnv *, int);

struct TokenizerEnv {
	const char         *source;
	size_t              sourceLength;
	kfileline_t         currentLine;
	kArray             *tokenList;
	int                 tabsize;
	const TokenizeFunc *cfuncItems;
	union {
		kFunc         **funcItems;
		kArray        **funcListItems;
	};
	kString            *preparedString;
};

/******
// ParseToken
#define VAR_ParseToken(TK, STR, UL) \
		kTokenVar *TK = (kTokenVar*)sfp[0].asObject;\
		kString *STR = sfp[1].asString;\
		int UL = (int)sfp[2].intValue;\
		(void)TK; (void)STR; (void)UL;\

*****/

// int PatternMatch(Stmt stmt, int nameid, Token[] toks, int s, int e)
#define VAR_PatternMatch(STMT, NAME, TLS, S, E) \
		kStmt *STMT = (kStmt*)sfp[1].asObject;\
		ksymbol_t NAME = (ksymbol_t)sfp[2].intValue;\
		kArray *TLS = (kArray*)sfp[3].o;\
		int S = (int)sfp[4].intValue;\
		int E = (int)sfp[5].intValue;\
		(void)STMT; (void)NAME; (void)TLS; (void)S; (void)E;\

// Expr ParseExpr(Stmt stmt, Token[] tokenArray, int s, int c, int e)
#define VAR_ParseExpr(STMT, TLS, S, C, E) \
		SugarSyntax *syn = (SugarSyntax*)sfp[0].unboxValue;\
		kStmt *STMT = (kStmt*)sfp[1].asObject;\
		kArray *TLS = (kArray*)sfp[2].o;\
		int S = (int)sfp[3].intValue;\
		int C = (int)sfp[4].intValue;\
		int E = (int)sfp[5].intValue;\
		(void)syn; (void)STMT; (void)TLS; (void)S; (void)C; (void)E;\

// boolean StmtTyCheck(Stmt stmt, Gamma gma)
#define VAR_StmtTyCheck(STMT, GMA) \
		kStmt *STMT = (kStmt*)sfp[1].asObject;\
		kGamma *GMA = (kGamma*)sfp[2].o;\
		(void)STMT; (void)GMA;\

// Expr ExprTyCheck(Stmt stmt, Expr expr, Gamma gma, int typeid)
#define VAR_ExprTyCheck(STMT, EXPR, GMA, TY) \
		kStmt *STMT = (kStmt*)sfp[1].asObject;\
		kExpr *EXPR = (kExpr*)sfp[2].o;\
		kGamma *GMA = (kGamma*)sfp[3].o;\
		ktype_t TY = (ktype_t)sfp[4].intValue;\
		(void)STMT; (void)EXPR; (void)GMA; (void)TY;\

typedef const struct SugarSyntaxVar   SugarSyntax;
typedef struct SugarSyntaxVar         SugarSyntaxVar;

struct SugarSyntaxVar {
	ksymbol_t  keyword;               kshortflag_t  flag;
	kArray                           *syntaxRuleNULL;
	kFunc                            *PatternMatch;
	kFunc                            *ParseExpr;
	kFunc                            *TopStmtTyCheck;
	kFunc                            *StmtTyCheck;
	kFunc                            *ExprTyCheck;
	// binary
	ktype_t  ty;
	kshort_t precedence_op2;        kshort_t precedence_op1;
};

#define SYNIDX_PatternMatch   0
#define SYNIDX_ParseExpr      1
#define SYNIDX_TopStmtTyCheck 2
#define SYNIDX_StmtTyCheck    3
#define SYNIDX_ExprTyCheck    4

#define PatternMatch_(NAME)    .PatternMatch   = PatternMatch_##NAME
#define ParseExpr_(NAME)       .ParseExpr      = ParseExpr_##NAME
#define TopStmtTyCheck_(NAME)  .TopStmtTyCheck = StmtTyCheck_##NAME
#define StmtTyCheck_(NAME)     .StmtTyCheck    = StmtTyCheck_##NAME
#define ExprTyCheck_(NAME)     .ExprTyCheck    = ExprTyCheck_##NAME

#define _TERM     .flag = SYNFLAG_ExprTerm
#define _OP       .flag = SYNFLAG_ExprOp
#define _OPLeft   .flag = (SYNFLAG_ExprOp|SYNFLAG_ExprLeftJoinOp2)

#define SYNFLAG_ExprTerm           ((kshortflag_t)1)
#define SYNFLAG_ExprOp             ((kshortflag_t)1 << 1)
#define SYNFLAG_ExprLeftJoinOp2    ((kshortflag_t)1 << 2)
#define SYNFLAG_ExprPostfixOp2     ((kshortflag_t)1 << 3)

#define SYNFLAG_StmtBreakExec      ((kshortflag_t)1 << 8)  /* return, throw */
#define SYNFLAG_StmtJumpAhead      ((kshortflag_t)1 << 9)  /* continue */
#define SYNFLAG_StmtJumpSkip       ((kshortflag_t)1 << 10)  /* break */

typedef struct KDEFINE_SYNTAX {
	ksymbol_t keyword;  kshortflag_t flag;
	const char *rule;
//	const char *op2;
//	const char *op1;
	int precedence_op2;
	int precedence_op1;
	int type;
	MethodFunc PatternMatch;
	MethodFunc ParseExpr;
	MethodFunc TopStmtTyCheck;
	MethodFunc StmtTyCheck;
	MethodFunc ExprTyCheck;
} KDEFINE_SYNTAX;

#define new_SugarFunc(F)     new_(Func, KLIB new_kMethod(kctx, 0, 0, 0, F))

#define SIZEOF_TOKENMATRIX (KCHAR_MAX * sizeof(TokenizeFunc) * 2)

struct kNameSpaceVar {
	KonohaObjectHeader h;
	kpackage_t packageId;  	kpackage_t packageDomain;
	kNameSpace                        *parentNULL;
	const TokenizeFunc                *tokenMatrix;
	KUtilsHashMap                     *syntaxMapNN;
	//
	kObject                           *scriptObject;
	kArray*                            methodList;   // default K_EMPTYARRAY
	KUtilsGrowingArray                 constTable;        // const variable
};

/* Token */

struct kTokenVar {
	KonohaObjectHeader h;
	union {
		kString *text;
		kArray  *subTokenList;
	};
	kfileline_t     uline;
	SugarSyntax    *resolvedSyntaxInfo;
	union {
		ksymbol_t   unresolvedTokenType; // (resolvedSyntaxInfo == NULL)
		ksymbol_t   resolvedSymbol;      // symbol (resolvedSyntaxInfo != NULL)
		ktype_t     resolvedTypeId;      // typeid if KW_TypePattern
	};
	union {
		kushort_t   indent;               // indent when kw == TokenType_INDENT
		kshort_t    topCharHint;
		ksymbol_t   stmtEntryKey;         // pattern name for 'setting key in Stmt'
	};
};

#define Token_isRule(o)      (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define Token_setRule(o,B)   TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local1,B)

typedef struct TokenChunk {
	kArray *tokenList;
	int beginIdx;
	int endIdx;
	kNameSpace *ns;
	kToken *errToken;
} TokenChunk;

typedef kbool_t (*CheckEndOfStmtFunc2)(KonohaContext *, TokenChunk *chunk, TokenChunk *sourceChunk, int *currentIdxRef, int *indentRef);

#define Token_isVirtualTypeLiteral(TK)     ((TK)->resolvedSyntaxInfo->keyword == KW_TypePattern)
#define Token_typeLiteral(TK)              (TK)->resolvedTypeId

#define TEXPR_UNTYPED       -1   /*THIS MUST NOT HAPPEN*/
#define TEXPR_CONST          0
#define TEXPR_NEW            1
#define TEXPR_NULL           2
#define TEXPR_NCONST         3
#define TEXPR_LOCAL          4/*variable*/
#define TEXPR_BLOCK          5
#define TEXPR_FIELD          6/*variable*/
#define TEXPR_BOX            7
#define TEXPR_UNBOX          8
#define TEXPR_CALL           9
#define TEXPR_AND           10
#define TEXPR_OR            11
#define TEXPR_LET           12
#define TEXPR_STACKTOP      13
#define TEXPR_MAX           14

#define Expr_isCONST(o)     (TEXPR_CONST <= (o)->build && (o)->build <= TEXPR_NCONST)
#define Expr_isTerm(o)      (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define Expr_setTerm(o,B)   TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local1,B)

#define Expr_hasObjectConstValue(o)      (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local2))
#define Expr_setObjectConstValue(o,B)   TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local2,B)

#define kExpr_at(E,N)        ((E)->cons->exprItems[(N)])

typedef kshort_t    kexpr_t;

struct kExprVar {
	KonohaObjectHeader h;
	SugarSyntax *syn;
	union {
		kToken  *termToken;     // Term
		kArray*  cons;          // Cons
		kExpr*   single;
		kBlock*  block;
	};
	ktype_t ty;    kexpr_t build;
	union {
		kObject*   objectConstValue;
		uintptr_t  unboxConstValue;
		intptr_t   index;
	};
};

#define TSTMT_UNDEFINED      0
#define TSTMT_ERR            1
#define TSTMT_EXPR           2
#define TSTMT_BLOCK          3
#define TSTMT_RETURN         4
#define TSTMT_IF             5
#define TSTMT_LOOP           6
#define TSTMT_JUMP           7

struct kStmtVar {
	KonohaObjectHeader h;
	kfileline_t        uline;
	SugarSyntax       *syn;
	kBlock            *parentBlockNULL;
	kushort_t          build;
};

#define kStmt_getObjectNULL(CTX, O, K)            (KLIB kObject_getObject(CTX, UPCAST(O), K, NULL))
#define kStmt_getObject(CTX, O, K, DEF)           (KLIB kObject_getObject(CTX, UPCAST(O), K, DEF))
#define kStmt_setObject(CTX, O, K, V)             KLIB kObject_setObject(CTX, UPCAST(O), K, O_typeId(V), UPCAST(V))
#define kStmt_getUnboxValue(CTX, O, K, DEF)       (KLIB kObject_getUnboxValue(CTX, UPCAST(O), K, DEF))
#define kStmt_setUnboxValue(CTX, O, K, T, V)      KLIB kObject_setUnboxValue(CTX, UPCAST(O), K, T, V)
#define kStmt_removeKey(CTX, O, K)                KLIB kObject_removeKey(CTX, UPCAST(O), K)
#define kStmt_protoEach(CTX, O, THUNK, F)         KLIB kObject_protoEach(CTX, UPCAST(O), THUNK, F)

struct kBlockVar {
	KonohaObjectHeader   h;
	kNameSpace          *blockNameSpace;
	kStmt               *parentStmtNULL;
	kArray              *stmtList;
	kExpr               *esp;
};

typedef kbool_t (*CheckEndOfStmtFunc)(KonohaContext *, kArray *, int *currentIdxRef, int endIdx, int *indentRef, kArray *tokenList, int beginIdx);

typedef struct {
	ktype_t    ty;    ksymbol_t  fn;
} GammaStackDecl ;

#define kGamma_TOPLEVEL        (kshortflag_t)(1)
#define kGamma_isTOPLEVEL(GMA)  TFLAG_is(kshortflag_t, GMA->genv->flag, kGamma_TOPLEVEL)
#define kGamma_ERROR           (kshortflag_t)(1<<1)
#define kGamma_isERROR(GMA)    TFLAG_is(kshortflag_t, GMA->genv->flag, kGamma_ERROR)
#define kGamma_setERROR(GMA,B) TFLAG_set(kshortflag_t, GMA->genv->flag, kGamma_ERROR, B)

typedef struct {
	GammaStackDecl *varItems;
	size_t varsize;
	size_t capacity;
	size_t allocsize;  // set size if not allocated  (by default on stack)
} GammaStack ;

typedef struct  {
	kshortflag_t  flag;      kshortflag_t  cflag;
	ktype_t       this_cid;  ktype_t       static_cid;
	kMethod      *currentWorkingMethod;
	GammaStack    localScope;
	int           blockScopeShiftSize;
} GammaAllocaData;

struct kGammaVar {
	KonohaObjectHeader h;
	GammaAllocaData *genv;
};

/* ------------------------------------------------------------------------ */

#define KonohaContext_getSugarContext(kctx)    ((SugarContext*)kctx->modlocal[MOD_sugar])
#define kmodsugar  ((KModuleSugar*)kctx->modshare[MOD_sugar])
#define CT_Token    kmodsugar->cToken
#define CT_Expr     kmodsugar->cExpr
#define CT_Stmt     kmodsugar->cStmt
#define CT_Block    kmodsugar->cBlock
#define CT_NameSpace    kmodsugar->cNameSpace
#define CT_Gamma    kmodsugar->cGamma

#define CT_TokenVar    kmodsugar->cToken
#define CT_ExprVar     kmodsugar->cExpr
#define CT_StmtVar     kmodsugar->cStmt
#define CT_BlockVar    kmodsugar->cBlock
#define CT_NameSpaceVar    kmodsugar->cNameSpace
#define CT_GammaVar    kmodsugar->cGamma

#define CT_TokenArray           kmodsugar->cTokenArray
#define kTokenArray             kArray
#define CT_ExprArray            CT_Array
#define kExprArray              kArray
#define CT_StmtArray            CT_Array
#define kStmtArray              kArray

#define IS_NameSpace(O)  ((O)->h.ct == CT_NameSpace)
#define IS_Token(O)  ((O)->h.ct == CT_Token)
#define IS_Expr(O)   ((O)->h.ct == CT_Expr)
#define IS_Stmt(O)   ((O)->h.ct == CT_Stmt)
#define IS_Block(O)  ((O)->h.ct == CT_Block)
#define IS_Gamma(O)  ((O)->h.ct == CT_Gamma)

#define K_NULLTOKEN  (kToken*)((CT_Token)->defaultValueAsNull)
#define K_NULLEXPR   (kExpr*)((CT_Expr)->defaultValueAsNull)
#define K_NULLBLOCK  (kBlock*)((CT_Block)->defaultValueAsNull)


typedef struct {
	KonohaModule  h;
	KonohaClass *cToken;
	KonohaClass *cExpr;
	KonohaClass *cStmt;
	KonohaClass *cBlock;
	KonohaClass *cNameSpace;
	KonohaClass *cGamma;
	KonohaClass *cTokenArray;

	kArray          *packageList;
	KUtilsHashMap   *packageMapNO;

	kFunc *UndefinedParseExpr;
	kFunc *UndefinedStmtTyCheck;
	kFunc *UndefinedExprTyCheck;
	kFunc *ParseExpr_Term;
	kFunc *ParseExpr_Op;

	// export
	void   (*kNameSpace_setTokenizeFunc)(KonohaContext *, kNameSpace *, int ch, TokenizeFunc, kFunc *, int isAddition);
	void   (*kNameSpace_tokenize)(KonohaContext *, kNameSpace *, const char *, kfileline_t, kArray *);
	kbool_t (*kNameSpace_resolveTokenArray)(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, kArray *resolvedTokenList);
	int (*kStmt_parseTypePattern)(KonohaContext *, kStmt *, kNameSpace *, kArray *, int , int , KonohaClass **classRef);

	kExpr* (*kExpr_setConstValue)(KonohaContext *, kExpr *, ktype_t ty, kObject *o);
	kExpr* (*kExpr_setUnboxConstValue)(KonohaContext *, kExpr *, ktype_t ty, uintptr_t unboxValue);
	kExpr* (*kExpr_setVariable)(KonohaContext *, kExpr *, kGamma *, kexpr_t build, ktype_t ty, intptr_t index);

	uintptr_t   (*kStmt_parseFlag)(KonohaContext *kctx, kStmt *stmt, KonohaFlagSymbolData *flagData, uintptr_t flag);
	kToken*     (*kStmt_getToken)(KonohaContext *, kStmt *, ksymbol_t kw, kToken *def);
	kExpr*      (*kStmt_getExpr)(KonohaContext *, kStmt *, ksymbol_t kw, kExpr *def);
	const char* (*kStmt_getText)(KonohaContext *, kStmt *, ksymbol_t kw, const char *def);
	kBlock*     (*kStmt_getBlock)(KonohaContext *, kStmt *, ksymbol_t kw, kBlock *def);

	kbool_t     (*kBlock_tyCheckAll)(KonohaContext *, kBlock *, kGamma *);
	kbool_t     (*kStmt_tyCheckByName)(KonohaContext *, kStmt*, ksymbol_t, kGamma *, ktype_t, int);
	kExpr*      (*kStmt_tyCheckExprAt)(KonohaContext *, kStmt *, kExpr *, size_t, kGamma *, ktype_t, int);
	kExpr *     (*kStmt_tyCheckCallParamExpr)(KonohaContext *, kStmt *, kExpr *, kMethod *, kGamma *, ktype_t);
	kExpr *     (*new_TypedMethodCall)(KonohaContext *, kStmt *, ktype_t ty, kMethod *mtd, kGamma *, int n, ...);

	SugarSyntax* (*kNameSpace_getSyntax)(KonohaContext *, kNameSpace *, ksymbol_t, int);
	void       (*kNameSpace_defineSyntax)(KonohaContext *, kNameSpace *, KDEFINE_SYNTAX *);
	void       (*kNameSpace_setSugarFunc)(KonohaContext *, kNameSpace *, ksymbol_t kw, size_t idx, kFunc *);
	void       (*kNameSpace_addSugarFunc)(KonohaContext *, kNameSpace *, ksymbol_t kw, size_t idx, kFunc *);

	kbool_t    (*makeSyntaxRule)(KonohaContext *, kArray*, int, int, kArray *);
	kBlock*    (*new_Block)(KonohaContext *, kNameSpace *, kStmt *, kArray *, int, int, CheckEndOfStmtFunc);
	void       (*kBlock_insertAfter)(KonohaContext *, kBlock *, kStmt *target, kStmt *);

	kExpr*     (*kStmt_parseExpr)(KonohaContext *, kStmt *, kArray *tokenArray, int s, int e);
	kExpr*     (*kStmt_parseOperatorExpr)(KonohaContext *, kStmt *, kArray *tokenArray, int beginIdx, int operatorIdx, int endIdx);
	kExpr*     (*new_ConsExpr)(KonohaContext *, SugarSyntax *syn, int n, ...);
	kExpr *    (*kStmt_addExprParam)(KonohaContext *, kStmt *, kExpr *, kArray *tokenArray, int, int, int allowEmpty);
	kExpr *    (*kStmt_rightJoinExpr)(KonohaContext *, kStmt *, kExpr *, kArray *, int, int);

	void       (*Token_pERR)(KonohaContext *, kTokenVar *, const char *fmt, ...);
	kExpr *    (*Stmt_p)(KonohaContext *, kStmt *, kToken *, int pe, const char *fmt, ...);

} KModuleSugar;

#define EXPORT_SUGAR(base) \
	base->kNameSpace_setTokenizeFunc = kNameSpace_setTokenizeFunc;\
	base->kNameSpace_tokenize        = kNameSpace_tokenize;\
	base->kNameSpace_resolveTokenArray = kNameSpace_resolveTokenArray;\
	base->kStmt_parseTypePattern     = kStmt_parseTypePattern;\
	base->kStmt_parseFlag            = kStmt_parseFlag;\
	base->kStmt_getToken             = kStmt_getToken;\
	base->kStmt_getBlock             = kStmt_getBlock;\
	base->kStmt_getExpr              = kStmt_getExpr;\
	base->kStmt_getText              = kStmt_getText;\
	base->kExpr_setConstValue        = kExpr_setConstValue;\
	base->kExpr_setUnboxConstValue   = kExpr_setUnboxConstValue;\
	base->kExpr_setVariable          = kExpr_setVariable;\
	base->kStmt_tyCheckExprAt      = kStmt_tyCheckExprAt;\
	base->kStmt_tyCheckByName        = kStmt_tyCheckByName;\
	base->kBlock_tyCheckAll          = kBlock_tyCheckAll;\
	base->kStmt_tyCheckCallParamExpr = kStmt_tyCheckCallParamExpr;\
	base->new_TypedMethodCall        = new_TypedMethodCall;\
	/*syntax*/\
	base->kNameSpace_defineSyntax    = kNameSpace_defineSyntax;\
	base->kNameSpace_getSyntax       = kNameSpace_getSyntax;\
	base->makeSyntaxRule             = makeSyntaxRule;\
	base->kNameSpace_setSugarFunc    = kNameSpace_setSugarFunc;\
	base->kNameSpace_addSugarFunc    = kNameSpace_addSugarFunc;\
	/*ast*/\
	base->new_Block                  = new_Block;\
	base->kBlock_insertAfter         = kBlock_insertAfter;\
	base->kStmt_parseExpr            = kStmt_parseExpr;\
	base->kStmt_parseOperatorExpr    = kStmt_parseOperatorExpr;\
	base->new_ConsExpr               = new_ConsExpr;\
	base->kStmt_addExprParam         = kStmt_addExprParam;\
	base->kStmt_rightJoinExpr        = kStmt_rightJoinExpr;\
	/*perror*/\
	base->Token_pERR                 = Token_pERR;\
	base->Stmt_p                     = Stmt_p;\


typedef struct {
	KonohaContextModule h;
	kArray            *preparedTokenList;
	KUtilsGrowingArray errorMessageBuffer;
	kArray            *errorMessageList;
	int                errorMessageCount;
	kbool_t            isBlockingErrorMessage;
	kBlock            *singleBlock;
	kGamma            *gma;
//	kArray            *lvarlst;
	kArray            *definedMethodList;
} SugarContext;

#define TPOL_NOCHECK              1
#define TPOL_ALLOWVOID      (1 << 1)
#define TPOL_COERCION       (1 << 2)
#define TPOL_CONST          (1 << 4)

#define new_ConstValueExpr(CTX, T, O)              SUGAR kExpr_setConstValue(CTX, NULL, T, O)
#define new_UnboxConstValueExpr(CTX, T, D)         SUGAR kExpr_setUnboxConstValue(CTX, NULL, T, D)
#define new_VariableExpr(CTX, GMA, BLD, TY, IDX)   SUGAR kExpr_setVariable(CTX, NULL, GMA, BLD, TY, IDX)

#ifdef USING_SUGAR_AS_BUILTIN

#define SUGAR

static kExpr* kExpr_setConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, kObject *o);
static kExpr* kExpr_setUnboxConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, uintptr_t unboxValue);
static kExpr* kExpr_setVariable(KonohaContext *kctx, kExpr *expr, kGamma *gma, int build, ktype_t ty, intptr_t index);

#define TY_NameSpace                       kmodsugar->cNameSpace->typeId
#define TY_Token                           kmodsugar->cToken->typeId
#define TY_Stmt                            kmodsugar->cStmt->typeId
#define TY_Block                           kmodsugar->cBlock->typeId
#define TY_Expr                            kmodsugar->cExpr->typeId
#define TY_Gamma                           kmodsugar->cGamma->typeId
#define TY_TokenArray                      kmodsugar->cTokenArray->typeId

#define SYN_(KS, KW)                       kNameSpace_getSyntax(kctx, KS, KW, 0)


#else/*SUGAR_EXPORTS*/

#define SUGAR        ((const KModuleSugar*)kmodsugar)->
#define TY_NameSpace                         SUGAR cNameSpace->typeId
#define TY_Token                             SUGAR cToken->typeId
#define TY_Stmt                              SUGAR cStmt->typeId
#define TY_Block                             SUGAR cBlock->typeId
#define TY_Expr                              SUGAR cExpr->typeId
#define TY_Gamma                             SUGAR cGamma->typeId
#define TY_TokenArray                        SUGAR cTokenArray->typeId

//#define KW_(T)                               _e->keyword(kctx, T, sizeof(T)-1, SYM_NONAME)
#define SYN_(KS, KW)                         SUGAR kNameSpace_getSyntax(kctx, KS, KW, 0)
#define NEWSYN_(KS, KW)                      (SugarSyntaxVar*)(SUGAR kNameSpace_getSyntax(kctx, KS, KW, 1))

#endif/*SUGAR_EXPORTS*/

///* ------------------------------------------------------------------------ */

static inline void kToken_setTypeId(KonohaContext *kctx, kToken *tk, kNameSpace *ns, ktype_t type)
{
	((kTokenVar*)tk)->resolvedTypeId = type;
	((kTokenVar*)tk)->resolvedSyntaxInfo = kmodsugar->kNameSpace_getSyntax(kctx, ns, KW_TypePattern, 0);
}

#define Stmt_nameSpace(STMT)   kStmt_nameSpace(kctx, STMT)
static inline kNameSpace *kStmt_nameSpace(KonohaContext *kctx, kStmt *stmt)
{
	return stmt->parentBlockNULL->blockNameSpace;
}

#define kStmt_setsyn(STMT, S)  Stmt_setsyn(kctx, STMT, S)
#define kStmt_done(STMT)       Stmt_setsyn(kctx, STMT, NULL)
static inline void Stmt_setsyn(KonohaContext *kctx, kStmt *stmt, SugarSyntax *syn)
{
//	if(syn == NULL && stmt->syn != NULL) {
//		DBG_P("DONE: STMT='%s'", KW_t(syn->keyword));
//	}
	((kStmtVar*)stmt)->syn = syn;
}

#define kStmt_typed(STMT, T)  Stmt_typed(STMT, TSTMT_##T)
static inline void Stmt_typed(kStmt *stmt, int build)
{
	if(stmt->build != TSTMT_ERR) {
		((kStmtVar*)stmt)->build = build;
	}
}


static inline kbool_t Expr_isSymbolTerm(kExpr *expr)
{
	return (Expr_isTerm(expr) && (expr->termToken->resolvedSyntaxInfo->keyword == KW_SymbolPattern));
}

static inline void kExpr_setsyn(kExpr *expr, SugarSyntax *syn)
{
	((kExprVar*)expr)->syn = syn;
}

#define kExpr_typed(E, B, TY)   Expr_typed(E, TEXPR_##B, TY)
static inline kExpr *Expr_typed(kExpr *expr, int build, ktype_t ty)
{
	((kExprVar*)expr)->build = build;
	((kExprVar*)expr)->ty = ty;
	return expr;
}

#ifdef __cplusplus
}
#endif

/* ------------------------------------------------------------------------ */


#endif /* SUGAR_H_ */
