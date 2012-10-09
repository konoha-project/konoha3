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

static void ERROR_UnclosedToken(KonohaContext *kctx, kTokenVar *tk, const char *ch)
{
	if(IS_NOTNULL(tk)) {
		kToken_printMessage(kctx, tk, ErrTag, "must close with %s", ch);
	}
}

static int ParseIndent(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int pos)
{
	int ch, indent = 0;
	while((ch = tokenizer->source[pos++]) != 0) {
		if(ch == '\t') { indent += tokenizer->tabsize; }
		else if(ch == ' ') { indent += 1; }
		break;
	}
	if(IS_NOTNULL(tk)) {
		tk->unresolvedTokenType = TokenType_INDENT;
		tk->indent = indent;
		tk->uline = tokenizer->currentLine;
	}
	return pos-1;
}

static int ParseLineFeed(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int pos)
{
	if(tokenizer->currentLine != 0) {
		tokenizer->currentLine += 1;
	}
	return ParseIndent(kctx, tk, tokenizer, pos+1);
}

static int ParseBackSlash(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int pos)
{
	if(tokenizer->source[pos+1] == '\n') {
		if(tokenizer->currentLine != 0) {
			tokenizer->currentLine += 1;
		}
		return ParseIndent(kctx, (kTokenVar*)KNULL(Token), tokenizer, pos+2);
	}
	return pos+1;
}

static int ParseNumber(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start;
	const char *ts = tokenizer->source;
	while((ch = ts[pos++]) != 0) {
		if(!isalnum(ch)) break;
	}
	if(IS_NOTNULL(tk)) {
		KFieldSet(tk, tk->text, KLIB new_kString(kctx, ts + tok_start, (pos-1)-tok_start, StringPolicy_ASCII));
		tk->unresolvedTokenType = TokenType_INT;
	}
	return pos - 1;  // next
}

static void kToken_setSymbolText(KonohaContext *kctx, kTokenVar *tk, const char *t, size_t len)
{
	if(IS_NOTNULL(tk)) {
		ksymbol_t kw = ksymbolA(t, len, SYM_NONAME);
		if(kw == SYM_UNMASK(kw)) {
			KFieldSet(tk, tk->text, SYM_s(kw));
		}
		else {
			KFieldSet(tk, tk->text, KLIB new_kString(kctx, t, len, StringPolicy_ASCII));
		}
		tk->unresolvedTokenType = TokenType_SYMBOL;
		if(len == 1) {
			tk->topCharHint = t[0];
		}
	}
}

static int ParseSymbol(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start;
	const char *ts = tokenizer->source;
	while((ch = ts[pos++]) != 0) {
		if(ch == '_' || isalnum(ch)) continue; // nothing
		break;
	}
	kToken_setSymbolText(kctx, tk, ts + tok_start, (pos-1)-tok_start);
	return pos - 1;  // next
}

static int ParseSingleOperator(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	kToken_setSymbolText(kctx, tk,  tokenizer->source + tok_start, 1);
	return tok_start+1;
}

static int ParseSemiColon(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	kToken_setSymbolText(kctx, tk,  tokenizer->source + tok_start, 1);
	if(IS_NOTNULL(tk)) {
		kToken_set(StatementSeparator, tk, true);
	}
	return tok_start+1;
}

static int ParseAnnotation(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	// parse @Annotation:
	// This is part of JStyle dependent. If you don't want to parse, override this.
	if(isalnum(tokenizer->source[tok_start+1])) {
		int pos = ParseSymbol(kctx, tk, tokenizer, tok_start+1);
		if(IS_NOTNULL(tk)) {  // pre-resolved
			tk->resolvedSymbol = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID) | MN_Annotation;
			tk->resolvedSyntaxInfo = SYN_(tokenizer->ns, KW_SymbolPattern);
		}
		return pos;
	}
	return ParseSingleOperator(kctx, tk, tokenizer, tok_start);
}

//static KMETHOD TokenFunc_JavaStyleAnnotation(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TokenFunc(tk, source);
//	if(isalnum(tokenizer->source[tok_start+1])) {  // tokenizer, tok_start is older style of tokneizer
//		int pos = ParseSymbol(kctx, tk, tokenizer, tok_start+1);
//		if(IS_NOTNULL(tk)) {  // pre-resolved
//			tk->resolvedSymbol = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID) | MN_Annotation;
//			tk->resolvedSyntaxInfo = SYN_(tokenizer->ns, KW_SymbolPattern);
//		}
//		RETURNi_(pos - tok_start);
//	}
//	RETURNi_(0);
//}

static int ParseOperator(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start;
	while((ch = tokenizer->source[pos++]) != 0) {
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
	kToken_setSymbolText(kctx, tk, tokenizer->source + tok_start, (pos-1)-tok_start);
	return pos-1;
}

static int ParseLineComment(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start;
	while((ch = tokenizer->source[pos++]) != 0) {
		if(ch == '\n') break;
	}
	return pos-1;/*EOF*/
}

static int ParseCStyleBlockComment(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, prev = 0, level = 1, pos = tok_start + 2;
	/*@#nnnn is line number */
	if(tokenizer->source[pos] == '@' && tokenizer->source[pos+1] == '#' && isdigit(tokenizer->source[pos+2])) {
		tokenizer->currentLine >>= (sizeof(kshort_t)*8);
		tokenizer->currentLine = (tokenizer->currentLine<<(sizeof(kshort_t)*8))  | (kshort_t)strtoll(tokenizer->source + pos + 2, NULL, 10);
	}
	while((ch = tokenizer->source[pos++]) != 0) {
		if(ch == '\n') {
			tokenizer->currentLine += 1;
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

static int ParseSlash(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	const char *ts = tokenizer->source + tok_start;
	if(ts[1] == '/') {
		return ParseLineComment(kctx, tk, tokenizer, tok_start);
	}
	if(ts[1] == '*') {
		return ParseCStyleBlockComment(kctx, tk, tokenizer, tok_start);
	}
	return ParseOperator(kctx, tk, tokenizer, tok_start);
}

static int ParseDoubleQuotedText(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, prev = '"', prev2 = '\0', pos = tok_start + 1, hasUTF8 = false;
	while((ch = tokenizer->source[pos++]) != 0) {
		if(ch == '\n') {
			break;
		}
		if(ch == '"' && (prev != '\\' || (prev == '\\' && prev2 == '\\'))) {
			if(IS_NOTNULL(tk)) {
				size_t length = pos - (tok_start + 1) - 1;
				KFieldSet(tk, tk->text, KLIB new_kString(kctx, tokenizer->source + tok_start + 1, length, hasUTF8 ? StringPolicy_UTF8 : StringPolicy_ASCII));
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

static int ParseWhiteSpace(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	size_t size = kArray_size(tokenizer->tokenList);
	if(size > 0) {
		kTokenVar *tk = tokenizer->tokenList->tokenVarItems[size-1];
		if(tk->uline == tokenizer->currentLine && tk->unresolvedTokenType != TokenType_INDENT) {
			kToken_set(BeforeWhiteSpace, tk, true);
		}
	}
	return tok_start+1;
}

static int ParseSkip(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	return tok_start+1;
}

static int ParseUndefinedToken(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	if(IS_NOTNULL(tk)) {
		kToken_printMessage(kctx, tk, ErrTag, "undefined token character: %c (ascii=%x)", tokenizer->source[tok_start], tokenizer->source[tok_start]);
		while(tokenizer->source[++tok_start] != 0);
		return tok_start;
	}
	return tok_start+1;
}

static int ParseLazyBlock(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start);

static const TokenizeFunc MiniKonohaTokenMatrix[] = {
	ParseSkip,  /* KonohaChar_Null */
	ParseSkip,  /* KonohaChar_Undefined */
	ParseNumber,   /* KonohaChar_Digit */
	ParseSymbol,
	ParseSymbol,
	ParseUndefinedToken,  /* KonohaChar_Unicode */
	ParseLineFeed,
	ParseWhiteSpace,
	ParseWhiteSpace,      /* KonohaChar_Space */
	ParseSingleOperator, /* KonohaChar_OpenParenthesis */
	ParseSingleOperator, /* KonohaChar_CloseParenthesis */
	ParseSingleOperator, /* KonohaChar_OpenBracket */
	ParseSingleOperator, /* KonohaChar_CloseBracket */
	ParseLazyBlock, /* KonohaChar_OpenBrace */
	ParseSingleOperator, /* KonohaChar_CloseBrace */
	ParseOperator,  /* KonohaChar_LessThan */
	ParseOperator,  /* KonohaChar_LessThan */
	ParseUndefinedToken,  /* parseUndefinedToken */
	ParseDoubleQuotedText,  /* KonohaChar_DoubleQuote */
	ParseUndefinedToken,  /* KonohaChar_BackQuote */
	ParseOperator, /* KonohaChar_Okidoki */
	ParseOperator, /* KonohaChar_Sharp */
	ParseOperator, /* KonohaChar_Dollar */
	ParseOperator, /* KonohaCharKonohaChar_Percent */
	ParseOperator, /* KonohaChar_And */
	ParseOperator, /* KonohaChar_Star */
	ParseOperator, /* KonohaChar_Plus */
	ParseSingleOperator,/* KonohaChar_Comma */
	ParseOperator, /* KonohaChar_Minus */
	ParseSingleOperator, /* KonohaChar_Dot */
	ParseSlash, /* KonohaChar_Slash */
	ParseOperator, /* KonohaChar_Colon */
	ParseSemiColon, /* KonohaChar_SemiColon */
	ParseOperator, /* KonohaChar_Equal */
	ParseOperator, /* KonohaChar_Question */
	ParseAnnotation, /* KonohaChar_AtMark */
	ParseOperator, /* KonohaChar_Var */
	ParseOperator, /* KonohaChar_Childer */
	ParseBackSlash/*ParseUndefinedToken*/,  /* KonohaChar_BackSlash */
	ParseOperator, /* KonohaChar_Hat */
	ParseSymbol,  /* KonohaChar_Underbar */
};


static int callTokenFunc(KonohaContext *kctx, kFunc *fo, kTokenVar *tk, Tokenizer *tokenizer, kStringVar *preparedString, int tok_start)
{
	DBG_ASSERT(IS_Func(fo));
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
	KUnsafeFieldSet(lsfp[K_CALLDELTA+0].o, fo->self);
	KUnsafeFieldSet(lsfp[K_CALLDELTA+1].o, (kObject*)tk);
	lsfp[K_CALLDELTA+1].unboxValue = (uintptr_t)tokenizer;
	KUnsafeFieldSet(lsfp[K_CALLDELTA+2].s, preparedString);
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KSetMethodCallStack(sfp, 0/*UL*/, fo->mtd, 2, KLIB Knull(kctx, CT_Int));
		KonohaRuntime_callMethod(kctx, sfp);
	}
	END_LOCAL();
	int pos = lsfp[0].intValue + tok_start;
	if(pos > tok_start && tokenizer->currentLine > 0) { // check new lines
		int i;
		const char *t = tokenizer->source;
		for(i = tok_start; i < pos; i++) {
			if(t[i] == '\n') tokenizer->currentLine += 1;
		}
	}
	return pos;
}

static kFunc** Tokenizer_funcTable(KonohaContext *kctx, Tokenizer *tokenizer, int kchar, int *sizeRef)
{
	kFunc *fo = tokenizer->funcItems[kchar];
	if(IS_Array(fo)) {
		sizeRef[0] =  kArray_size(tokenizer->funcListItems[kchar]);
		return tokenizer->funcListItems[kchar]->funcItems;
	}
	sizeRef[0] = 1;
	return (kFunc**)&(tokenizer->funcItems[kchar]);
}

static int Tokenizer_doEach(KonohaContext *kctx, Tokenizer *tokenizer, int kchar, int tok_start, kTokenVar* tk)
{
	int pos;
	if(tokenizer->funcItems[kchar] != NULL) {
		int i, size;
		kFunc **funcItems = Tokenizer_funcTable(kctx, tokenizer, kchar, &size);
		kStringVar *preparedString = (kStringVar*)tokenizer->preparedString;
		preparedString->text = tokenizer->source + tok_start;   // this is a really bad manner !!
		preparedString->bytesize = tokenizer->sourceLength - tok_start;
		for(i = size - 1; i >= 0; i--) {
			pos = callTokenFunc(kctx, funcItems[i], tk, tokenizer, preparedString, tok_start);
			if(pos > tok_start) {
				return pos;
			}
		}
	}
	pos = tokenizer->cfuncItems[kchar](kctx, tk, tokenizer, tok_start);
	return pos;
}

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

static void Tokenizer_tokenize(KonohaContext *kctx, Tokenizer *tokenizer)
{
	int ch, pos = 0;
	kTokenVar *tk = GCSAFE_new(TokenVar, 0);
	pos = ParseIndent(kctx, tk, tokenizer, pos);
	while((ch = AsciiToKonohaChar(tokenizer->source[pos])) != 0) {
		if(tk->unresolvedTokenType != 0) {
			KLIB kArray_add(kctx, tokenizer->tokenList, tk);
			tk = GCSAFE_new(TokenVar, 0);
			tk->uline = tokenizer->currentLine;
		}
		pos = Tokenizer_doEach(kctx, tokenizer, ch, pos, tk);
	}
	if(tk->unresolvedTokenType != 0) {
		KLIB kArray_add(kctx, tokenizer->tokenList, tk);
	}
}

static int ParseLazyBlock(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, level = 1, pos = tok_start + 1;
	while((ch = AsciiToKonohaChar(tokenizer->source[pos])) != 0) {
		if(ch == KonohaChar_CloseBrace/*}*/) {
			level--;
			if(level == 0) {
				if(IS_NOTNULL(tk)) {
					KFieldSet(tk, tk->text, KLIB new_kString(kctx, tokenizer->source + tok_start + 1, ((pos-2)-(tok_start)+1), 0));
					tk->unresolvedTokenType = TokenType_CODE;
				}
				return pos + 1;
			}
			pos++;
		}
		else if(ch == KonohaChar_OpenBrace/*'{'*/) {
			level++; pos++;
		}
		else {
			pos = Tokenizer_doEach(kctx, tokenizer, ch, pos, (kTokenVar*)K_NULLTOKEN);
		}
	}
	ERROR_UnclosedToken(kctx, tk, "}");
	return pos-1;
}

static const TokenizeFunc *kNameSpace_tokenMatrix(KonohaContext *kctx, kNameSpace *ns)
{
	if(ns->tokenMatrix == NULL) {
		//DBG_ASSERT(KCHAR_MAX * sizeof(TokenizeFunc) == sizeof(MiniKonohaTokenMatrix));
		TokenizeFunc *tokenMatrix = (TokenizeFunc*)KMALLOC(SIZEOF_TOKENMATRIX);
		if(ns->parentNULL != NULL && ns->parentNULL->tokenMatrix != NULL) {
			memcpy(tokenMatrix, ns->parentNULL->tokenMatrix, sizeof(MiniKonohaTokenMatrix));
			bzero(tokenMatrix + KCHAR_MAX, sizeof(MiniKonohaTokenMatrix));
			/* TODO:  import parent tokenFunc */
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

static void TokenSequence_tokenize(KonohaContext *kctx, TokenSequence *tokens, const char *source, kfileline_t uline)
{
	Tokenizer tenv = {};
	tenv.source = source;
	tenv.sourceLength = strlen(source);
	tenv.currentLine  = uline;
	tenv.tokenList    = tokens->tokenList;
	tenv.tabsize = 4;
	tenv.ns = tokens->ns;
	tenv.cfuncItems = kNameSpace_tokenMatrix(kctx, tokens->ns);
	tenv.funcItems  = kNameSpace_tokenFuncMatrix(kctx, tokens->ns);
	{
		INIT_GCSTACK();
		kString *preparedString = KLIB new_kString(kctx, tenv.source, tenv.sourceLength, StringPolicy_ASCII|StringPolicy_TEXT|StringPolicy_NOPOOL);
		PUSH_GCSTACK(preparedString);
		tenv.preparedString = preparedString;
		Tokenizer_tokenize(kctx, &tenv);
		RESET_GCSTACK();
	}
	TokenSequence_end(kctx, tokens);
}

#ifdef __cplusplus
}
#endif
