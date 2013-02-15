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

#define KCheckStringPolicy(ch, policy)  if(ch < 0) { policy = StringPolicy_UTF8; }
static void kToken_SetParsedText(KonohaContext *kctx, kTokenVar *tk, ksymbol_t tokenType, const char *t, size_t len, int policy)
{
	if(IS_NOTNULL(tk)) {
		KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, t, len, policy));
		tk->tokenType = tokenType;
	}
}

static int CountIndent(KonohaContext *kctx, Tokenizer *tokenizer, int pos, int *indentRef)
{
	int ch, indent = 0;
	for(; (ch = tokenizer->source[pos]) != 0; pos++) {
		if(ch == '\t') { indent += tokenizer->tabsize; }
		else if(ch == ' ') { indent += 1; }
		break;
	}
	indentRef[0] = indent;
	return pos;
}

static int TokenizeFirstIndent(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int pos)
{
	int indent = 0;
	pos = CountIndent(kctx, tokenizer, pos, &indent);
	if(IS_NOTNULL(tk) && (indent > 0 || tokenizer->currentLine != 0)) {
		tk->tokenType = TokenType_Indent;
		tk->indent = indent;
		tk->uline = tokenizer->currentLine;
	}
	return pos;
}

static int SkipBlockIndent(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, indent, pos = tok_start;
	int policy = StringPolicy_ASCII;
	for(; (ch = tokenizer->source[pos]) != 0; pos++) {
		KCheckStringPolicy(ch, policy);
		if(ch == '\n') {
			pos = CountIndent(kctx, tokenizer, pos+1, &indent);
			if(indent <= tokenizer->baseIndent) break;
		}
	}
	kToken_SetParsedText(kctx, tk, TokenType_Indent, tokenizer->source + tok_start, pos - tok_start, policy);
	return pos;
}

static int TokenizeIndent(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int pos)
{
	int indent = 0;
	pos = CountIndent(kctx, tokenizer, pos, &indent);
	if(IS_NOTNULL(tk)) {
		tk->tokenType = TokenType_Indent;
		tk->indent = indent;
		tk->uline = tokenizer->currentLine;
		if(indent > tokenizer->baseIndent) {
			//DBG_P(">>>>>>>> baseindent=%d, indent=%d", tokenizer->baseIndent, indent);
			pos = SkipBlockIndent(kctx, tk, tokenizer, pos);
		}
	}
	else {
		if(tokenizer->baseIndent == 0) {
			tokenizer->baseIndent = indent;
		}
	}
	return pos;
}

static int TokenizeLineFeed(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int pos)
{
	if(tokenizer->currentLine != 0) {
		tokenizer->currentLine += 1;
	}
	return TokenizeIndent(kctx, tk, tokenizer, pos+1);
}

static int TokenizeBackSlash(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int pos)
{
	if(tokenizer->source[pos+1] == '\n') {
		if(tokenizer->currentLine != 0) {
			tokenizer->currentLine += 1;
		}
		return TokenizeIndent(kctx, (kTokenVar *)KNULL(Token), tokenizer, pos+2);
	}
	return pos+1;
}

static int TokenizeNumber(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start, tokenType = TokenType_Number;
	const char *ts = tokenizer->source;
	int policy = StringPolicy_ASCII;
	for(; (ch = ts[pos]) != 0; pos++) {
		KCheckStringPolicy(ch, policy);
		if(ch == '.') {
			if(isalnum(ts[pos+1])) {
				tokenType = KSymbol_("$Float");
				continue;
			}
		}
		if(!isalnum(ch)) break;
	}
	kToken_SetParsedText(kctx, tk, tokenType, ts + tok_start, (pos)-tok_start, policy);
	return pos;  // next
}

static void kToken_SetSymbolText(KonohaContext *kctx, kTokenVar *tk, ksymbol_t tokenType, const char *t, size_t len, int policy)
{
	if(IS_NOTNULL(tk)) {
		ksymbol_t symbol = KAsciiSymbol(t, len, _NEWID);
		if(symbol == KSymbol_Unmask(symbol)) {
			KFieldSet(tk, tk->text, KSymbol_GetString(kctx, symbol));
		}
		else {
			KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, t, len, policy));
		}
		tk->tokenType = tokenType;
		tk->symbol = symbol;
	}
}

static int TokenizeSymbol(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start;
	const char *ts = tokenizer->source;
	int policy = StringPolicy_ASCII;
	for(; (ch = ts[pos]) != 0; pos++) {
		KCheckStringPolicy(ch, policy);
		if(ch == '_' || isalnum(ch)) continue; // nothing
		break;
	}
	kToken_SetSymbolText(kctx, tk, TokenType_Symbol, ts + tok_start, (pos)-tok_start, policy);
	return pos;  // next
}

static int TokenizeSingleOperator(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	kToken_SetSymbolText(kctx, tk,  TokenType_Symbol, tokenizer->source + tok_start, 1, StringPolicy_ASCII);
	return tok_start+1;
}

static int TokenizeOpenParenthesis(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	if(IS_NOTNULL(tk)) {  // pre-resolved
		kToken_SetSymbolText(kctx, tk, KSymbol_ParenthesisGroup, tokenizer->source + tok_start, 1, StringPolicy_ASCII);
		kToken_Set(OpenGroup, tk, true);
	}
	return tok_start+1;
}

static int TokenizeCloseParenthesis(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	if(IS_NOTNULL(tk)) {  // pre-resolved
		kToken_SetSymbolText(kctx, tk, KSymbol_ParenthesisGroup, tokenizer->source + tok_start, 1, StringPolicy_ASCII);
		kToken_Set(CloseGroup, tk, true);
	}
	return tok_start+1;
}

static int TokenizeOpenBracket(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	if(IS_NOTNULL(tk)) {  // pre-resolved
		kToken_SetSymbolText(kctx, tk, KSymbol_BracketGroup, tokenizer->source + tok_start, 1, StringPolicy_ASCII);
		kToken_Set(OpenGroup, tk, true);
	}
	return tok_start+1;
}

static int TokenizeCloseBracket(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	if(IS_NOTNULL(tk)) {  // pre-resolved
		kToken_SetSymbolText(kctx, tk, KSymbol_BracketGroup, tokenizer->source + tok_start, 1, StringPolicy_ASCII);
		kToken_Set(CloseGroup, tk, true);
	}
	return tok_start+1;
}

static int TokenizeAnnotation(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	// parse @Annotation:
	// This is part of JStyle dependent. If you don't want to parse, override this.
	if(isalnum(tokenizer->source[tok_start+1])) {
		int pos = TokenizeSymbol(kctx, tk, tokenizer, tok_start+1);
		if(IS_NOTNULL(tk)) {  // pre-resolved
			tk->symbol = tk->symbol | KSymbolAttr_Annotation;
		}
		return pos;
	}
	return TokenizeSingleOperator(kctx, tk, tokenizer, tok_start);
}

//static KMETHOD TokenFunc_JavaStyleAnnotation(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TokenFunc(tk, source);
//	if(isalnum(tokenizer->source[tok_start+1])) {  // tokenizer, tok_start is older style of tokneizer
//		int pos = TokenizeSymbol(kctx, tk, tokenizer, tok_start+1);
//		if(IS_NOTNULL(tk)) {  // pre-resolved
//			tk->symbol = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewId) | KSymbolAttr_Annotation;
//			tk->resolvedSyntaxInfo = kSyntax_(tokenizer->ns, KSymbol_SymbolPattern);
//		}
//		KReturnUnboxValue(pos - tok_start);
//	}
//	KReturnUnboxValue(0);
//}

static int TokenizeOperator(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start;
	for(; (ch = tokenizer->source[pos]) != 0; pos++) {
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
	kToken_SetSymbolText(kctx, tk, KSymbol_SymbolPattern, tokenizer->source + tok_start, (pos)-tok_start, StringPolicy_ASCII);
	return pos;
}

static int TokenizeLineComment(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, pos = tok_start;
	for(; (ch = tokenizer->source[pos]) != 0; pos++) {
		if(ch == '\n') break;
	}
	return pos;/*EOF*/
}

static int TokenizeCStyleBlockComment(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, prev = 0, level = 1, pos = tok_start + 2;
	/* @#nnnn is a notation to reset its line number */
	if(tokenizer->source[pos] == '@' && tokenizer->source[pos+1] == '#' && isdigit(tokenizer->source[pos+2])) {
		tokenizer->currentLine >>= (sizeof(kshort_t)*8);
		tokenizer->currentLine = (tokenizer->currentLine<<(sizeof(kshort_t)*8))  | (kshort_t)strtoll(tokenizer->source + pos + 2, NULL, 10);
	}
	for(; (ch = tokenizer->source[pos]) != 0; pos++) {
		if(ch == '\n') {
			tokenizer->currentLine += 1;
		}
		if(prev == '*' && ch == '/') {
			level--;
			if(level == 0) return pos+1;
		} else if(prev == '/' && ch == '*') {
			level++;
		}
		prev = ch;
	}
	ERROR_UnclosedToken(kctx, tk, "*/");
	return pos;/*EOF*/
}

static int TokenizeSlash(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	const char *ts = tokenizer->source + tok_start;
	if(ts[1] == '/') {
		return TokenizeLineComment(kctx, tk, tokenizer, tok_start);
	}
	if(ts[1] == '*') {
		return TokenizeCStyleBlockComment(kctx, tk, tokenizer, tok_start);
	}
	return TokenizeOperator(kctx, tk, tokenizer, tok_start);
}

static int TokenizeMember(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	const char *ts = tokenizer->source + tok_start;
	if(isalpha(ts[1]) || ts[1] == '_' || ts[1] < 0) {
		int pos = TokenizeSymbol(kctx, tk, tokenizer, tok_start+1);
		tk->tokenType = TokenType_Member;
		return pos;
	}
	if(isdigit(ts[1])) {
		return TokenizeNumber(kctx, tk, tokenizer, tok_start);
	}
	return TokenizeOperator(kctx, tk, tokenizer, tok_start);
}

static int SkipBackQuoteOrNewLineOrDoubleQuote(const char *source, int *posPtr, int *hasUTF8)
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

static int TokenizeDoubleQuotedText(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int pos = tok_start + 1, hasUTF8 = false, hasBQ = false;
	int ch = 0;
	while(true) {
		ch = SkipBackQuoteOrNewLineOrDoubleQuote(tokenizer->source, &pos, &hasUTF8);
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
		kToken_Set(RequiredReformat, tk, true);
	}
	if(ch == '"') {
		if(IS_NOTNULL(tk)) {
			size_t length = pos - (tok_start + 1) - 1;
			kString *text = KLIB new_kString(kctx, OnField,
					tokenizer->source + tok_start + 1, length,
					hasUTF8 ? StringPolicy_UTF8 : StringPolicy_ASCII);
			KFieldSet(tk, tk->text, text);
			tk->tokenType = TokenType_Text;
		}
		return pos;
	}
	ERROR_UnclosedToken(kctx, tk, "\"");
	return pos - 1;
}

static int TokenizeWhiteSpace(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	size_t size = kArray_size(tokenizer->tokenList);
	if(size > 0) {
		kTokenVar *tk = tokenizer->tokenList->TokenVarItems[size-1];
		if(tk->uline == tokenizer->currentLine && tk->tokenType != TokenType_Indent) {
			kToken_Set(BeforeWhiteSpace, tk, true);
		}
	}
	return tok_start+1;
}

static int TokenizeSkip(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	return tok_start+1;
}

static int TokenizeUndefinedToken(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	if(IS_NOTNULL(tk)) {
		kToken_ToError(kctx, tk, ErrTag, "undefined token character: %c (ascii=%x)", tokenizer->source[tok_start], tokenizer->source[tok_start]);
		while(tokenizer->source[++tok_start] != 0);
		return tok_start;
	}
	return tok_start+1;
}

static int TokenizeLazyBlock(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start);

static const TokenizeFunc MiniKonohaTokenMatrix[] = {
	TokenizeSkip,  /* KonohaChar_Null */
	TokenizeSkip,  /* KonohaChar_Undefined */
	TokenizeNumber,   /* KonohaChar_Digit */
	TokenizeSymbol,
	TokenizeSymbol,
	TokenizeUndefinedToken,  /* KonohaChar_Unicode */
	TokenizeLineFeed,
	TokenizeWhiteSpace,
	TokenizeWhiteSpace,      /* KonohaChar_Space */
	TokenizeOpenParenthesis, /*TokenizeSingleOperator,  KonohaChar_OpenParenthesis */
	TokenizeCloseParenthesis, /*TokenizeSingleOperator,  KonohaChar_CloseParenthesis */
	TokenizeOpenBracket, /*TokenizeSingleOperator, KonohaChar_OpenBracket */
	TokenizeCloseBracket, /*TokenizeSingleOperator, KonohaChar_CloseBracket */
	TokenizeLazyBlock, /* KonohaChar_OpenBrace */
	TokenizeSingleOperator, /* KonohaChar_CloseBrace */
	TokenizeOperator,  /* KonohaChar_LessThan */
	TokenizeOperator,  /* KonohaChar_LessThan */
	TokenizeUndefinedToken,  /* parseUndefinedToken */
	TokenizeDoubleQuotedText,  /* KonohaChar_DoubleQuote */
	TokenizeUndefinedToken,  /* KonohaChar_BackQuote */
	TokenizeOperator, /* KonohaChar_Okidoki */
	TokenizeOperator, /* KonohaChar_Sharp */
	TokenizeOperator, /* KonohaChar_Dollar */
	TokenizeOperator, /* KonohaCharKonohaChar_Percent */
	TokenizeOperator, /* KonohaChar_And */
	TokenizeOperator, /* KonohaChar_Star */
	TokenizeOperator, /* KonohaChar_Plus */
	TokenizeSingleOperator,/* KonohaChar_Comma */
	TokenizeOperator, /* KonohaChar_Minus */
	TokenizeMember/*TokenizeSingleOperator*/, /* KonohaChar_Dot */
	TokenizeSlash, /* KonohaChar_Slash */
	TokenizeOperator, /* KonohaChar_Colon */
	TokenizeSingleOperator, /* SemiColon, KonohaChar_SemiColon */
	TokenizeOperator, /* KonohaChar_Equal */
	TokenizeOperator, /* KonohaChar_Question */
	TokenizeAnnotation, /* KonohaChar_AtMark */
	TokenizeOperator, /* KonohaChar_Var */
	TokenizeOperator, /* KonohaChar_Childer */
	TokenizeBackSlash, /* KonohaChar_BackSlash */
	TokenizeOperator, /* KonohaChar_Hat */
	TokenizeSymbol,  /* KonohaChar_Underbar */
};

static void CallSugarMethod(KonohaContext *kctx, KonohaStack *sfp, kFunc *fo, int argc, kObject *obj)
{
	KStackSetFuncAll(sfp, obj, 0, fo, argc);
	KLIB KRuntime_tryCallMethod(kctx, sfp);
}

static int CallTokenFunc(KonohaContext *kctx, kFunc *fo, kTokenVar *tk, Tokenizer *tokenizer, kStringVar *preparedString, int tok_start)
{
	DBG_ASSERT(IS_Func(fo));
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[0].asNameSpace, tokenizer->ns);
	KUnsafeFieldSet(lsfp[1].asToken, tk);
	lsfp[1].unboxValue = (uintptr_t)tokenizer;
	KUnsafeFieldSet(lsfp[2].asString, preparedString);
	CallSugarMethod(kctx, lsfp, fo, 2, KLIB Knull(kctx, KClass_Int));
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

static kFunc **Tokenizer_funcTable(KonohaContext *kctx, Tokenizer *tokenizer, int kchar, int *sizeRef)
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
	return tokenizer->cFuncItems[kchar](kctx, tk, tokenizer, tok_start);
}

static void Tokenizer_Tokenize(KonohaContext *kctx, Tokenizer *tokenizer)
{
	int ch, pos = 0;
	kTokenVar *tk = new_(TokenVar, 0, OnGcStack);
	pos = TokenizeFirstIndent(kctx, tk, tokenizer, pos);
	while((ch = AsciiToKonohaChar(tokenizer->source[pos])) != 0) {
		if(tk->tokenType != 0) {
			KLIB kArray_Add(kctx, tokenizer->tokenList, tk);
			tk = new_(TokenVar, 0, OnGcStack);
			tk->uline = tokenizer->currentLine;
		}
		pos = Tokenizer_DoEach(kctx, tokenizer, ch, pos, tk);
	}
	if(tk->tokenType != 0) {
		KLIB kArray_Add(kctx, tokenizer->tokenList, tk);
	}
}

static int TokenizeLazyBlock(KonohaContext *kctx, kTokenVar *tk, Tokenizer *tokenizer, int tok_start)
{
	int ch, level = 1, pos = tok_start + 1;
	int baseIndent = tokenizer->baseIndent;
	tokenizer->baseIndent = 0;
	while((ch = AsciiToKonohaChar(tokenizer->source[pos])) != 0) {
		if(ch == KonohaChar_CloseBrace/*}*/) {
			level--;
			if(level == 0) {
				if(IS_NOTNULL(tk)) {
					KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, tokenizer->source + tok_start + 1, ((pos-2)-(tok_start)+1), 0));
					tk->tokenType = TokenType_LazyBlock;
					tk->indent = tokenizer->baseIndent;
					//DBG_P(">>>>>>>>>>>>>>>>>>>>>> baseIndent=%d, '''%s'''", tk->indent, kString_text(tk->text));
					tokenizer->baseIndent = baseIndent;
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
	tokenizer->baseIndent = baseIndent;
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

static void Tokenize(KonohaContext *kctx, kNameSpace *ns, const char *source, kfileline_t uline, int baseIndent, kArray *bufferList)
{
	INIT_GCSTACK();
	Tokenizer tenv = {};
	tenv.source = source;
	tenv.sourceLength = strlen(source);
	tenv.currentLine  = uline;
	tenv.tokenList    = bufferList;
	tenv.tabsize = 4;
	tenv.baseIndent = baseIndent;
	//DBG_P("<<<<<<<< baseIndent=%d, '''%s'''", baseIndent, source);
	tenv.ns = ns;
	tenv.cFuncItems = kNameSpace_tokenMatrix(kctx, ns);
	tenv.FuncItems  = kNameSpace_tokenFuncMatrix(kctx, ns);
	tenv.preparedString = KLIB new_kString(kctx, _GcStack, tenv.source, tenv.sourceLength, StringPolicy_ASCII|StringPolicy_TEXT|StringPolicy_NOPOOL);
	Tokenizer_Tokenize(kctx, &tenv);
	RESET_GCSTACK();
}

#ifdef __cplusplus
}
#endif
