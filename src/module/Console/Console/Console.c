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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#if defined(__MINGW32__) || defined(_MSC_VER)
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#if !defined(__CYGWIN__)
#include <sys/ttydefaults.h>
#endif
#endif
#include <konoha3/konoha.h>
#include <konoha3/klib.h>

// -------------------------------------------------------------------------
/* Console */

static const char *BeginTag(KonohaContext *kctx, kinfotag_t t)
{
	static const char *tags[] = {
		"\x1b[1m\x1b[31m", /*CritTag*/
		"\x1b[1m\x1b[31m", /*ErrTag*/
		"\x1b[1m\x1b[31m", /*WarnTag*/
		"\x1b[1m", /*NoticeTag*/
		"\x1b[1m", /*InfoTag*/
		"", /*DebugTag*/
		"", /* NoneTag*/
	};
	DBG_ASSERT(t <= NoneTag);
	if(!KonohaContext_Is(Interactive, kctx)) t = NoneTag;
	return tags[(int)t];
}

static const char *EndTag(KonohaContext *kctx, kinfotag_t t)
{
	static const char *tags[] = {
			"\x1b[0m", /*CritTag*/
			"\x1b[0m", /*ErrTag*/
			"\x1b[0m", /*WarnTag*/
			"\x1b[0m", /*NoticeTag*/
			"\x1b[0m", /*InfoTag*/
			"", /* Debug */
			"", /* NoneTag*/
	};
	DBG_ASSERT(t <= NoneTag);
	if(!KonohaContext_Is(Interactive, kctx)) t = NoneTag;
	return tags[(int)t];
}

static void UI_ReportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{
	const char *beginTag = BeginTag(kctx, level);
	const char *endTag = EndTag(kctx, level);
	const char *kLF = isNewLine ? "\n" : "";
	if(pline > 0) {
		const char *file = KFileLine_textFileName(pline);
		PLATAPI printf_i("%s - (%s:%d) %s%s%s" , beginTag, PLATAPI shortFilePath(file), (kushort_t)pline, msg, kLF, endTag);
	}
	else {
		PLATAPI printf_i("%s%s%s%s", beginTag,  msg, kLF, endTag);
	}
}

static void UI_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
	const char *beginTag = BeginTag(kctx, taglevel);
	const char *endTag = EndTag(kctx, taglevel);
	PLATAPI printf_i("%s - %s%s\n", beginTag, msg, endTag);
}

static void KBuffer_WriteValue(KonohaContext *kctx, KBuffer *wb, KClass *c, KonohaStack *sfp)
{
	if(KClass_Is(UnboxType, c)) {
		c->format(kctx, sfp, 0, wb);
	}
	else {
		KLIB kObject_WriteToBuffer(kctx, sfp[0].asObject, false/*delim*/, wb, NULL, 0);
	}
}

static void UI_ReportCaughtException(KonohaContext *kctx, kException *e, KonohaStack *bottomStack, KonohaStack *topStack)
{
	const char *exceptionName;
	const char *optionalMessage;
	int fault;
	KonohaStack *sfp;
	KBuffer wb;

	DBG_ASSERT(IS_Exception(e));
	exceptionName = KSymbol_text(e->symbol);
	optionalMessage = kString_text(e->Message);
	fault = e->fault;

	PLATAPI printf_i("%s", BeginTag(kctx, ErrTag));
	if(optionalMessage != NULL && optionalMessage[0] != 0) {
		PLATAPI printf_i("%s: SoftwareFault %s", exceptionName, optionalMessage);
	}
	else {
		PLATAPI printf_i("%s:", exceptionName);
		if(KFlag_Is(int, fault, SoftwareFault)) {
			PLATAPI printf_i(" SoftwareFault");
		}
		if(KFlag_Is(int, fault, UserFault)) {
			PLATAPI printf_i(" UserFault");
		}
		if(KFlag_Is(int, fault, SystemFault)) {
			PLATAPI printf_i(" SystemFault");
		}
		if(KFlag_Is(int, fault, ExternalFault)) {
			PLATAPI printf_i(" ExternalFault");
		}
	}
	PLATAPI printf_i("%s\n", EndTag(kctx, ErrTag));
	PLATAPI printf_i("%sStackTrace\n", BeginTag(kctx, InfoTag));

	sfp = topStack;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	while(bottomStack < sfp) {
		kMethod *mtd = sfp[K_MTDIDX].calledMethod;
		kfileline_t uline = sfp[K_RTNIDX].calledFileLine;
		const char *file = PLATAPI shortFilePath(KFileLine_textFileName(uline));
		KClass *cThis;
		unsigned i;
		kParam *param;
		PLATAPI printf_i(" [%ld] (%s:%d) %s.%s%s(", (sfp - kctx->stack->stack), file, (kushort_t)uline, kMethod_Fmt3(mtd));
		cThis = KClass_(mtd->typeId);
		if(!KClass_Is(UnboxType, cThis)) {
			cThis = kObject_class(sfp[0].asObject);
		}
		if(!kMethod_Is(Static, mtd)) {
			KBuffer_WriteValue(kctx, &wb, cThis, sfp);
			PLATAPI printf_i("this=(%s) %s, ", KClass_text(cThis), KLIB KBuffer_text(kctx, &wb, 1));
			KLIB KBuffer_Free(&wb);
		}
		param = kMethod_GetParam(mtd);
		for(i = 0; i < param->psize; i++) {
			KClass *c;
			if(i > 0) {
				PLATAPI printf_i(", ");
			}
			c = KClass_(param->paramtypeItems[i].attrTypeId);
			c = c->realtype(kctx, c, cThis);
			KBuffer_WriteValue(kctx, &wb, c, sfp + i + 1);
			PLATAPI printf_i("%s=(%s) %s", KSymbol_text(KSymbol_Unmask(param->paramtypeItems[i].name)), KClass_text(c), KLIB KBuffer_text(kctx, &wb, 1));
			KLIB KBuffer_Free(&wb);
		}
		PLATAPI printf_i(")\n");
		sfp = sfp[K_SHIFTIDX].previousStack;
	}
	KLIB KBuffer_Free(&wb);
	PLATAPI printf_i("%s\n", EndTag(kctx, InfoTag));
}

static void ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{
	va_list ap;
	va_start(ap , fmt);
	fflush(stdout);
	fprintf(stderr, "DEBUG(%s:%d) ", func, line);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static char *myreadline(const char *prompt)
{
	static int checkCTL = 0;
	int ch, pos = 0;
	static char linebuf[1024]; // THREAD-UNSAFE
	char *p;
	fputs(prompt, stdout);
	while((ch = fgetc(stdin)) != EOF) {
		if(ch == '\r') continue;
		if(ch == 27) {
			/* ^[[A */;
			fgetc(stdin); fgetc(stdin);
			if(checkCTL == 0) {
				fprintf(stdout, " - use readline, it provides better shell experience.\n");
				checkCTL = 1;
			}
			continue;
		}
		if(ch == '\n' || pos == sizeof(linebuf) - 1) {
			linebuf[pos] = 0;
			break;
		}
		linebuf[pos] = ch;
		pos++;
	}
	if(ch == EOF) return NULL;
	p = (char *)malloc(pos+1);
	memcpy(p, linebuf, pos+1);
	return p;
}

static int myadd_history(const char *line)
{
	return 0;
}

static char *InputUserText(KonohaContext *kctx, const char *message, int flag)
{
	return myreadline(message);
}

static int InputUserApproval(KonohaContext *kctx, const char *message, const char *yes, const char *no, int defval)
{
	char buff[BUFSIZ] = {0};
	const char *ykey = defval ? "Y" : "y";
	const char *nkey = defval ? "n" : "N";
	char *ret;
	if(message == NULL || message[strlen(message)] == '\0') message = "Do you approve?";
	if(yes == NULL || yes[0] == '\0') yes = "yes";
	if(no == NULL || no[0] == '\0') no = "no";
	fprintf(stdout, "%s (%s %s, %s %s): ", message, yes, ykey, no, nkey);
	ret = fgets(buff, BUFSIZ, stdin);
	if(defval) {
		return ((buff[0] == 'N' || buff[0] == 'n') && buff[1] == '\n') ? false : true;
	}
	else {
		return ((buff[0] == 'Y' || buff[0] == 'y') && buff[1] == '\n') ? false : true;
	}
	(void)ret; /* remove compiler warn :: ignoring return value */
}

static char *InputUserPassword(KonohaContext *kctx, const char *message)
{
	char buff[BUFSIZ] = {0};
	size_t len;
	char *p, *ret;
#if defined(__MINGW32__) || defined(_MSC_VER)
	HANDLE stdInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD oldMode;
	fprintf(stdout, "%s", message);
	GetConsoleMode(stdInput, &oldMode);
	SetConsoleMode(stdInput, 0);
	ret = fgets(buff, BUFSIZ, stdin);
	SetConsoleMode(stdInput, oldMode);
#else
	struct termios tm, tm_save;
	fprintf(stdout, "%s", message);
	tcgetattr(fileno(stdin), &tm);
	tm_save = tm;
//	tm.c_lflag |= ~ECHO;
	tm.c_lflag &= ~ECHO;
	tm.c_lflag |= ECHONL;
	tcsetattr(fileno(stdin), TCSANOW, &tm);
	ret = fgets(buff, BUFSIZ, stdin);
	tcsetattr(fileno(stdin), TCSANOW, &tm_save);
#endif
	len = strlen(buff) + 1;
	p = (char *) malloc(len);
	if(p != NULL) {
		memcpy(p, buff, len);
	}
	(void)ret; /* remove compiler warn :: ignoring return value */
	return p;
}

// -------------------------------------------------------------------------

kbool_t LoadConsoleModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"Console", "0.1", 0, "term",
	};
	factory->ConsoleModule.ConsoleInfo     = &ModuleInfo;
	if(factory->readline_i == NULL) {
		factory->readline_i = myreadline;
	}
	if(factory->add_history_i == NULL) {
		factory->add_history_i = myadd_history;
	}
	factory->ConsoleModule.ReportUserMessage        = UI_ReportUserMessage;
	factory->ConsoleModule.ReportCompilerMessage    = UI_ReportCompilerMessage;
	factory->ConsoleModule.ReportCaughtException    = UI_ReportCaughtException;
	factory->ConsoleModule.ReportDebugMessage       = ReportDebugMessage;

	factory->ConsoleModule.InputUserApproval        = InputUserApproval;
	factory->ConsoleModule.InputUserText            = InputUserText;
	factory->ConsoleModule.InputUserPassword        = InputUserPassword;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
