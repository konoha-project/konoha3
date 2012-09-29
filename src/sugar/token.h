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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define strtoll _strtoi64
#endif

/* ------------------------------------------------------------------------ */
/* TokenRange */

static TokenRange* new_TokenListRange(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, TokenRange *bufRange)
{
	bufRange->ns        = ns;
	bufRange->tokenList = tokenList;
	bufRange->beginIdx  = kArray_size(tokenList);
	bufRange->endIdx    = bufRange->beginIdx;
	bufRange->errToken  = NULL;
	return bufRange;
}

static TokenRange* new_TokenStackRange(KonohaContext *kctx, TokenRange *range, TokenRange *bufRange)
{
	bufRange->ns        = range->ns;
	bufRange->tokenList = range->tokenList;
	bufRange->beginIdx  = kArray_size(range->tokenList);
	bufRange->endIdx    = bufRange->beginIdx;
	bufRange->errToken  = range->errToken;
	return bufRange;
}

/* ------------------------------------------------------------------------ */

static void ERROR_UnclosedToken(KonohaContext *kctx, kTokenVar *tk, const char *ch)
{
	if(IS_NOTNULL(tk)) {
		kToken_printMessage(kctx, tk, ErrTag, "must close with %s", ch);
	}
}

static int parseINDENT(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int pos)
{
	int ch, indent = 0;
	while((ch = tenv->source[pos++]) != 0) {
		if(ch == '\t') { indent += tenv->tabsize; }
		else if(ch == ' ') { indent += 1; }
		break;
	}
	if(IS_NOTNULL(tk)) {
		tk->unresolvedTokenType = TokenType_INDENT;
		tk->indent = indent;
	}
	return pos-1;
}

static int parseNL(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int pos)
{
	tenv->currentLine += 1;
	return parseINDENT(kctx, tk, tenv, pos+1);
}

static int parseNUM(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	int ch, pos = tok_start;
	const char *ts = tenv->source;
	while((ch = ts[pos++]) != 0) {
		//if(ch == '_') continue; // nothing
		if(!isalnum(ch)) break;
	}
	if(IS_NOTNULL(tk)) {
		KSETv(tk, tk->text, KLIB new_kString(kctx, ts + tok_start, (pos-1)-tok_start, SPOL_ASCII));
		tk->unresolvedTokenType = TokenType_INT;
	}
	return pos - 1;  // next
}

static void kToken_setSymbolText(KonohaContext *kctx, kTokenVar *tk, const char *t, size_t len)
{
	if(IS_NOTNULL(tk)) {
		ksymbol_t kw = ksymbolA(t, len, SYM_NONAME);
		if(kw == SYM_UNMASK(kw)) {
			KSETv(tk, tk->text, SYM_s(kw));
		}
		else {
			KSETv(tk, tk->text, KLIB new_kString(kctx, t, len, SPOL_ASCII));
		}
		tk->unresolvedTokenType = TokenType_SYMBOL;
		if(len == 1) {
			tk->topCharHint = t[0];
		}
	}
}

static int parseSYMBOL(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	int ch, pos = tok_start;
	const char *ts = tenv->source;
	while((ch = ts[pos++]) != 0) {
		if(ch == '_' || isalnum(ch)) continue; // nothing
		break;
	}
	kToken_setSymbolText(kctx, tk, ts + tok_start, (pos-1)-tok_start);
	return pos - 1;  // next
}

static int parseOP1(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	kToken_setSymbolText(kctx, tk,  tenv->source + tok_start, 1);
	return tok_start+1;
}

static int parseOP(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	int ch, pos = tok_start;
	while((ch = tenv->source[pos++]) != 0) {
		if(isalnum(ch)) break;
		switch(ch) {
			case '<': case '>': case '@': case '$': case '#':
			case '+': case '-': case '*': case '%': case '/':
			case '=': case '&': case '?': case ':': case '.':
			case '^': case '!': case '~': case '|':
			continue;
		}
		break;
	}
	kToken_setSymbolText(kctx, tk, tenv->source + tok_start, (pos-1)-tok_start);
	return pos-1;
}

static int parseLINE(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	int ch, pos = tok_start;
	while((ch = tenv->source[pos++]) != 0) {
		if(ch == '\n') break;
	}
	return pos-1;/*EOF*/
}

static int parseCOMMENT(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	int ch, prev = 0, level = 1, pos = tok_start + 2;
	/*@#nnnn is line number */
	if(tenv->source[pos] == '@' && tenv->source[pos+1] == '#' && isdigit(tenv->source[pos+2])) {
		tenv->currentLine >>= (sizeof(kshort_t)*8);
		tenv->currentLine = (tenv->currentLine<<(sizeof(kshort_t)*8))  | (kshort_t)strtoll(tenv->source + pos + 2, NULL, 10);
	}
	while((ch = tenv->source[pos++]) != 0) {
		if(ch == '\n') {
			tenv->currentLine += 1;
		}
		if(prev == '*' && ch == '/') {
			level--;
			if(level == 0) return pos;
		} else if(prev == '/' && ch == '*') {
			level++;
		}
		prev = ch;
	}
	ERROR_UnclosedToken(kctx, tk, "*/");
	return pos-1;/*EOF*/
}

static int parseSLASH(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	const char *ts = tenv->source + tok_start;
	if(ts[1] == '/') {
		return parseLINE(kctx, tk, tenv, tok_start);
	}
	if(ts[1] == '*') {
		return parseCOMMENT(kctx, tk, tenv, tok_start);
	}
	return parseOP(kctx, tk, tenv, tok_start);
}

static int parseDoubleQuotedText(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	int ch, prev = '"', prev2 = '\0', pos = tok_start + 1, hasUTF8 = false;
	while((ch = tenv->source[pos++]) != 0) {
		if(ch == '\n') {
			break;
		}
		if(ch == '"' && (prev != '\\' || (prev == '\\' && prev2 == '\\'))) {
			if(IS_NOTNULL(tk)) {
				size_t length = pos - (tok_start + 1) - 1;
				KSETv(tk, tk->text, KLIB new_kString(kctx, tenv->source + tok_start + 1, length, hasUTF8 ? SPOL_UTF8 : SPOL_ASCII));
				tk->unresolvedTokenType = TokenType_TEXT;
			}
			return pos;
		}
		if(ch == '\\' && IS_NOTNULL(tk)) {
			kToken_set(RequiredReformat, tk, true);
		}
		if(ch < 0) {
			hasUTF8 = true;
		}
		prev2 = prev;
		prev = ch;
	}
	ERROR_UnclosedToken(kctx, tk, "\"");
	return pos-1;
}

static int ParseWhiteSpace(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	size_t size = kArray_size(tenv->tokenList);
	if(size > 0) {
		kTokenVar *tk = tenv->tokenList->tokenVarItems[size-1];
		if(tk->uline == tenv->currentLine && tk->unresolvedTokenType != TokenType_INDENT) {
			kToken_set(BeforeWhiteSpace, tk, true);
		}
	}
	return tok_start+1;
}

static int parseSKIP(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	return tok_start+1;
}

static int parseUndefinedToken(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	if(IS_NOTNULL(tk)) {
		kToken_printMessage(kctx, tk, ErrTag, "undefined token character: %c (ascii=%x)", tenv->source[tok_start], tenv->source[tok_start]);
	}
	while(tenv->source[++tok_start] != 0);
	return tok_start;
}

static int parseLazyBlock(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start);

static const TokenizeFunc MiniKonohaTokenMatrix[] = {
#define _NULL      0
	parseSKIP,
#define _UNDEF     1
	parseSKIP,
#define _DIGIT     2
	parseNUM,
#define _UALPHA    3
	parseSYMBOL,
#define _LALPHA    4
	parseSYMBOL,
#define _MULTI     5
	parseUndefinedToken,
#define _NL        6
	parseNL,
#define _TAB       7
	ParseWhiteSpace,
#define _SP_       8
	ParseWhiteSpace,
#define _LPAR      9
	parseOP1,
#define _RPAR      10
	parseOP1,
#define _LSQ       11
	parseOP1,
#define _RSQ       12
	parseOP1,
#define _LBR       13
	parseLazyBlock,
#define _RBR       14
	parseOP1,
#define _LT        15
	parseOP,
#define _GT        16
	parseOP,
#define _QUOTE     17
	parseUndefinedToken,
#define _DQUOTE    18
	parseDoubleQuotedText,
#define _BKQUOTE   19
	parseUndefinedToken,
#define _OKIDOKI   20
	parseOP,
#define _SHARP     21
	parseOP,
#define _DOLLAR    22
	parseOP,
#define _PER       23
	parseOP,
#define _AND       24
	parseOP,
#define _STAR      25
	parseOP,
#define _PLUS      26
	parseOP,
#define _COMMA     27
	parseOP1,
#define _MINUS     28
	parseOP,
#define _DOT       29
	parseOP1,
#define _SLASH     30
	parseSLASH,
#define _COLON     31
	parseOP,
#define _SEMICOLON 32
	parseOP1,
#define _EQ        33
	parseOP,
#define _QUESTION  34
	parseOP,
#define _AT_       35
	parseOP1,
#define _VAR       36
	parseOP,
#define _CHILDER   37
	parseOP,
#define _BKSLASH   38
	parseUndefinedToken,
#define _HAT       39
	parseOP,
#define _UNDER     40
	parseSYMBOL,
#define KCHAR_MAX  41
};

static const char cMatrix[128] = {
	0/*nul*/, 1/*soh*/, 1/*stx*/, 1/*etx*/, 1/*eot*/, 1/*enq*/, 1/*ack*/, 1/*bel*/,
	1/*bs*/,  _TAB/*ht*/, _NL/*nl*/, 1/*vt*/, 1/*np*/, 1/*cr*/, 1/*so*/, 1/*si*/,
	/*020 dle  021 dc1  022 dc2  023 dc3  024 dc4  025 nak  026 syn  027 etb */
	1, 1, 1, 1,     1, 1, 1, 1,
	/*030 can  031 em   032 sub  033 esc  034 fs   035 gs   036 rs   037 us */
	1, 1, 1, 1,     1, 1, 1, 1,
	/*040 sp   041  !   042  "   043  #   044  $   045  %   046  &   047  ' */
	_SP_, _OKIDOKI, _DQUOTE, _SHARP, _DOLLAR, _PER, _AND, _QUOTE,
	/*050  (   051  )   052  *   053  +   054  ,   055  -   056  .   057  / */
	_LPAR, _RPAR, _STAR, _PLUS, _COMMA, _MINUS, _DOT, _SLASH,
	/*060  0   061  1   062  2   063  3   064  4   065  5   066  6   067  7 */
	_DIGIT, _DIGIT, _DIGIT, _DIGIT,  _DIGIT, _DIGIT, _DIGIT, _DIGIT,
	/*070  8   071  9   072  :   073  ;   074  <   075  =   076  >   077  ? */
	_DIGIT, _DIGIT, _COLON, _SEMICOLON, _LT, _EQ, _GT, _QUESTION,
	/*100  @   101  A   102  B   103  C   104  D   105  E   106  F   107  G */
	_AT_, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA,
	/*110  H   111  I   112  J   113  K   114  L   115  M   116  N   117  O */
	_UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA,
	/*120  P   121  Q   122  R   123  S   124  T   125  U   126  V   127  W */
	_UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA, _UALPHA,
	/*130  X   131  Y   132  Z   133  [   134  \   135  ]   136  ^   137  _ */
	_UALPHA, _UALPHA, _UALPHA, _LSQ, _BKSLASH, _RSQ, _HAT, _UNDER,
	/*140  `   141  a   142  b   143  c   144  d   145  e   146  f   147  g */
	_BKQUOTE, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA,
	/*150  h   151  i   152  j   153  k   154  l   155  m   156  n   157  o */
	_LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA,
	/*160  p   161  q   162  r   163  s   164  t   165  u   166  v   167  w */
	_LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA, _LALPHA,
	/*170  x   171  y   172  z   173  {   174  |   175  }   176  ~   177 del*/
	_LALPHA, _LALPHA, _LALPHA, _LBR, _VAR, _RBR, _CHILDER, 1,
};

static int kchar(const char *t, int pos)
{
	int ch = t[pos];
	return (ch < 0) ? _MULTI : cMatrix[ch];
}

static int callTokennizeFunc(KonohaContext *kctx, kFunc *fo, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	// The above string operation is bad thing. Don't repeat it
	DBG_ASSERT(IS_Func(fo));
	kStringVar *preparedString = (kStringVar*)tenv->preparedString;
	preparedString->text = tenv->source + tok_start;
	preparedString->bytesize = tenv->sourceLength - tok_start;
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)tk, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].s, preparedString, GC_NO_WRITE_BARRIER);
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KSetMethodCallStack(sfp, 0/*UL*/, fo->mtd, 2, KLIB Knull(kctx, CT_Int));
		KonohaRuntime_callMethod(kctx, sfp);
	}
	END_LOCAL();
	int pos = lsfp[0].intValue + tok_start;
	if(pos > tok_start) { // check new lines
		int i;
		const char *t = tenv->source;
		for(i = tok_start; i < pos; i++) {
			if(t[i] == '\n') tenv->currentLine += 1;
		}
	}
	return pos;
}

static int tokenizeEach(KonohaContext *kctx, int kchar, kTokenVar* tk, TokenizerEnv *tenv, int tok_start)
{
	int pos;
	if(tenv->funcItems != NULL && tenv->funcItems[kchar] != NULL) {
		kFunc *fo = tenv->funcItems[kchar];
		if(IS_Array(fo)) {
			kArray *a = (kArray*)fo;
			int i;
			for(i = kArray_size(a) - 1; i >= 0; i--) {
				pos = callTokennizeFunc(kctx, a->funcItems[i], tk, tenv, tok_start);
				if(pos > tok_start) return pos;
			}
			fo = a->funcItems[0];
		}
		pos = callTokennizeFunc(kctx, fo, tk, tenv, tok_start);
		if(pos > tok_start) return pos;
	}
	pos = tenv->cfuncItems[kchar](kctx, tk, tenv, tok_start);
	return pos;
}

static void tokenize(KonohaContext *kctx, TokenizerEnv *tenv)
{
	int ch, pos = 0;
	kTokenVar *tk = GCSAFE_new(TokenVar, 0);
	tk->uline = tenv->currentLine;
	pos = parseINDENT(kctx, tk, tenv, pos);
	while((ch = kchar(tenv->source, pos)) != 0) {
		if(tk->unresolvedTokenType != 0) {
			KLIB kArray_add(kctx, tenv->tokenList, tk);
			tk = GCSAFE_new(TokenVar, 0);
			tk->uline = tenv->currentLine;
		}
		pos = tokenizeEach(kctx, ch, tk, tenv, pos);
	}
	if(tk->unresolvedTokenType != 0) {
		KLIB kArray_add(kctx, tenv->tokenList, tk);
	}
}

static int parseLazyBlock(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	int ch, level = 1, pos = tok_start + 1;
	while((ch = kchar(tenv->source, pos)) != 0) {
		if(ch == _RBR/*}*/) {
			level--;
			if(level == 0) {
				if(IS_NOTNULL(tk)) {
					KSETv(tk, tk->text, KLIB new_kString(kctx, tenv->source + tok_start + 1, ((pos-2)-(tok_start)+1), 0));
					tk->unresolvedTokenType = TokenType_CODE;
				}
				return pos + 1;
			}
			pos++;
		}
		else if(ch == _LBR/*'{'*/) {
			level++; pos++;
		}
		else {
			pos = tokenizeEach(kctx, ch, (kTokenVar*)K_NULLTOKEN, tenv, pos);
		}
	}
	ERROR_UnclosedToken(kctx, tk, "}");
	return pos-1;
}

static const TokenizeFunc *kNameSpace_tokenMatrix(KonohaContext *kctx, kNameSpace *ns)
{
	if(ns->tokenMatrix == NULL) {
		DBG_ASSERT(KCHAR_MAX * sizeof(TokenizeFunc) == sizeof(MiniKonohaTokenMatrix));
		TokenizeFunc *tokenMatrix = (TokenizeFunc*)KMALLOC(SIZEOF_TOKENMATRIX);
		if(ns->parentNULL != NULL && ns->parentNULL->tokenMatrix != NULL) {
			memcpy(tokenMatrix, ns->parentNULL->tokenMatrix, SIZEOF_TOKENMATRIX);
		}
		else {
			memcpy(tokenMatrix, MiniKonohaTokenMatrix, sizeof(MiniKonohaTokenMatrix));
			bzero(tokenMatrix + KCHAR_MAX, sizeof(MiniKonohaTokenMatrix));
		}
		((kNameSpaceVar*)ns)->tokenMatrix = (void*)tokenMatrix;
	}
	return (TokenizeFunc*)ns->tokenMatrix;
}

static kFunc **kNameSpace_tokenFuncMatrix(KonohaContext *kctx, kNameSpace *ns)
{
	kFunc **funcMatrix = (kFunc**)kNameSpace_tokenMatrix(kctx, ns);
	return funcMatrix + KCHAR_MAX;
}

static void kNameSpace_setTokenizeFunc(KonohaContext *kctx, kNameSpace *ns, int ch, TokenizeFunc cfunc, kFunc *funcTokenize, int isAddition)
{
	int kchar = (ch < 0) ? _MULTI : cMatrix[ch];
	if(cfunc != NULL) {
		TokenizeFunc *funcMatrix = (TokenizeFunc *)kNameSpace_tokenMatrix(kctx, ns);
		funcMatrix[kchar] = cfunc;
	}
	else {
		kFunc ** funcMatrix = kNameSpace_tokenFuncMatrix(kctx, ns);
		if(funcMatrix[kchar] == NULL) {
			KINITp(ns, funcMatrix[kchar], funcTokenize);
		}
		else {
			if(isAddition) {
				kArray *a = (kArray*)funcMatrix[kchar];
				if(!IS_Array(a)) {
					a = new_(Array, 0);
					KLIB kArray_add(kctx, a, funcMatrix[kchar]);
					KSETv(ns, funcMatrix[kchar], (kFunc*)a);
				}
				KLIB kArray_add(kctx, a, funcTokenize);
			}
			else {
				KSETv(ns, funcMatrix[kchar], funcTokenize);
			}
		}
	}
}

static void TokenRange_tokenize(KonohaContext *kctx, TokenRange *range, const char *source, kfileline_t uline)
{
	DBG_ASSERT(range->beginIdx == range->endIdx);
	TokenizerEnv tenv = {0};
	tenv.source = source;
	tenv.sourceLength = strlen(source);
	tenv.currentLine  = uline;
	tenv.tokenList    = range->tokenList;
	tenv.tabsize = 4;
	tenv.cfuncItems   = (range->ns == NULL) ? MiniKonohaTokenMatrix : kNameSpace_tokenMatrix(kctx, range->ns);

	INIT_GCSTACK();
	kString *preparedString = KLIB new_kString(kctx, tenv.source, tenv.sourceLength, SPOL_ASCII|SPOL_TEXT|SPOL_NOPOOL);
	PUSH_GCSTACK(preparedString);
	tenv.preparedString = preparedString;
	if(range->ns != NULL) {
		tenv.funcItems = kNameSpace_tokenFuncMatrix(kctx, range->ns);
	}
	tokenize(kctx, &tenv);
	TokenRange_end(kctx, range);
	RESET_GCSTACK();
	if(uline == 0) {
		size_t i;
		for(i = range->beginIdx; i < range->endIdx; i++) {
			range->tokenList->tokenVarItems[i]->uline = 0;
		}
	}
}

// ---------------------------------------------------------------------------

static kbool_t kArray_addSyntaxRule(KonohaContext *kctx, kArray *ruleList, TokenRange *sourceRange);
#define Token_topch(tk) ((IS_String(tk->text) && S_size((tk)->text) == 1) ? S_text((tk)->text)[0] : 0)

static int findCloseChar2(KonohaContext *kctx, TokenRange *sourceRange, int beginIdx, int closech)
{
	int i;
	for(i = beginIdx; i < sourceRange->endIdx; i++) {
		kToken *tk = sourceRange->tokenList->tokenItems[i];
		if(Token_topch(tk) == closech) return i;
	}
	return sourceRange->endIdx;
}

static int makeNestedSyntaxRule(KonohaContext *kctx, TokenRange *sourceRange, int currentIdx, ksymbol_t KW_AST, int closech)
{
	kTokenVar *tk = sourceRange->tokenList->tokenVarItems[currentIdx];
	int ne = findCloseChar2(kctx, sourceRange, currentIdx+1, closech);
	tk->resolvedSymbol = KW_AST;
	tk->resolvedSyntaxInfo = SYN_(sourceRange->ns, KW_AST);
	KSETv(tk, tk->subTokenList, new_(TokenArray, 0));
	TokenRange nestedSourceRange = {sourceRange->ns, sourceRange->tokenList, currentIdx+1, ne};
	return kArray_addSyntaxRule(kctx, tk->subTokenList, &nestedSourceRange) ? ne : sourceRange->endIdx;
}

static kbool_t kArray_addSyntaxRule(KonohaContext *kctx, kArray *ruleList, TokenRange *sourceRange)
{
	int i;
	ksymbol_t stmtEntryKey = 0;
	//KdumpTokenArray(kctx, sourceRange->tokenList, sourceRange->beginIdx, sourceRange->endIdx);
	for(i = sourceRange->beginIdx; i < sourceRange->endIdx; i++) {
		kTokenVar *tk = sourceRange->tokenList->tokenVarItems[i];
		DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
		if(tk->resolvedSyntaxInfo->keyword == KW_TextPattern) {
			int topch = Token_topch(tk);
			KLIB kArray_add(kctx, ruleList, tk);
			if(topch == '(') {
				i = makeNestedSyntaxRule(kctx, sourceRange, i, KW_ParenthesisGroup, ')');
			}
			else if(topch == '[') {
				i = makeNestedSyntaxRule(kctx, sourceRange, i, KW_BracketGroup, ']');
			}
			else {
				tk->resolvedSymbol = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
			}
			continue;
		}
		if(tk->resolvedSyntaxInfo->keyword == KW_BracketGroup) {
			TokenRange nestedSourceRange = {sourceRange->ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			tk->resolvedSymbol = KW_OptionalGroup;
			PUSH_GCSTACK(tk->subTokenList);  // avoid gc
			KSETv(tk, tk->subTokenList, new_(TokenArray, 0));
			kArray_addSyntaxRule(kctx, tk->subTokenList, &nestedSourceRange);
			KLIB kArray_add(kctx, ruleList, tk);
			continue;
		}
		if(tk->topCharHint == '$' && i+1 < sourceRange->endIdx) {  // $PatternName
			tk = sourceRange->tokenList->tokenVarItems[++i];
			if(IS_String(tk->text)) {
				tk->resolvedSyntaxInfo = SYN_(sourceRange->ns, KW_SymbolPattern);
				tk->resolvedSymbol = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWRAW) | KW_PATTERN;
				if(stmtEntryKey == 0) stmtEntryKey = tk->resolvedSymbol;
				tk->stmtEntryKey = stmtEntryKey;
				stmtEntryKey = 0;
				KLIB kArray_add(kctx, ruleList, tk);
				continue;
			}
		}
		if(i + 1 < sourceRange->endIdx && sourceRange->tokenList->tokenItems[i+1]->topCharHint == ':' && IS_String(tk->text)) {
			stmtEntryKey = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWRAW);
			i++;
			continue;
		}
		kToken_printMessage(kctx, tk, ErrTag, "illegal syntax rule: %s", Token_text(tk));
		return false;
	}
	//KdumpTokenArray(kctx, ruleList, 0, kArray_size(ruleList));
	return true;
}

static kbool_t TokenRange_resolved(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange);

static void kNameSpace_parseSugarRule2(KonohaContext *kctx, kNameSpace *ns, const char *rule, kfileline_t uline, kArray *ruleList)
{
	TokenRange rawRangeBuf, *rawRange = new_TokenListRange(kctx, ns, KonohaContext_getSugarContext(kctx)->preparedTokenList, &rawRangeBuf);
	TokenRange_tokenize(kctx, rawRange, rule, uline);
	TokenRange sourceRangeBuf, *sourceRange = new_TokenStackRange(kctx, rawRange, &sourceRangeBuf);
	TokenRange_resolved(kctx, sourceRange, rawRange);
	KdumpTokenRange(kctx, "resolved", sourceRange);
	kArray_addSyntaxRule(kctx, ruleList, sourceRange);
	TokenRange_pop(kctx, rawRange);
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
