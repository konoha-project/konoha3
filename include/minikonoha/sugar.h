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

#ifdef __cplusplus
extern "C" {
#endif

#define KSymbol_END              ((ksymbol_t)-1)
#define KSymbol_ERR              (((ksymbol_t)0)|0) /**/
#define KSymbol_ExprPattern      (((ksymbol_t)1)|KSymbolAttr_Pattern) /*$Node*/
#define KSymbol_SymbolPattern    (((ksymbol_t)2)|KSymbolAttr_Pattern) /*$Symbol*/
#define KSymbol_TextPattern      (((ksymbol_t)3)|KSymbolAttr_Pattern) /*$Text*/
#define KSymbol_NumberPattern    (((ksymbol_t)4)|KSymbolAttr_Pattern) /*$Number*/
#define KSymbol_TypePattern      (((ksymbol_t)5)|KSymbolAttr_Pattern) /*$Type*/

#define KSymbol_ParenthesisGroup (((ksymbol_t)6)) /*()*/
#define KSymbol_BracketGroup     (((ksymbol_t)7)) /*[]*/
#define KSymbol_BraceGroup       (((ksymbol_t)8)) /*{}*/
#define KSymbol_TypeCastGroup    (((ksymbol_t)6)|KSymbolAttr_Pattern)    /*$()*/
#define KSymbol_TypeParamGroup   (((ksymbol_t)7)|KSymbolAttr_Pattern)    /*$[]*/
#define KSymbol_OptionalGroup    (((ksymbol_t)7)|KSymbol_ATMARK)         /*@[]*/
#define KSymbol_BlockPattern     (((ksymbol_t)9)|KSymbolAttr_Pattern)    /*$Node*/
#define KSymbol_ParamPattern     (((ksymbol_t)10)|KSymbolAttr_Pattern)   /*$Param*/
#define KSymbol_TypeDeclPattern  (((ksymbol_t)11)|KSymbolAttr_Pattern)   /*$TypeDecl*/
#define KSymbol_MethodDeclPattern  (((ksymbol_t)12)|KSymbolAttr_Pattern) /*$MethodDecl*/
#define KSymbol_TokenPattern     (((ksymbol_t)13)|KSymbolAttr_Pattern)   /*$Token*/

#define KSymbol_NodeOperator        KSymbol_ParamPattern
#define KSymbol_NodeTerm            KSymbol_SymbolPattern
#define KSymbol_NodeMethodCall      KSymbol_ParamPattern

#define KSymbol_DOT     14
#define KSymbol_DIV     (1+KSymbol_DOT)
#define KSymbol_MOD     (2+KSymbol_DOT)
#define KSymbol_MUL     (3+KSymbol_DOT)
#define KSymbol_ADD     (4+KSymbol_DOT)
#define KSymbol_SUB     (5+KSymbol_DOT)
#define KSymbol_LT      (6+KSymbol_DOT)
#define KSymbol_LTE     (7+KSymbol_DOT)
#define KSymbol_GT      (8+KSymbol_DOT)
#define KSymbol_GTE     (9+KSymbol_DOT)
#define KSymbol_EQ      (10+KSymbol_DOT)
#define KSymbol_NEQ     (11+KSymbol_DOT)
#define KSymbol_AND     (12+KSymbol_DOT)
#define KSymbol_OR      (13+KSymbol_DOT)
#define KSymbol_NOT     (14+KSymbol_DOT)
#define KSymbol_LET     (15+KSymbol_DOT)
#define KSymbol_COMMA   (16+KSymbol_DOT)
#define KSymbol_DOLLAR  KSymbolAttr_Pattern
#define KSymbol_ATMARK  KSymbolAttr_Annotation
#define KSymbol_COLON   (17+KSymbol_DOT)
#define KSymbol_SEMICOLON (18+KSymbol_DOT)

#define KSymbol_true      33
#define KSymbol_false     (1+KSymbol_true)
#define KSymbol_if        (2+KSymbol_true)
#define KSymbol_else      (3+KSymbol_true)
#define KSymbol_return    (4+KSymbol_true)

typedef enum {
	TokenType_INDENT = 1,
	TokenType_SYMBOL = KSymbol_SymbolPattern,
	TokenType_TEXT   = KSymbol_TextPattern,
	TokenType_INT    = KSymbol_NumberPattern,
	TokenType_CODE   = KSymbol_BlockPattern,
	TokenType_ERR    = KSymbol_TokenPattern
} kTokenType;

// reserved
//#define MN_new       (8+KSymbol_void)
#define FN_this      KFieldName_("this")

/* KonohaChar */

typedef enum {
	KonohaChar_Null                 =  0,
	KonohaChar_Undefined            =  1,
	KonohaChar_Digit                =  2,
	KonohaChar_UpperCaseAlphabet    =  3,
	KonohaChar_LowerCaseAlphabet    =  4,
	KonohaChar_Unicode              =  5,
	KonohaChar_NewLine              =  6,
	KonohaChar_Tab                  =  7,
	KonohaChar_Space                =  8,
	KonohaChar_OpenParenthesis      =  9,
	KonohaChar_CloseParenthesis     = 10,
	KonohaChar_OpenBracket          = 11,
	KonohaChar_CloseBracket         = 12,
	KonohaChar_OpenBrace            = 13,
	KonohaChar_CloseBrace           = 14,
	KonohaChar_LessThan             = 15,
	KonohaChar_GreaterThan          = 16,
	KonohaChar_Quote                = 17,
	KonohaChar_DoubleQuote          = 18,
	KonohaChar_BackQuote            = 19,
	KonohaChar_Surprised            = 20,
	KonohaChar_Sharp                = 21,
	KonohaChar_Dollar               = 22,
	KonohaChar_Percent              = 23,
	KonohaChar_And                  = 24,
	KonohaChar_Star                 = 25,
	KonohaChar_Plus                 = 26,
	KonohaChar_Comma                = 27,
	KonohaChar_Minus                = 28,
	KonohaChar_Dot                  = 29,
	KonohaChar_Slash                = 30,
	KonohaChar_Colon                = 31,
	KonohaChar_SemiColon            = 32,
	KonohaChar_Equal                = 33,
	KonohaChar_Question             = 34,
	KonohaChar_AtMark               = 35,
	KonohaChar_Var                  = 36,
	KonohaChar_Childer              = 37,
	KonohaChar_BackSlash            = 38,
	KonohaChar_Hat                  = 39,
	KonohaChar_UnderBar             = 40,
	KonohaChar_MAX                  = 41
} KonohaChar;

#ifdef USE_AsciiToKonohaChar
static const char cMatrix[128] = {
	0/*nul*/, 1/*soh*/, 1/*stx*/, 1/*etx*/, 1/*eot*/, 1/*enq*/, 1/*ack*/, 1/*bel*/,
	1/*bs*/,  KonohaChar_Tab/*ht*/, KonohaChar_NewLine/*nl*/, 1/*vt*/, 1/*np*/, 1/*cr*/, 1/*so*/, 1/*si*/,
	/*020 dle  021 dc1  022 dc2  023 dc3  024 dc4  025 nak  026 syn  027 etb */
	1, 1, 1, 1,     1, 1, 1, 1,
	/*030 can  031 em   032 sub  033 esc  034 fs   035 gs   036 rs   037 us */
	1, 1, 1, 1,     1, 1, 1, 1,
	/*040 sp   041  !   042  "   043  #   044  $   045  %   046  &   047  ' */
	KonohaChar_Space, KonohaChar_Surprised, KonohaChar_DoubleQuote, KonohaChar_Sharp, KonohaChar_Dollar, KonohaChar_Percent, KonohaChar_And, KonohaChar_Quote,
	/*050  (   051  )   052  *   053  +   054  ,   055  -   056  .   057  / */
	KonohaChar_OpenParenthesis, KonohaChar_CloseParenthesis, KonohaChar_Star, KonohaChar_Plus, KonohaChar_Comma, KonohaChar_Minus, KonohaChar_Dot, KonohaChar_Slash,
	/*060  0   061  1   062  2   063  3   064  4   065  5   066  6   067  7 */
	KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Digit,  KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Digit,
	/*070  8   071  9   072  :   073  ;   074  <   075  =   076  >   077  ? */
	KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Colon, KonohaChar_SemiColon, KonohaChar_LessThan, KonohaChar_Equal, KonohaChar_GreaterThan, KonohaChar_Question,
	/*100  @   101  A   102  B   103  C   104  D   105  E   106  F   107  G */
	KonohaChar_AtMark, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet,
	/*110  H   111  I   112  J   113  K   114  L   115  M   116  N   117  O */
	KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet,
	/*120  P   121  Q   122  R   123  S   124  T   125  U   126  V   127  W */
	KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet,
	/*130  X   131  Y   132  Z   133  [   134  \   135  ]   136  ^   137  _ */
	KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_OpenBracket, KonohaChar_BackSlash, KonohaChar_CloseBracket, KonohaChar_Hat, KonohaChar_UnderBar,
	/*140  `   141  a   142  b   143  c   144  d   145  e   146  f   147  g */
	KonohaChar_BackQuote, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet,
	/*150  h   151  i   152  j   153  k   154  l   155  m   156  n   157  o */
	KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet,
	/*160  p   161  q   162  r   163  s   164  t   165  u   166  v   167  w */
	KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet,
	/*170  x   171  y   172  z   173  {   174  |   175  }   176  ~   177 del*/
	KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_OpenBrace, KonohaChar_Var, KonohaChar_CloseBrace, KonohaChar_Childer, 1,
};

static int AsciiToKonohaChar(int ascii)
{
	return (ascii < 0) ? KonohaChar_Unicode : cMatrix[ascii];
}
#endif/*USE_AsciiToKonohaChar*/

#define KCHAR_MAX  KonohaChar_MAX
#define SIZEOF_TOKENMATRIX   (sizeof(void *) * KCHAR_MAX * 2)
typedef struct Tokenizer Tokenizer;
typedef int (*TokenizeFunc)(KonohaContext *, kTokenVar *, Tokenizer *, int);

struct Tokenizer {
	kNameSpace         *ns;
	const char         *source;
	size_t              sourceLength;
	kfileline_t         currentLine;
	kArray             *tokenList;
	int                 tabsize;
	const TokenizeFunc *cFuncItems;
	union {
		kFunc         **FuncItems;
		kArray        **funcListItems;
	};
	kString            *preparedString;
};

#define VAR_TRACE
#ifndef VAR_TRACE
#define VAR_TRACE DBG_P("tracing..")
#endif

// int TokenFunc(Token tk, Source s)
#define VAR_TokenFunc(TK, S)\
		kTokenVar *TK = (kTokenVar *)sfp[1].asObject;\
		kString *S = sfp[2].asString;\
		Tokenizer *tokenizer = (Tokenizer *)sfp[1].unboxValue;\
		int tok_start = (ksymbol_t)sfp[2].intValue;\
		VAR_TRACE; (void)TK; (void)S; (void)tok_start; (void)tokenizer;

// int Parse(Node stmt, Symbol name, Token[] toks, int s, int op, int e)
#define VAR_Parse(STMT, NAME, TLS, S, OP, E)\
		kNode *STMT = (kNode *)sfp[1].asObject;\
		ksymbol_t NAME = (ksymbol_t)sfp[2].intValue;\
		kArray *TLS = (kArray *)sfp[3].asObject;\
		int S = (int)sfp[4].intValue;\
		int OP = (int)sfp[5].intValue;\
		int E = (int)sfp[6].intValue;\
		VAR_TRACE; (void)STMT; (void)NAME; (void)TLS; (void)S; (void)OP; (void)E

#define VAR_PatternMatch(STMT, NAME, TLS, S, E)\
		kNode *STMT = (kNode *)sfp[1].asObject;\
		ksymbol_t NAME = (ksymbol_t)sfp[2].intValue;\
		kArray *TLS = (kArray *)sfp[3].asObject;\
		int S = (int)sfp[4].intValue;\
		int E = (int)sfp[6].intValue;\
		VAR_TRACE; (void)STMT; (void)NAME; (void)TLS; (void)S; (void)E

#define VAR_Expression(STMT, TLS, S, C, E)\
		KSyntax *syn = (KSyntax *)sfp[0].unboxValue;\
		kNode *STMT = (kNode *)sfp[1].asObject;\
		kArray *TLS = (kArray *)sfp[3].asObject;\
		int S = (int)sfp[4].intValue;\
		int C = (int)sfp[5].intValue;\
		int E = (int)sfp[6].intValue;\
		VAR_TRACE; (void)syn; (void)STMT; (void)TLS; (void)S; (void)C; (void)E

// Node TypeCheck(Node expr, Gamma gma, Object type)
#define VAR_TypeCheck(EXPR, GMA, TY) \
		kNode *EXPR = (kNode *)sfp[1].asObject;\
		kGamma *GMA = (kGamma *)sfp[2].asObject;\
		KClass* TY = kObject_class(sfp[3].asObject);\
		VAR_TRACE; (void)EXPR; (void)GMA; (void)TY

#define VAR_TypeCheck2(STMT, EXPR, GMA, TY) \
		kNode *EXPR = (kNode *)sfp[1].asObject;\
		kGamma *GMA = (kGamma *)sfp[2].asObject;\
		KClass* TY = kObject_class(sfp[3].asObject);\
		kNode *STMT = expr;\
		VAR_TRACE; (void)STMT; (void)EXPR; (void)GMA; (void)TY

typedef const struct KSyntaxVar   KSyntax;
typedef struct KSyntaxVar         KSyntaxVar;

//typedef enum {
//	SugarFunc_TokenFunc         = 0,
//	KSugarParseFunc      = 1,
//	KSugarParseFunc        = 2,
//	SugarFunc_TopLevelStatement = 3,
//	SugarFunc_Statement         = 4,
//	KSugarTypeCheckFunc         = 5,
//	SugarFunc_SIZE              = 6
//} SugerFunc;

typedef enum {
	SugarFunc_TokenFunc         = 0,
	KSugarParseFunc             = 1,
	KSugarTypeCheckFunc         = 2,
	SugarFunc_SIZE              = 3
} SugerFunc;

#define SYNFLAG_Macro               ((kshortflag_t)1)
#define SYNFLAG_CFunc               (SYNFLAG_CParseFunc|SYNFLAG_CTypeCheckFunc)
#define SYNFLAG_CParseFunc          ((kshortflag_t)1 << 3)
#define SYNFLAG_CTypeCheckFunc      ((kshortflag_t)1 << 4)

#define SYNFLAG_NodeLeftJoinOp2     ((kshortflag_t)1 << 1)
#define SYNFLAG_NodePostfixOp2      ((kshortflag_t)1 << 2)

#define SYNFLAG_NodeBreakExec       ((kshortflag_t)1 << 8)  /* return, throw */
#define SYNFLAG_NodeJumpAhead0      ((kshortflag_t)1 << 9)  /* continue */
#define SYNFLAG_NodeJumpSkip0       ((kshortflag_t)1 << 10)  /* break */

#define KSyntax_Is(P, o)       (KFlag_Is(uintptr_t,(o)->flag, SYNFLAG_##P))
#define KSyntax_Set(P,o,B)     KFlag_Set(uintptr_t,(o)->flag, SYNFLAG_##P, B)

struct KSyntaxVar {
	ksymbol_t  keyword;               kshortflag_t  flag;
	const struct KSyntaxVar          *parentSyntaxNULL;
	kArray                           *syntaxPatternListNULL;
	union {
		kFunc                        *sugarFuncTable[SugarFunc_SIZE];
		kArray                       *sugarFuncListTable[SugarFunc_SIZE];
	};
	kshort_t tokenKonohaChar;
	kshort_t precedence_op2;          kshort_t precedence_op1;
	kpackageId_t lastLoadedPackageId;
	kshort_t macroParamSize;
	kArray                            *macroDataNULL;
};

//#define PatternMatch_(NAME)       .PatternMatch   = PatternMatch_##NAME
//#define Expression_(NAME)         .Expression      = Expression_##NAME
//#define TopLevelStatement_(NAME)  .TopLevelStatement = Statement_##NAME
//#define Statement_(NAME)          .Statement    = Statement_##NAME
//#define TypeCheck_(NAME)          .TypeCheck    = TypeCheck_##NAME

//#define _OPLeft   .flag = (SYNFLAG_NodeLeftJoinOp2)

// operator prcedence

typedef enum {
	Precedence_CStyleCALL     = 100,  /*x(), x[], x.x x->x x++ */
	Precedence_CStylePREUNARY = 200,  /*++x, --x, sizeof x &x +x -x !x */
	Precedence_CStyleCAST     = 300,  /* (T)x */
	Precedence_CStyleMUL      = 400,  /* x * x, x / x, x % x*/
	Precedence_CStyleADD      = 500,  /* x + x, x - x */
	Precedence_CStyleSHIFT    = 600,  /* x << x, x >> x */
	Precedence_CStyleCOMPARE  = 700,
	Precedence_CStyleEQUALS   = 800,
	Precedence_CStyleBITAND   = 900,
	Precedence_CStyleBITXOR   = 1000,
	Precedence_CStyleBITOR    = 1100,
	Precedence_CStyleAND      = 1200,
	Precedence_CStyleOR       = 1300,
	Precedence_CStyleTRINARY  = 1400,  /* ? : */
	Precedence_CStyleASSIGN   = 1500,
	Precedence_CStyleCOMMA    = 1600,
	Precedence_CStyleStatementEnd    = 2000
} Precedence;

typedef struct KDEFINE_SYNTAX {
	ksymbol_t    keyword;
	kshortflag_t flag;
//	const char *rule;
	int precedence_op2;
	int precedence_op1;
	union {
		kFunc* parseFunc;
		KMethodFunc parseMethodFunc;
	};
	union {
		kFunc* typeCheckFunc;
		KMethodFunc typeCheckMethodFunc;
	};
} KDEFINE_SYNTAX;

#define KSugarFunc(ns, F)     new_(Func, KLIB new_kMethod(kctx, (ns)->NameSpaceConstList, 0, 0, 0, F), (ns)->NameSpaceConstList)

#define kToken_SetHintChar(tk, ch, ch2)           tk->hintChar = ((ch << 8) | ((char)ch2))
#define kToken_GetOpenHintChar(tk)                ((int)(tk->hintChar >> 8))
#define kToken_GetCloseHintChar(tk)               ((char)tk->hintChar)

/* Token */
struct kTokenVar {
	kObjectHeader h;
	union {
		kString *text;
		kArray  *subTokenList;
		kNode   *parsedNode;
	};
	kfileline_t     uline;
	KSyntax        *resolvedSyntaxInfo;
	union {
		ksymbol_t   unresolvedTokenType; // (resolvedSyntaxInfo == NULL)
		ksymbol_t   resolvedSymbol;      // symbol (resolvedSyntaxInfo != NULL)
		ktypeattr_t resolvedTypeId;      // typeid if KSymbol_TypePattern
	};
	union {
		kushort_t   indent;               // indent when kw == TokenType_INDENT
		kshort_t    hintChar;
		ksymbol_t   stmtEntryKey;         // pattern name for 'setting key in Node'
	};
};

#define kToken_IsIndent(T)  ((T)->unresolvedTokenType == TokenType_INDENT && (T)->resolvedSyntaxInfo == NULL)

#define kTokenFlag_StatementSeparator    kObjectFlag_Local1
#define kTokenFlag_MatchPreviousPattern  kObjectFlag_Local1
#define kTokenFlag_RequiredReformat      kObjectFlag_Local2
#define kTokenFlag_BeforeWhiteSpace      kObjectFlag_Local3

#define kToken_Is(P, o)      (KFlag_Is(uintptr_t,(o)->h.magicflag, kTokenFlag_##P))
#define kToken_Set(P,o,B)    KFlag_Set(uintptr_t,(o)->h.magicflag, kTokenFlag_##P, B)

typedef struct KMacroSet {
	int/*ksymbol_t*/          symbol;
	kArray                   *tokenList;
	int                       beginIdx;
	int                       endIdx;
} KMacroSet;

struct KTokenSeqSource {
	kToken *openToken;
	int     stopChar;
	kToken *foundErrorToken;
};

struct KTokenSeqTarget {
	int RemovingIndent;
	int ExpandingBraceGroup;
	KSyntax *syntaxSymbolPattern;
};

typedef struct KTokenSeq {
	kNameSpace *ns;
	kArray     *tokenList;
	int         beginIdx;
	int         endIdx;
	union {
		struct KTokenSeqSource SourceConfig;
		struct KTokenSeqTarget TargetPolicy;
	};
} KTokenSeq;


#define KTokenSeq_Push(kctx, tokens) \
	size_t _PopCheckIdx = kArray_size(tokens.tokenList);\
	tokens.beginIdx      = kArray_size(tokens.tokenList);\
	tokens.endIdx        = 0;\

#define KTokenSeq_Pop(kctx, tokens)   do {\
	KLIB kArray_Clear(kctx, tokens.tokenList, _PopCheckIdx);\
	DBG_ASSERT(_PopCheckIdx == kArray_size(tokens.tokenList));\
} while(0)

#define KTokenSeq_End(kctx, tokens)   tokens->endIdx = kArray_size(tokens->tokenList)

#define Token_isVirtualTypeLiteral(TK)     ((TK)->resolvedSyntaxInfo->keyword == KSymbol_TypePattern)
#define Token_typeLiteral(TK)              (TK)->resolvedTypeId

typedef kshort_t       knode_t;

#define KNodeList(OP) \
	OP(Done)\
	OP(Const)\
	OP(New)\
	OP(Null)\
	OP(UnboxConst)\
	OP(Local)\
	OP(Field)\
	OP(MethodCall)\
	OP(And)\
	OP(Or)\
	OP(Assign)\
	OP(Block)\
	OP(If)\
	OP(While)\
	OP(DoWhile)\
	OP(Return)\
	OP(Break)\
	OP(Continue)\
	OP(Try)\
	OP(Throw)\
	OP(Error)

#define DEFINE_KNode(NAME) KNode_##NAME,

typedef enum {
	KNodeList(DEFINE_KNode)
} KNode_;

struct kNodeVar {
	kObjectHeader h;
	union {
		struct kNodeVar   *Parent;   /* if parent is a NameSpace, it is a root node */
		kNameSpace        *RootNodeNameSpace;
	};
	union {
		kToken        *KeyOperatorToken;     // Node
		kToken        *TermToken;            // Term
	};
	union {
		kArray        *NodeList;      // Node
		kNameSpace    *StmtNameSpace;  // Statement
	};
	union {
		KSyntax       *syn;  /* untyped */
		uintptr_t      unboxConstValue;
		intptr_t       index;
		kObject*       ObjectConstValue;
		intptr_t       stacktop;
		kString       *ErrorMessage;
	};
	knode_t node; 	   ktypeattr_t attrTypeId;
};

#define kNode_uline(O)   (O)->KeyOperatorToken->uline

#define kNode_IsRootNode(O)       IS_NameSpace(O->RootNodeNameSpace)
#define kNode_ns(O)                kNode_GetNameSpace(kctx, O)
static inline kNameSpace *kNode_GetNameSpace(KonohaContext *kctx, kNode *node)
{
	kNameSpace *ns = node->StmtNameSpace;
	while(!IS_NameSpace(ns)) {
		if(kNode_IsRootNode(node)) {
			ns = node->RootNodeNameSpace;
			break;
		}
		node = node->Parent;
		ns = node->StmtNameSpace;
	}
	return ns;
}

static inline kNode *kNode_Type(KonohaContext *kctx, kNode *node, knode_t nodeType, ktypeattr_t attrTypeId)
{
	kNodeVar *vnode = (kNodeVar *)node;
	vnode->node = nodeType;
	vnode->attrTypeId = attrTypeId;
	return node;
}

#define kNode_IsTerm(N)           IS_Token((N)->TermToken)

#define kNode_IsConstValue(o)     (KNode_Const <= (o)->node && (o)->node <= KNode_UnboxConst)

#define kNodeFlag_StackIndex         kObjectFlag_Local1
#define kNodeFlag_ObjectConst        kObjectFlag_Local2

#define kNodeFlag_OpenBlock          kObjectFlag_Local3  /* KNode_Block */
#define kNodeFlag_CatchContinue      kObjectFlag_Local3
#define kNodeFlag_CatchBreak         kObjectFlag_Local4

#define kNode_Is(P, O)      (KFlag_Is(uintptr_t,(O)->h.magicflag, kNodeFlag_##P))
#define kNode_Set(P, O, B)   KFlag_Set(uintptr_t,(O)->h.magicflag, kNodeFlag_##P, B)

#define kNode_At(E, N)            ((E)->NodeList->NodeItems[(N)])
#define kNode_IsERR(STMT)         ((STMT)->node == KNode_Error)

#define kNode_GetObjectNULL(CTX, O, K)            (KLIB kObject_getObject(CTX, UPCAST(O), K, NULL))
#define kNode_GetObject(CTX, O, K, DEF)           (KLIB kObject_getObject(CTX, UPCAST(O), K, DEF))
#define kNode_SetObject(CTX, O, K, V)             KLIB kObjectProto_SetObject(CTX, UPCAST(O), K, kObject_typeId(V), UPCAST(V))
#define kNode_SetUnboxValue(CTX, O, K, T, V)      KLIB kObjectProto_SetUnboxValue(CTX, UPCAST(O), K, T, V)
#define kNode_RemoveKey(CTX, O, K)                KLIB kObjectProto_RemoveKey(CTX, UPCAST(O), K)
#define kNode_DoEach(CTX, O, THUNK, F)            kObjectProto_DoEach(CTX, UPCAST(O), THUNK, F)

#define kNode_Message(kctx, STMT, PE, FMT, ...)            SUGAR kNode_Message2(kctx, STMT, NULL, PE, FMT, ## __VA_ARGS__)
#define kNodeToken_Message(kctx, STMT, TK, PE, FMT, ...)   SUGAR kNode_Message2(kctx, STMT, TK, PE, FMT, ## __VA_ARGS__)
#define kNodeNode_Message(kctx, STMT, EXPR, PE, FMT, ...)  SUGAR kNode_Message2(kctx, STMT, (kToken *)EXPR, PE, FMT, ## __VA_ARGS__)


typedef struct {
	ktypeattr_t    attrTypeId;    ksymbol_t  name;
} KGammaStackDecl;

#define kGamma_TopLevel        (kshortflag_t)(1)
#define Gamma_isTopLevel(GMA)  KFlag_Is(kshortflag_t, GMA->genv->flag, kGamma_TopLevel)
#define kGamma_ERROR           (kshortflag_t)(1<<1)
#define Gamma_hasERROR(GMA)    KFlag_Is(kshortflag_t, GMA->genv->flag, kGamma_ERROR)
#define Gamma_setERROR(GMA,B) KFlag_Set(kshortflag_t, GMA->genv->flag, kGamma_ERROR, B)

typedef struct {
	KGammaStackDecl *varItems;
	size_t varsize;
	size_t capacity;
	size_t allocsize;  // set size if not allocated  (by default on stack)
} KGammaStack;

typedef struct  {
	kshortflag_t  flag;      kshortflag_t  cflag;
	KClass   *thisClass;
	//ktypeattr_t      static_cid;
	kMethod      *currentWorkingMethod;
	KGammaStack    localScope;
	int           blockScopeShiftSize;
} KGammaAllocaData;

struct kGammaVar {
	kObjectHeader h;
	KGammaAllocaData *genv;
};


/* ------------------------------------------------------------------------ */

#define KGetParserContext(kctx)    ((KParserContext *)kctx->modlocal[MOD_sugar])
#define KPARSERM       ((KParserModule *)kctx->modshare[MOD_sugar])
#define KClass_Symbol       KPARSERM->cSymbol
#define KClass_SymbolVar       KPARSERM->cSymbol
#define KClass_Token        KPARSERM->cToken
#define KClass_TokenVar        KPARSERM->cToken
#define KClass_Node         KPARSERM->cNode
#define KClass_NodeVar      KPARSERM->cNode
#define KClass_Gamma        KPARSERM->cGamma
#define KClass_GammaVar        KPARSERM->cGamma


#define KClass_TokenArray           KPARSERM->cTokenArray
#define kTokenArray             kArray
#define KClass_NodeArray            KClass_Array
#define kNodeArray              kArray
#define KClass_NodeArray            KClass_Array
#define kNodeArray              kArray

#define IS_Token(O)  (kObject_class(O) == KClass_Token)
#define IS_Node(O)   (kObject_class(O) == KClass_Node)
#define IS_Gamma(O)  (kObject_class(O) == KClass_Gamma)

#define K_NULLTOKEN  (kToken *)((KClass_Token)->defaultNullValue)
#define K_NULLNODE   (kNode *)((KClass_Node)->defaultNullValue)
#define K_NULLBLOCK  (kNode *)((KClass_Node)->defaultNullValue)

typedef kNode* (*KTypeDeclFunc)(KonohaContext *kctx, kNode *stmt, kGamma *gma, ktypeattr_t ty, kNode *termNode, kNode *vexpr, kObject *thunk);

struct KBuilder;

typedef struct {
	KRuntimeModule  h;
	KClass *cSymbol;
	KClass *cToken;
	KClass *cNode;
	KClass *cGamma;
	KClass *cTokenArray;

	KSyntax*    (*kNameSpace_GetSyntax)(KonohaContext *, kNameSpace *, ksymbol_t, int);
	void            (*kNameSpace_DefineSyntax)(KonohaContext *, kNameSpace *, KDEFINE_SYNTAX *, KTraceInfo *);
	kbool_t         (*kArray_AddSyntaxRule)(KonohaContext *, kArray *ruleList, KTokenSeq *sourceRange);
	KSyntaxVar* (*kNameSpace_SetTokenFunc)(KonohaContext *, kNameSpace *, ksymbol_t, int ch, kFunc *);
	KSyntaxVar* (*kNameSpace_AddSugarFunc)(KonohaContext *, kNameSpace *, ksymbol_t kw, size_t idx, kFunc *);
	kbool_t         (*kNameSpace_SetMacroData)(KonohaContext *, kNameSpace *, ksymbol_t, int, const char *, int optionMacro);

	void        (*KTokenSeq_Tokenize)(KonohaContext *, KTokenSeq *, const char *, kfileline_t);
	kbool_t     (*KTokenSeq_ApplyMacro)(KonohaContext *, KTokenSeq *, kArray *, int, int, size_t, KMacroSet *);
	int         (*KTokenSeq_Preprocess)(KonohaContext *, KTokenSeq *, KMacroSet *, KTokenSeq *, int);
	kstatus_t   (*KTokenSeq_Eval)(KonohaContext *, KTokenSeq *, KTraceInfo *);

	int         (*ParseTypePattern)(KonohaContext *, kNameSpace *, kArray *, int , int , KClass **classRef);
	kTokenVar*  (*kToken_ToBraceGroup)(KonohaContext *, kTokenVar *, kNameSpace *, KMacroSet *);

	void        (*kNode_AddParsedObject)(KonohaContext *, kNode *, ksymbol_t, kObject *o);
	int         (*FindEndOfStatement)(KonohaContext *, kNameSpace *, kArray *, int, int);

	uintptr_t   (*kNode_ParseFlag)(KonohaContext *kctx, kNode *stmt, KFlagSymbolData *flagData, uintptr_t flag);
	kToken*     (*kNode_GetToken)(KonohaContext *, kNode *, ksymbol_t kw, kToken *def);
	kNode*      (*kNode_GetNode)(KonohaContext *, kNode *, ksymbol_t kw, kNode *def);
	const char* (*kNode_GetText)(KonohaContext *, kNode *, ksymbol_t kw, const char *def);
	kNode*      (*kNode_GetBlock)(KonohaContext *, kNode *, kNameSpace *, ksymbol_t kw, kNode *def);

	kNode*      (*new_BlockNode)(KonohaContext *, kNode *, KMacroSet *, KTokenSeq *);
//	kNodeVar*    (*new_BlockNode)(KonohaContext *kctx, kArray *gcstack, KSyntax *syn, ...);
	void         (*kNode_InsertAfter)(KonohaContext *, kNode *, kNode *target, kNode *);

	kNode*       (*kNode_Termnize)(KonohaContext *, kNode *, kToken *);
	kNodeVar*    (*new_UntypedOperatorNode)(KonohaContext *, KSyntax *syn, int n, ...);
	kNode*       (*kNode_ParseOperatorNode)(KonohaContext *, kNode *, KSyntax *, kArray *tokenList, int beginIdx, int operatorIdx, int endIdx);
	kNode*       (*ParseNewNode)(KonohaContext *, kNameSpace *, kArray *tokenList, int s, int e, int option, const char *hintBeforeText);
	kNode*       (*AddParamNode)(KonohaContext *, kNameSpace *, kNode *, kArray *tokenList, int, int, const char *hintBeforeText);
	kNode*       (*kNode_RightJoinNode)(KonohaContext *, kNode *, kNode *, kArray *, int, int);

	kNode*       (*kNode_SetConstValue)(KonohaContext *, kNodeVar *, KClass *, kObject *o);
	kNode*       (*kNode_SetUnboxConstValue)(KonohaContext *, kNodeVar *, ktypeattr_t, uintptr_t unboxValue);
	kNode*       (*kNode_SetVariable)(KonohaContext *, kNodeVar *, kGamma *, knode_t build, ktypeattr_t, intptr_t index);
	kNode *      (*new_TypedCallNode)(KonohaContext *, kNode *, kGamma *, KClass *, kMethod *mtd, int n, ...);

	kbool_t     (*TypeCheckBlock)(KonohaContext *, kNode *, kGamma *);
	kbool_t     (*kNode_TypeCheckByName)(KonohaContext *, kNode*, ksymbol_t, kGamma *, KClass *, int);
	kNode*      (*kNode_TypeCheckNodeAt)(KonohaContext *, kNode *, kNode *, size_t, kGamma *, KClass *, int);
	kNode *     (*TypeCheckCallParam)(KonohaContext *, kNode *, kNodeVar *, kMethod *, kGamma *, KClass *);
	int         (*kGamma_AddLocalVariable)(KonohaContext *, kGamma *, ktypeattr_t, ksymbol_t);
	kbool_t     (*kNode_DeclType)(KonohaContext *, kNode *, kGamma *, ktypeattr_t, kNode *, kObject *, KTypeDeclFunc, kNode **);
	kNode*      (*kNode_TypeCheckVariableNULL)(KonohaContext *, kNode *, kNodeVar *, kGamma *, KClass *);

	void       (*kToken_ToError)(KonohaContext *, kTokenVar *, kinfotag_t, const char *fmt, ...);
	kNode *    (*kNode_Message2)(KonohaContext *, kNode *, kToken *, kinfotag_t, const char *fmt, ...);

	kbool_t (*VisitNode)(KonohaContext *, struct KBuilder *, kNode *block);
//	kbool_t (*VisitNode)(KonohaContext *, struct KBuilder *, kNode *stmt);
//	void    (*VisitNode)(KonohaContext *, struct KBuilder *, kNode *stmt, kNode *expr);

	void (*dumpToken)(KonohaContext *kctx, kToken *tk, int n);
	void (*dumpTokenArray)(KonohaContext *kctx, int nest, kArray *a, int s, int e);
//	void (*dumpNode)(KonohaContext *kctx, int n, int nest, kNode *expr);
	void (*dumpNode)(KonohaContext *kctx, kNode *stmt);

} KParserModule;

typedef struct {
	KContextModule     h;
	kArray            *preparedTokenList;
	KGrowingArray      errorMessageBuffer;
	kArray            *errorMessageList;
	int                errorMessageCount;
	kbool_t            isNodeedErrorMessage;
	kGamma            *preparedGamma;
	kArray            *definedMethodList;
} KParserContext;

#define KClass_INFER    KClass_(KType_var)

typedef enum {
	TypeCheckPolicy_NOPOLICY              = 0,
	TypeCheckPolicy_NOCHECK               = 1,
	TypeCheckPolicy_ALLOWVOID      = (1 << 1),
	TypeCheckPolicy_COERCION       = (1 << 2),
	TypeCheckPolicy_CONST          = (1 << 4)
} TypeCheckPolicy;

#define new_ConstValueNode(CTX, T, O)              SUGAR kNode_SetConstValue(CTX, NULL, T, O)
#define new_UnboxConstValueNode(CTX, T, D)         SUGAR kNode_SetUnboxConstValue(CTX, NULL, T, D)
#define new_VariableNode(CTX, GMA, BLD, TY, IDX)   SUGAR kNode_SetVariable(CTX, NULL, GMA, BLD, TY, IDX)

#ifdef USING_SUGAR_AS_BUILTIN

#define SUGAR

//static kNode* kNode_SetConstValue(KonohaContext *kctx, kNodeVar *expr, ktypeattr_t ty, kObject *o);
//static kNode* kNode_SetUnboxConstValue(KonohaContext *kctx, kNodeVar *expr, ktypeattr_t ty, uintptr_t unboxValue);
//static kNode* kNode_SetVariable(KonohaContext *kctx, kNode *expr, kGamma *gma, knode_t build, ktypeattr_t ty, intptr_t index);

#define KType_Symbol                          KPARSERM->cSymbol->typeId
#define KType_Token                           KPARSERM->cToken->typeId
#define KType_Node                            KPARSERM->cNode->typeId
#define KType_Node                           KPARSERM->cNode->typeId
#define KType_Node                            KPARSERM->cNode->typeId
#define KType_Gamma                           KPARSERM->cGamma->typeId
#define KType_TokenArray                      KPARSERM->cTokenArray->typeId

#define KSyntax_(KS, KW)                       kNameSpace_GetSyntax(kctx, KS, KW, 0)


#else/*SUGAR_EXPORTS*/

#define SUGAR        ((const KParserModule *)KPARSERM)->
#define KType_Symbol                            SUGAR cSymbol->typeId
#define KType_Token                             SUGAR cToken->typeId
#ifdef USE_NODE
#define KType_Node                              SUGAR cNode->typeId
#define KType_Node                             SUGAR cNode->typeId
#define KType_Node                              SUGAR cNode->typeId
#else
#define KType_Node                              SUGAR cNode->typeId
#define KType_Node                             SUGAR cNode->typeId
#define KType_Node                              SUGAR cNode->typeId
#endif
#define KType_Gamma                             SUGAR cGamma->typeId
#define KType_TokenArray                        SUGAR cTokenArray->typeId

//#define KSymbol_(T)                               _e->keyword(kctx, T, sizeof(T)-1, KSymbol_Noname)
#define KSyntax_(KS, KW)                        SUGAR kNameSpace_GetSyntax(kctx, KS, KW, 0)
#define NEWKSyntax_(KS, KW)                     (KSyntaxVar *)(SUGAR kNameSpace_GetSyntax(kctx, KS, KW, 1))

#endif/*SUGAR_EXPORTS*/

#ifdef USE_SMALLBUILD
#define KdumpToken(ctx, tk)
#define KdumpTokenArray(CTX, TLS, S, E)
#define KdumpKTokenSeq(CTX, MSG, R)
#define KdumpNode(CTX, EXPR)
#else
#define KDump(O)                         KLIB DumpObject(kctx, (kObject *)O, __FILE__, __FUNCTION__, __LINE__)
#define KdumpToken(ctx, tk)              ((const KParserModule *)KPARSERM)->dumpToken(ctx, tk, 0)
#define KdumpTokenArray(CTX, TLS, S, E)  DBG_P("@"); ((const KParserModule *)KPARSERM)->dumpTokenArray(CTX, 1, TLS, S, E)
#define KdumpKTokenSeq(CTX, MSG, R)     DBG_P(MSG); ((const KParserModule *)KPARSERM)->dumpTokenArray(CTX, 1, R->tokenList, R->beginIdx, R->endIdx)
#define KdumpNode(CTX, EXPR)             ((const KParserModule *)KPARSERM)->dumpNode(CTX, 0, 0, EXPR)
#endif

/* ------------------------------------------------------------------------ */
/* BuilderAPI */

struct KBuilder;
typedef struct KBuilder KBuilder;

typedef kbool_t (*KNodeVisitFunc)(KonohaContext *kctx, KBuilder *builder, kNode *stmt);
//typedef void (*KNodeVisitFunc)(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr);

struct KBuilderCommon {
	struct KBuilderAPI2* api;
	int option;
	kfileline_t uline;
	int a; /* whatis a ? */
	int shift;
	int espidx;
};

#define DefineVisitFunc(NAME) KNodeVisitFunc visit##NAME##Node;

struct KBuilderAPI2 {
	const char *target;
	struct KVirtualCode*   (*GenerateKVirtualCode)(KonohaContext *, kMethod *mtd, kNode *block, int option);
	KMethodFunc            (*GenerateKMethodFunc)(KonohaContext *, struct KVirtualCode *);
	struct KVirtualCode *  (*RunVirtualMachine)(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc);
	KNodeList(DefineVisitFunc)
	size_t allocSize;
};

/* ------------------------------------------------------------------------ */

static inline void kToken_SetTypeId(KonohaContext *kctx, kToken *tk, kNameSpace *ns, ktypeattr_t type)
{
	((kTokenVar *)tk)->resolvedTypeId = type;
	((kTokenVar *)tk)->resolvedSyntaxInfo = KPARSERM->kNameSpace_GetSyntax(kctx, ns, KSymbol_TypePattern, 0);
}

#define kNode_isSymbolTerm(expr)   1

#ifdef __cplusplus
}
#endif

/* ------------------------------------------------------------------------ */
#endif /* SUGAR_H_ */
