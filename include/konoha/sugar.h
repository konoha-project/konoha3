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

// reserved
//#define MN_new     (8+KSymbol_void)
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
	int                 baseIndent;
	int                 currentIndent;
	const TokenizeFunc *cFuncItems;
	union {
		kFunc         **FuncItems;
		kArray        **funcListItems;
	};
	kString            *preparedString;
};

//#define VAR_TRACE
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

// int ReplaceFunc(kNameSpace *ns, Token[] tokenList, int s, int op, int e,Token[] bufferList)
#define VAR_ReplaceFunc(NS, TLS, S, OP, E, BUF)\
		kNameSpace *NS = sfp[1].asNameSpace;\
		kArray *TLS = (kArray *)sfp[2].asObject;\
		int S = (int)sfp[3].intValue;\
		int OP = (int)sfp[4].intValue;\
		int E = (int)sfp[5].intValue;\
		kArray *BUF = (kArray *)sfp[6]].asObject;\
		VAR_TRACE; (void)NS; (void)TLS; (void)S; (void)OP; (void)E; (void)BUF;


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
		kSyntax *syn = (kSyntax *)sfp[0].unboxValue;\
		kNode *STMT = (kNode *)sfp[1].asObject;\
		kArray *TLS = (kArray *)sfp[3].asObject;\
		int S = (int)sfp[4].intValue;\
		int C = (int)sfp[5].intValue;\
		int E = (int)sfp[6].intValue;\
		VAR_TRACE; (void)syn; (void)STMT; (void)TLS; (void)S; (void)C; (void)E

// Node TypeCheck(Node expr, Gamma ns, Object type)
#define VAR_TypeCheck(EXPR, GMA, TY) \
		kNode *EXPR = (kNode *)sfp[1].asObject;\
		kNameSpace *GMA = kNode_ns(EXPR);\
		KClass* TY = kObject_class(sfp[3].asObject);\
		VAR_TRACE; (void)EXPR; (void)GMA; (void)TY

#define VAR_TypeCheck2(STMT, EXPR, GMA, TY) \
		kNode *EXPR = (kNode *)sfp[1].asObject;\
		kNameSpace *GMA = kNode_ns(EXPR);\
		KClass* TY = kObject_class(sfp[3].asObject);\
		kNode *STMT = expr;\
		VAR_TRACE; (void)STMT; (void)EXPR; (void)GMA; (void)TY

typedef const struct kSyntaxVar   kSyntax;
typedef struct kSyntaxVar         kSyntaxVar;

typedef enum {
	KSugarTokenFunc         = 0,
	KSugarParseFunc         = 1,
	KSugarTypeFunc          = 2,
	SugarFunc_SIZE          = 3
} SugerFunc;

#define SUGARFUNC   (kFunc *)

#define SYNFLAG_Macro               ((kshortflag_t)1)
#define SYNFLAG_MetaPattern         ((kshortflag_t)1 << 1)
#define SYNFLAG_NodeLeftJoinOp2     ((kshortflag_t)1 << 2)
#define SYNFLAG_Suffix              ((kshortflag_t)1 << 3)
#define SYNFLAG_TypeSuffix          ((kshortflag_t)1 << 4)

#define SYNFLAG_NodeBreakExec       ((kshortflag_t)1 << 6)  /* return, throw */
#define SYNFLAG_NodeJumpAhead0      ((kshortflag_t)1 << 7)  /* continue */
#define SYNFLAG_NodeJumpSkip0       ((kshortflag_t)1 << 8)  /* break */

#define SYNFLAG_CFunc               (SYNFLAG_CParseFunc|SYNFLAG_CTypeFunc|SYNFLAG_CTokenFunc)
#define SYNFLAG_CParseFunc          ((kshortflag_t)1 << 10)
#define SYNFLAG_CTypeFunc           ((kshortflag_t)1 << 11)
#define SYNFLAG_CTokenFunc          ((kshortflag_t)1 << 12)

#define SYNFLAG_CallNode            ((kshortflag_t)1 << 13)

#define kSyntax_Is(P, o)       (KFlag_Is(kshortflag_t,(o)->flag, SYNFLAG_##P))
#define kSyntax_Set(P,o,B)     KFlag_Set(kshortflag_t,(o)->flag, SYNFLAG_##P, B)

struct kSyntaxVar {
	kObjectHeader h;
	kNameSpace                       *packageNameSpace;
	ksymbol_t  keyword;               kshortflag_t  flag;
	kArray                           *syntaxPatternListNULL;
	kArray                           *macroDataNULL;
	kFunc                            *TokenFuncNULL;
	kFunc                            *ParseFuncNULL;
	kFunc                            *TypeFuncNULL;
	kFunc                            *ReplaceFuncNULL;
	kshort_t tokenKonohaChar;         kshort_t macroParamSize;
	kshort_t precedence_op2;          kshort_t precedence_op1;
};

// operator prcedence
// http://ja.cppreference.com/w/cpp/language/operator_precedence

typedef enum {
	Precedence_CPPStyleScope  =  50,
	Precedence_CStyleSuffixCall     = 100,  /*x(), x[], x.x x->x x++ */
	Precedence_CStylePrefixOperator = 200,  /*++x, --x, sizeof x &x +x -x !x (T)x  */
//	Precedence_CppMember      = 300,  /* .x ->x */
	Precedence_CStyleMUL      = 400,  /* x * x, x / x, x % x*/
	Precedence_CStyleADD      = 500,  /* x + x, x - x */
	Precedence_CStyleSHIFT    = 600,  /* x << x, x >> x */
	Precedence_CStyleCOMPARE  = 700,
	Precedence_CStyleEquals   = 800,
	Precedence_CStyleBITAND   = 900,
	Precedence_CStyleBITXOR   = 1000,
	Precedence_CStyleBITOR    = 1100,
	Precedence_CStyleAND      = 1200,
	Precedence_CStyleOR       = 1300,
	Precedence_CStyleTRINARY  = 1400,  /* ? : */
	Precedence_CStyleAssign   = 1500,
	Precedence_CStyleCOMMA    = 1600,
	Precedence_Statement      = 1900,
	Precedence_CStyleStatementEnd    = 2000
} Precedence;

typedef struct KDEFINE_SYNTAX {
	ksymbol_t    keyword;
	kshortflag_t flag;
	int precedence_op2;
	int precedence_op1;
	union {
		kFunc* parseFunc;
		KMethodFunc parseMethodFunc;
	};
	union {
		kFunc* typeFunc;
		KMethodFunc typeMethodFunc;
	};
	int tokenChar;
	union {
		kFunc* tokenFunc;
		KMethodFunc tokenMethodFunc;
	};
} KDEFINE_SYNTAX;

#define KSugarFunc(ns, F)     new_(Func, KLIB new_kMethod(kctx, (ns)->NameSpaceConstList, 0, 0, 0, F), (ns)->NameSpaceConstList)

/* Token */

struct kTokenVar {
	kObjectHeader h;
	kfileline_t     uline;
	union {
		kString *text;
		kArray  *GroupTokenList;
		kNode   *parsedNode;
	};
	union {
		ksymbol_t   tokenType;           // (resolvedSyntaxInfo == NULL)
//		ksymbol_t   symbol;      // symbol (resolvedSyntaxInfo != NULL)
	};
	union {
		kushort_t   indent;               // indent when kw == TokenType_Indent
		kushort_t   openCloseChar;
	};
	ksymbol_t   symbol;
	union {
		ktypeattr_t resolvedTypeId;      // typeid if KSymbol_TypePattern
		ksymbol_t   ruleNameSymbol;      // pattern rule
	};
	kSyntax   *resolvedSyntaxInfo;
};

#define kToken_SetOpenCloseChar(tk, ch, ch2)           tk->openCloseChar = ((ch << 8) | ((char)ch2))
#define kToken_GetOpenChar(tk)                         ((int)(tk->openCloseChar >> 8))
#define kToken_GetCloseChar(tk)                        ((char)tk->openCloseChar)

typedef enum {
	TokenType_Skip      = 0,
	TokenType_Indent    = KSymbol_IndentPattern,
	TokenType_Symbol    = KSymbol_SymbolPattern,
	TokenType_Text      = KSymbol_TextPattern,
	TokenType_Number    = KSymbol_NumberPattern,
	TokenType_Member    = KSymbol_MemberPattern,
	TokenType_LazyBlock = KSymbol_BraceGroup,
	TokenType_Error     = KSymbol_TokenPattern
} kTokenType;

#define kToken_IsIndent(T)  ((T)->tokenType == TokenType_Indent)
#define kToken_IsStatementSeparator(T)  ((T)->resolvedSyntaxInfo->precedence_op2 == Precedence_CStyleStatementEnd)

#define kTokenFlag_BeforeWhiteSpace      kObjectFlag_Local1
#define kTokenFlag_MatchPreviousPattern  kObjectFlag_Local2
#define kTokenFlag_RequiredReformat      kObjectFlag_Local2
#define kTokenFlag_OpenGroup             kObjectFlag_Local3/*reserved*/
#define kTokenFlag_CloseGroup            kObjectFlag_Local4/*reserved*/
//#define kTokenFlag_StatementSeparator    kObjectFlag_Local4/*obsolete*/

#define kToken_Is(P, o)      (KFlag_Is(uintptr_t,(o)->h.magicflag, kTokenFlag_##P))
#define kToken_Set(P,o,B)    KFlag_Set(uintptr_t,(o)->h.magicflag, kTokenFlag_##P, B)

typedef struct KMacroSet {
	ksymbol_t  symbol;
	kArray    *tokenList;
	int        beginIdx;
	int        endIdx;
} KMacroSet;

struct KTokenSeqSource {
	kToken *openToken;
	int     stopChar;
	kToken *foundErrorToken;
};

struct KTokenSeqTarget {
	int RemovingIndent;
	int ExpandingBraceGroup;
	kSyntax *syntaxSymbolPattern;
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

#define KTokenSeq_End(kctx, T)   T.endIdx = kArray_size(T.tokenList)

#define RangeGroup(A)                 A, 1, (kArray_size(A)-1)
#define RangeArray(A)                 A, 0, kArray_size(A)
#define RangeTokenSeq(T)              T.tokenList, T.beginIdx, T.endIdx

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
	OP(Box)\
	OP(Push)\
	OP(MethodCall)\
	OP(And)\
	OP(Or)\
	OP(Assign)\
	OP(Block)\
	OP(If)\
	OP(While)\
	OP(DoWhile)\
	OP(For)\
	OP(Return)\
	OP(Break)\
	OP(Continue)\
	OP(Try)\
	OP(Throw)\
	OP(Function)\
	OP(Error)

#define DEFINE_KNode(NAME) KNode_##NAME,

typedef enum KNode_Type {
	KNodeList(DEFINE_KNode)
	KNode_MAX
} KNode_;

#define kNode_node(o)             (kObject_HashCode(o))
#define kNode_setnode(o, node)    kObject_SetHashCode(o, (node))
#define kNode_IsConstValue(o)     (KNode_Const <= kNode_node(o) && kNode_node(o) <= KNode_UnboxConst)
#define kNode_IsValue(o)          (KNode_Const <= kNode_node(o) && kNode_node(o) <= KNode_Field)

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
		kArray          *NodeList;       // Node
		kNameSpace      *StmtNameSpace;  // Statement
		struct kNodeVar *NodeToPush;     // KNode_Push
		kString         *ErrorMessage;   // KNode_Error
	};
	union {
		kSyntax       *syn;  /* untyped */
		uintptr_t      unboxConstValue;
		intptr_t       index;
		kObject*       ObjectConstValue;
	};
	kshort_t stackbase; ktypeattr_t attrTypeId;
};

#define kNode_uline(O)   (O)->KeyOperatorToken->uline

#define KNewNode(ns)     new_(Node, ns, OnGcStack)

#define kNode_IsRootNode(O)       IS_NameSpace(O->RootNodeNameSpace)
#define kNode_ns(O)               kNode_GetNameSpace(kctx, O)
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

#define kNode_GetParent(kctx, node)  ((IS_Node(node->Parent)) ? node->Parent : K_NULLNODE)
#define kNode_GetParentNULL(stmt)    ((IS_Node(stmt->Parent)) ? stmt->Parent : NULL)
#define kNode_SetParent(kctx, node, parent)   KFieldSet(node, node->Parent, parent)


static inline kNode *kNode_Type(KonohaContext *kctx, kNode *node, knode_t nodeType, ktypeattr_t attrTypeId)
{
	if(kNode_node(node) != KNode_Error) {
		kNode_setnode(node, nodeType);
		node->attrTypeId = attrTypeId;
	}
	return node;
}

static inline size_t kNode_GetNodeListSize(KonohaContext *kctx, kNode *node)
{
	return (IS_Array(node->NodeList)) ? kArray_size(node->NodeList) : 0;
}

#define kNode_IsTerm(N)           IS_Token((N)->TermToken)

#define kNodeFlag_ObjectConst        kObjectFlag_Local1

#define kNodeFlag_OpenBlock          kObjectFlag_Local2  /* KNode_Block */
#define kNodeFlag_CatchContinue      kObjectFlag_Local3  /* KNode_Block */
#define kNodeFlag_CatchBreak         kObjectFlag_Local4  /* KNode_Block */

#define kNode_Is(P, O)       (KFlag_Is(uintptr_t,(O)->h.magicflag, kNodeFlag_##P))
#define kNode_Set(P, O, B)   KFlag_Set(uintptr_t,(O)->h.magicflag, kNodeFlag_##P, B)

#define kNode_At(E, N)            ((E)->NodeList->NodeItems[(N)])
#define kNode_IsError(STMT)         (kNode_node(STMT) == KNode_Error)

#define kNode_GetObjectNULL(CTX, O, K)            (KLIB kObject_getObject(CTX, UPCAST(O), K, NULL))
#define kNode_GetObject(CTX, O, K, DEF)           (KLIB kObject_getObject(CTX, UPCAST(O), K, DEF))
#define kNode_SetObject(CTX, O, K, V)             KLIB kObjectProto_SetObject(CTX, UPCAST(O), K, kObject_typeId(V), UPCAST(V))
#define kNode_SetUnboxValue(CTX, O, K, T, V)      KLIB kObjectProto_SetUnboxValue(CTX, UPCAST(O), K, T, V)
#define kNode_RemoveKey(CTX, O, K)                KLIB kObjectProto_RemoveKey(CTX, UPCAST(O), K)
#define kNode_DoEach(CTX, O, THUNK, F)            kObjectProto_DoEach(CTX, UPCAST(O), THUNK, F)

#define kNode_Message(kctx, STMT, PE, FMT, ...)            SUGAR MessageNode(kctx, STMT, NULL, NULL, PE, FMT, ## __VA_ARGS__)
#define kNodeToken_Message(kctx, STMT, TK, PE, FMT, ...)   SUGAR MessageNode(kctx, STMT, TK, NULL, PE, FMT, ## __VA_ARGS__)


typedef struct {
	ktypeattr_t    attrTypeId;    ksymbol_t  name;
} KGammaStackDecl;

#define kNameSpace_TopLevel              (kshortflag_t)(1)
#define kNameSpace_IsTopLevel(GMA)       KFlag_Is(kshortflag_t, GMA->genv->flag, kNameSpace_TopLevel)

struct KGammaStack {
	KGammaStackDecl *varItems;
	size_t varsize;
	size_t capacity;
	size_t allocsize;  // set size if not allocated  (by default on stack)
} ;

struct KGammaLocalData {
	kshortflag_t  flag;   kshortflag_t cflag;
	KClass   *thisClass;
	kMethod  *currentWorkingMethod;
	struct KGammaStack    localScope;
} ;

/* ------------------------------------------------------------------------ */

#define KGetParserContext(kctx)    ((KParserContext *)kctx->modlocal[MOD_sugar])
#define KPARSERM            ((KParserModule *)kctx->modshare[MOD_sugar])
#define KClass_Symbol       KPARSERM->cSymbol
#define KClass_SymbolVar    KPARSERM->cSymbol
#define KClass_Syntax       KPARSERM->cSyntax
#define KClass_SyntaxVar    KPARSERM->cSyntax
#define KClass_Token        KPARSERM->cToken
#define KClass_TokenVar     KPARSERM->cToken
#define KClass_Node         KPARSERM->cNode
#define KClass_NodeVar      KPARSERM->cNode
#define KClass_Gamma        KPARSERM->cGamma
#define KClass_GammaVar     KPARSERM->cGamma


#define KClass_TokenArray       KPARSERM->cTokenArray
#define kTokenArray             kArray
#define KClass_NodeArray        KClass_Array
#define kNodeArray              kArray
#define KClass_NodeArray        KClass_Array
#define kNodeArray              kArray

#define IS_Syntax(O) (kObject_class(O) == KClass_Syntax)
#define IS_Token(O)  (kObject_class(O) == KClass_Token)
#define IS_Node(O)   (kObject_class(O) == KClass_Node)
#define IS_Gamma(O)  (kObject_class(O) == KClass_Gamma)

#define K_NULLTOKEN  ((kToken *)(KClass_Token)->defaultNullValue)
#define K_NULLNODE   (kNode *)((KClass_Node)->defaultNullValue)
#define K_NULLBLOCK  (kNode *)((KClass_Node)->defaultNullValue)

typedef kNode* (*KTypeDeclFunc)(KonohaContext *kctx, kNode *stmt, kNameSpace *ns, ktypeattr_t ty, kNode *termNode, kNode *vexpr, kObject *thunk);

typedef enum {
	ParseExpressionOption = 0,
	ParseMetaPatternOption = 1,
	OnlyPatternMatch = 1 << 2,
	ParseBlockOption = 1 << 3
} ParseOption;

struct KBuilder;

typedef kbool_t (*IsSeparatorFunc)(KonohaContext *kctx, kToken *tk);

typedef struct KParserModule {
	KRuntimeModule  h;
	KClass *cSymbol;
	KClass *cSyntax;
	KClass *cToken;
	KClass *cNode;
	KClass *cTokenArray;
	//
	kFunc  *termParseFunc;
	kFunc  *opParseFunc;
	kFunc  *patternParseFunc;
	kFunc  *methodTypeFunc;
	//
	kSyntax*      (*kNameSpace_GetSyntax)(KonohaContext *, kNameSpace *, ksymbol_t);
	void          (*kNameSpace_DefineSyntax)(KonohaContext *, kNameSpace *, KDEFINE_SYNTAX *, KTraceInfo *);
	void          (*kNameSpace_AddSyntax)(KonohaContext *, kNameSpace *, kSyntax *, KTraceInfo *);
	void          (*kNameSpace_UseDefaultVirtualMachine)(KonohaContext *, kNameSpace *);
	void          (*kSyntax_AddPattern)(KonohaContext *, kSyntax *, const char *rule, kfileline_t uline, KTraceInfo *);
	kbool_t       (*SetMacroData)(KonohaContext *, kNameSpace *, ksymbol_t, int, const char *, int optionMacro);

	void         (*Tokenize)(KonohaContext *, kNameSpace *, const char *, kfileline_t, int baseIndent, kArray *bufferList);
	void         (*ApplyMacroData)(KonohaContext *, kNameSpace *, kArray *, int, int, size_t, KMacroSet *, kArray *bufferList);
	void         (*Preprocess)(KonohaContext *, kNameSpace *, kArray *, int, int, KMacroSet *, kArray *bufferList);
	kstatus_t    (*EvalTokenList)(KonohaContext *, KTokenSeq *, KTraceInfo *);

	int          (*ParseTypePattern)(KonohaContext *, kNameSpace *, kArray *, int , int , KClass **classRef);
	kTokenVar*   (*kToken_ToBraceGroup)(KonohaContext *, kTokenVar *, kNameSpace *, KMacroSet *);

	void         (*kNode_AddParsedObject)(KonohaContext *, kNode *, ksymbol_t, kObject *o);
	int          (*FindEndOfStatement)(KonohaContext *, kNameSpace *, kArray *, int, int);

	uintptr_t    (*kNode_ParseFlag)(KonohaContext *kctx, kNode *stmt, KFlagSymbolData *flagData, uintptr_t flag);
	kToken*      (*kNode_GetToken)(KonohaContext *, kNode *, ksymbol_t kw, kToken *def);
	kNode*       (*kNode_GetNode)(KonohaContext *, kNode *, ksymbol_t kw, kNode *def);
	kNode*       (*kNode_AddNode)(KonohaContext *, kNode *, kNode *);
	void         (*kNode_InsertAfter)(KonohaContext *, kNode *, kNode *target, kNode *);

//	kNode*       (*kNode_Termnize)(KonohaContext *, kNode *, kToken *);
	kNode*       (*kNode_Op)(KonohaContext *kctx, kNode *, kToken *keyToken, int n, ...);
//	kNodeVar*    (*new_UntypedOperatorNode)(KonohaContext *, kSyntax *syn, int n, ...);
	int          (*ParseSyntaxNode)(KonohaContext *, kSyntax *, kNode *, ksymbol_t, kArray *, int beginIdx, int opIdx, int endIdx);

	int          (*ParseNode)(KonohaContext *, kNode *, kArray *, int beginIdx, int endIdx, ParseOption, const char *requiredTokenText);
	kNode*       (*ParseNewNode)(KonohaContext *, kNameSpace *, kArray *tokenList, int* s, int e, ParseOption, const char *requiredTokenText);
	kNode*       (*AppendParsedNode)(KonohaContext *, kNode *, kArray *tokenList, int, int, IsSeparatorFunc, ParseOption, const char *requiredTokenText);

	kNode*       (*kNode_SetConst)(KonohaContext *, kNode *, KClass *, kObject *);
	kNode*       (*kNode_SetUnboxConst)(KonohaContext *, kNode *, ktypeattr_t, uintptr_t);
	kNode*       (*kNode_SetVariable)(KonohaContext *, kNode *, knode_t build, ktypeattr_t, intptr_t index);

	kNode*       (*new_MethodNode)(KonohaContext *, kNameSpace *, KClass *, kMethod *mtd, int n, ...);

	kNode*      (*TypeCheckNodeByName)(KonohaContext *, kNode*, ksymbol_t, kNameSpace *, KClass *, int);
	kNode*      (*TypeCheckNodeAt)(KonohaContext *, kNode *, size_t, kNameSpace *, KClass *, int);
	kNode *     (*TypeCheckMethodParam)(KonohaContext *, kMethod *mtd, kNode *, kNameSpace *, KClass *);
	int         (*AddLocalVariable)(KonohaContext *, kNameSpace *, ktypeattr_t, ksymbol_t);
	void        (*kNode_DeclType)(KonohaContext *, kNode *, kNameSpace *, ktypeattr_t, kNode *, kObject *, KTypeDeclFunc);
	kNode*      (*TypeVariableNULL)(KonohaContext *, kNode *, kNameSpace *, KClass *);

	void       (*kToken_ToError)(KonohaContext *, kTokenVar *, kinfotag_t, const char *fmt, ...);
	kNode *    (*MessageNode)(KonohaContext *, kNode *, kToken *, kNameSpace *, kinfotag_t, const char *fmt, ...);

	kbool_t    (*VisitNode)(KonohaContext *, struct KBuilder *, kNode *node, void *thunk);

	void (*dumpToken)(KonohaContext *kctx, kToken *tk, int n);
	void (*dumpTokenArray)(KonohaContext *kctx, int nest, kArray *a, int s, int e);
	void (*dumpNode)(KonohaContext *kctx, kNode *stmt);

} KParserModule;

typedef struct {
	KContextModule     h;
	kArray            *preparedTokenList;
	KGrowingArray      errorMessageBuffer;
	kArray            *errorMessageList;
	int                errorMessageCount;
	kbool_t            isBlockedErrorMessage;
	kArray            *definedMethodList;
} KParserContext;

#define KClass_INFER    KClass_(KType_var)

typedef enum {
	TypeCheckPolicy_NoPolicy       = 0,
	TypeCheckPolicy_NoCheck        = (1 << 0),
	TypeCheckPolicy_AllowVoid      = (1 << 1),
	TypeCheckPolicy_Coercion       = (1 << 2),
	TypeCheckPolicy_AllowEmpty     = (1 << 3),
	TypeCheckPolicy_CONST          = (1 << 4),  /* Reserved */
	TypeCheckPolicy_Creation       = (1 << 6)   /* TypeCheckNodeByName */
} TypeCheckPolicy;

#define KPushMethodCall(gma)   SUGAR AddLocalVariable(kctx, ns, KType_var, 0)

#define new_ConstNode(CTX, NS, T, O)              SUGAR kNode_SetConst(CTX, KNewNode(NS), T, O)
#define new_UnboxConstNode(CTX, NS, T, D)         SUGAR kNode_SetUnboxConst(CTX, KNewNode(NS), T, D)
#define new_VariableNode(CTX, NS, BLD, TY, IDX)   SUGAR kNode_SetVariable(CTX, KNewNode(NS), BLD, TY, IDX)

#define SUGAR                                   ((const KParserModule *)KPARSERM)->
#define KType_Syntax                            SUGAR cSyntax->typeId
#define KType_Symbol                            SUGAR cSymbol->typeId
#define KType_Token                             SUGAR cToken->typeId
#define KType_Node                              SUGAR cNode->typeId
#define KType_TokenArray                        SUGAR cTokenArray->typeId

//#define KSymbol_(T)                               _e->keyword(kctx, T, sizeof(T)-1, KSymbol_Noname)
#define kSyntax_(NS, KW)                        SUGAR kNameSpace_GetSyntax(kctx, NS, KW)
//#define NEWkSyntax_(KS, KW)                     (kSyntaxVar *)(SUGAR kNameSpace_GetSyntax(kctx, KS, KW, 1))

#ifdef USE_SMALLBUILD
#define KDump(O)
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

typedef kbool_t (*KNodeVisitFunc)(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk);

struct KBuilderCommon {
	const struct KBuilderAPI *api;
	int option;
	kfileline_t uline;
};

#define DefineVisitFunc(NAME) KNodeVisitFunc visit##NAME##Node;

struct KBuilderAPI {
	const char *target;
	const struct ExecutionEngineModule *ExecutionEngineModule;
	KNodeList(DefineVisitFunc)
};

/* ------------------------------------------------------------------------ */

static inline void kToken_SetTypeId(KonohaContext *kctx, kToken *tk, kNameSpace *ns, ktypeattr_t type)
{
	((kTokenVar *)tk)->resolvedTypeId = type;
	((kTokenVar *)tk)->resolvedSyntaxInfo = SUGAR kNameSpace_GetSyntax(kctx, ns, KSymbol_TypePattern);
}

#define kNode_isSymbolTerm(expr)   1

#ifdef __cplusplus
}
#endif

/* ------------------------------------------------------------------------ */
#endif /* SUGAR_H_ */
