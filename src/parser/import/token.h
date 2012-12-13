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
		kToken_ToError(kctx, tk, ErrTag, "must close with %s", ch);
	}
}

static int ParseFirstIndent(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int pos)
{
	int ch, indent = 0;
	while((ch = tokenizer->source[pos++]) != 0) {
		if(ch == '\t') { indent += tokenizer->tabsize; }
		else if(ch == ' ') { indent += 1; }
		break;
	}
	if(IS_NOTNULL(tk) && (indent > 0 || tokenizer->currentLine != 0)) {
		tk->unresolvedTokenType = TokenType_INDENT;
		tk->indent = indent;
		tk->uline = tokenizer->currentLine;
	}
	return pos-1;
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
		return ParseIndent(kctx, (kTokenVar *)KNULL(Token), tokenizer, pos+2);
	}
	return pos+1;
}

static int ParseNumber(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start, tokenType = TokenType_INT;
	const char *ts = tokenizer->source;
	while((ch = ts[pos++]) != 0) {
		if(ch == '.') {
			if(isalnum(ts[pos])) {
				tokenType = SYM_("$Float");
				continue;
			}
		}
		if(!isalnum(ch)) break;
	}
	if(IS_NOTNULL(tk)) {
		KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, ts + tok_start, (pos-1)-tok_start, StringPolicy_ASCII));
		tk->unresolvedTokenType = tokenType;
	}
	return pos - 1;  // next
}

static void kToken_setSymbolText(KonohaContext *kctx, kTokenVar *tk, const char *t, size_t len)
{
	if(IS_NOTNULL(tk)) {
		ksymbol_t kw = ksymbolA(t, len, SYM_NONAME);
		if(kw == Symbol_Unmask(kw)) {
			KFieldSet(tk, tk->text, SYM_s(kw));
		}
		else {
			KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, t, len, StringPolicy_ASCII));
		}
		tk->unresolvedTokenType = TokenType_SYMBOL;
		if(len == 1) {
			kToken_SetHintChar(tk, 0, t[0]);
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
			tk->resolvedSymbol = ksymbolA(kString_text(tk->text), kString_size(tk->text), SYM_NEWID) | MN_Annotation;
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
//			tk->resolvedSymbol = ksymbolA(kString_text(tk->text), kString_size(tk->text), SYM_NEWID) | MN_Annotation;
//			tk->resolvedSyntaxInfo = SYN_(tokenizer->ns, KW_SymbolPattern);
//		}
//		KReturnUnboxValue(pos - tok_start);
//	}
//	KReturnUnboxValue(0);
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

static int skipBackQuoteOrNewLineOrDoubleQuote(const char *source, int *posPtr, int *hasUTF8)
{
	char ch;
	int pos = *posPtr;
	for(ch = source[pos++]; ch != 0; ch = source[pos++]) {
		if(ch < 0) {
			*hasUTF8 = 1;
		}
		if(ch == '\\' || ch == '\n' || ch == '"') {
			*posPtr = pos;
			return ch;
		}
	}
	*posPtr = pos;
	return ch;
}

static int ParseDoubleQuotedText(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int pos = tok_start + 1, hasUTF8 = false, hasBQ = false;
	int ch = 0;
	while(true) {
		ch = skipBackQuoteOrNewLineOrDoubleQuote(tokenizer->source, &pos, &hasUTF8);
		if(ch == 0 || ch == '"' || ch == '\n') {
			break;
		}
		else {
			assert(ch == '\\');
			hasBQ = 1;
			pos++;
		}
	}
	if(hasBQ && IS_NOTNULL(tk)) {
		kToken_set(RequiredReformat, tk, true);
	}
	if(ch == '"') {
		if(IS_NOTNULL(tk)) {
			size_t length = pos - (tok_start + 1) - 1;
			kString *text = KLIB new_kString(kctx, OnField,
					tokenizer->source + tok_start + 1, length,
					hasUTF8 ? StringPolicy_UTF8 : StringPolicy_ASCII);
			KFieldSet(tk, tk->text, text);
			tk->unresolvedTokenType = TokenType_TEXT;
		}
		return pos;
	}
	ERROR_UnclosedToken(kctx, tk, "\"");
	return pos - 1;
}

static int ParseWhiteSpace(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	size_t size = kArray_size(tokenizer->tokenList);
	if(size > 0) {
		kTokenVar *tk = tokenizer->tokenList->TokenVarItems[size-1];
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
		kToken_ToError(kctx, tk, ErrTag, "undefined token character: %c (ascii=%x)", tokenizer->source[tok_start], tokenizer->source[tok_start]);
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

static void CallSugarMethod(KonohaContext *kctx, KonohaStack *sfp, kFunc *fo, int argc, kObject *obj)
{
	KStackSetFuncAll(sfp, obj, 0, fo, argc);
	KLIB KonohaRuntime_tryCallMethod(kctx, sfp);
}

static int CallTokenFunc(KonohaContext *kctx, kFunc *fo, kTokenVar *tk, Tokenizer *tokenizer, kStringVar *preparedString, int tok_start)
{
	DBG_ASSERT(IS_Func(fo));
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[0].asNameSpace, tokenizer->ns);
	KUnsafeFieldSet(lsfp[1].asToken, tk);
	lsfp[1].unboxValue = (uintptr_t)tokenizer;
	KUnsafeFieldSet(lsfp[2].asString, preparedString);
	CallSugarMethod(kctx, lsfp, fo, 2, KLIB Knull(kctx, CT_Int));
	END_UnusedStack();
	int pos = lsfp[K_RTNIDX].intValue + tok_start;
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
	kFunc *fo = tokenizer->FuncItems[kchar];
	if(IS_Array(fo)) {
		sizeRef[0] =  kArray_size(tokenizer->funcListItems[kchar]);
		return tokenizer->funcListItems[kchar]->FuncItems;
	}
	sizeRef[0] = 1;
	return (kFunc**)&(tokenizer->FuncItems[kchar]);
}

static int Tokenizer_DoEach(KonohaContext *kctx, Tokenizer *tokenizer, int kchar, int tok_start, kTokenVar* tk)
{
	int pos;
	if(tokenizer->FuncItems[kchar] != NULL) {
		int i, size;
		kFunc **FuncItems = Tokenizer_funcTable(kctx, tokenizer, kchar, &size);
		kStringVar *preparedString = (kStringVar *)tokenizer->preparedString;
		preparedString->text = tokenizer->source + tok_start;   // this is a really bad manner !!
		preparedString->bytesize = tokenizer->sourceLength - tok_start;
		for(i = size - 1; i >= 0; i--) {
			pos = CallTokenFunc(kctx, FuncItems[i], tk, tokenizer, preparedString, tok_start);
			if(pos > tok_start) {
				return pos;
			}
		}
	}
	pos = tokenizer->cFuncItems[kchar](kctx, tk, tokenizer, tok_start);
	return pos;
}


static void Tokenizer_Tokenize(KonohaContext *kctx, Tokenizer *tokenizer)
{
	int ch, pos = 0;
	kTokenVar *tk = new_(TokenVar, 0, OnGcStack);
	pos = ParseFirstIndent(kctx, tk, tokenizer, pos);
	while((ch = AsciiToKonohaChar(tokenizer->source[pos])) != 0) {
		if(tk->unresolvedTokenType != 0) {
			KLIB kArray_Add(kctx, tokenizer->tokenList, tk);
			tk = new_(TokenVar, 0, OnGcStack);
			tk->uline = tokenizer->currentLine;
		}
		pos = Tokenizer_DoEach(kctx, tokenizer, ch, pos, tk);
	}
	if(tk->unresolvedTokenType != 0) {
		KLIB kArray_Add(kctx, tokenizer->tokenList, tk);
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
					KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, tokenizer->source + tok_start + 1, ((pos-2)-(tok_start)+1), 0));
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
			pos = Tokenizer_DoEach(kctx, tokenizer, ch, pos, (kTokenVar *)K_NULLTOKEN);
		}
	}
	ERROR_UnclosedToken(kctx, tk, "}");
	return pos-1;
}

static const TokenizeFunc *kNameSpace_tokenMatrix(KonohaContext *kctx, kNameSpace *ns)
{
	if(ns->tokenMatrix == NULL) {
		//DBG_ASSERT(KCHAR_MAX * sizeof(TokenizeFunc) == sizeof(MiniKonohaTokenMatrix));
		TokenizeFunc *tokenMatrix = (TokenizeFunc *)KMalloc_UNTRACE(SIZEOF_TOKENMATRIX);
		if(ns->parentNULL != NULL && ns->parentNULL->tokenMatrix != NULL) {
			memcpy(tokenMatrix, ns->parentNULL->tokenMatrix, sizeof(MiniKonohaTokenMatrix));
			bzero(tokenMatrix + KCHAR_MAX, sizeof(MiniKonohaTokenMatrix));
			/* TODO:  import parent tokenFunc */
		}
		else {
			memcpy(tokenMatrix, MiniKonohaTokenMatrix, sizeof(MiniKonohaTokenMatrix));
			bzero(tokenMatrix + KCHAR_MAX, sizeof(MiniKonohaTokenMatrix));
		}
		((kNameSpaceVar *)ns)->tokenMatrix = (void *)tokenMatrix;
	}
	return (TokenizeFunc *)ns->tokenMatrix;
}

static kFunc **kNameSpace_tokenFuncMatrix(KonohaContext *kctx, kNameSpace *ns)
{
	kFunc **funcMatrix = (kFunc**)kNameSpace_tokenMatrix(kctx, ns);
	return funcMatrix + KCHAR_MAX;
}

static void TokenSeq_Tokenize(KonohaContext *kctx, TokenSeq *tokens, const char *source, kfileline_t uline)
{
	INIT_GCSTACK();
	Tokenizer tenv = {};
	tenv.source = source;
	tenv.sourceLength = strlen(source);
	tenv.currentLine  = uline;
	tenv.tokenList    = tokens->tokenList;
	tenv.tabsize = 4;
	tenv.ns = tokens->ns;
	tenv.cFuncItems = kNameSpace_tokenMatrix(kctx, tokens->ns);
	tenv.FuncItems  = kNameSpace_tokenFuncMatrix(kctx, tokens->ns);
	tenv.preparedString = KLIB new_kString(kctx, _GcStack, tenv.source, tenv.sourceLength, StringPolicy_ASCII|StringPolicy_TEXT|StringPolicy_NOPOOL);
	Tokenizer_Tokenize(kctx, &tenv);
	TokenSeq_End(kctx, tokens);
	RESET_GCSTACK();
}

#ifdef __cplusplus
}
#endif
