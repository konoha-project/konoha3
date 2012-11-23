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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/ttydefaults.h>
#include <minikonoha/minikonoha.h>
#include <minikonoha/klib.h>

// -------------------------------------------------------------------------
/* Console */

static const char* BeginTag(KonohaContext *kctx, kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	if(!KonohaContext_Is(Interactive, kctx)) t = NoneTag;
	static const char* tags[] = {
		"\x1b[1m\x1b[31m", /*CritTag*/
		"\x1b[1m\x1b[31m", /*ErrTag*/
		"\x1b[1m\x1b[31m", /*WarnTag*/
		"\x1b[1m", /*NoticeTag*/
		"\x1b[1m", /*InfoTag*/
		"", /*DebugTag*/
		"", /* NoneTag*/
	};
	return tags[(int)t];
}

static const char* EndTag(KonohaContext *kctx, kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	if(!KonohaContext_Is(Interactive, kctx)) t = NoneTag;
	static const char* tags[] = {
			"\x1b[0m", /*CritTag*/
			"\x1b[0m", /*ErrTag*/
			"\x1b[0m", /*WarnTag*/
			"\x1b[0m", /*NoticeTag*/
			"\x1b[0m", /*InfoTag*/
			"", /* Debug */
			"", /* NoneTag*/
	};
	return tags[(int)t];
}

static void UI_ReportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{
	const char *beginTag = BeginTag(kctx, level);
	const char *endTag = EndTag(kctx, level);
	const char *kLF = isNewLine ? "\n" : "";
	if(pline > 0) {
		const char *file = FileId_t(pline);
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

static void Kwb_WriteValue(KonohaContext *kctx, KGrowingBuffer *wb, KonohaClass *c, KonohaStack *sfp)
{
	if(CT_isUnbox(c)) {
		c->p(kctx, sfp, 0, wb);
	}
	else {
		KLIB kObject_WriteToBuffer(kctx, sfp[0].asObject, false/*delim*/, wb, NULL, 0);
	}
}

static void UI_ReportCaughtException(KonohaContext *kctx, const char *exceptionName, int fault, const char *optionalMessage, KonohaStack *bottomStack, KonohaStack *topStack)
{
	PLATAPI printf_i("%s", BeginTag(kctx, ErrTag));
	if(optionalMessage != NULL && optionalMessage[0] != 0) {
		PLATAPI printf_i("%s: SoftwareFault %s", exceptionName, optionalMessage);
	}
	else {
		PLATAPI printf_i("%s:", exceptionName);
		if(TFLAG_is(int, fault, SoftwareFault)) {
			PLATAPI printf_i(" SoftwareFault");
		}
		if(TFLAG_is(int, fault, UserFault)) {
			PLATAPI printf_i(" UserFault");
		}
		if(TFLAG_is(int, fault, SystemFault)) {
			PLATAPI printf_i(" SystemFault");
		}
		if(TFLAG_is(int, fault, ExternalFault)) {
			PLATAPI printf_i(" ExternalFault");
		}
	}
	PLATAPI printf_i("%s\n", EndTag(kctx, ErrTag));
	PLATAPI printf_i("%sStackTrace\n", BeginTag(kctx, InfoTag));

	KonohaStack *sfp = topStack;
	KGrowingBuffer wb;
	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
	while(bottomStack < sfp) {
		kMethod *mtd = sfp[K_MTDIDX].calledMethod;
		kfileline_t uline = sfp[K_RTNIDX].calledFileLine;
		const char *file = PLATAPI shortFilePath(FileId_t(uline));
		PLATAPI printf_i(" [%ld] (%s:%d) %s.%s%s(", (sfp - kctx->stack->stack), file, (kushort_t)uline, Method_t(mtd));
		KonohaClass *cThis = CT_(mtd->typeId);
		if(!CT_isUnbox(cThis)) {
			cThis = O_ct(sfp[0].asObject);
		}
		if(!kMethod_Is(Static, mtd)) {
			Kwb_WriteValue(kctx, &wb, cThis, sfp);
			PLATAPI printf_i("this=(%s) %s, ", CT_t(cThis), KLIB Kwb_top(kctx, &wb, 1));
			KLIB Kwb_Free(&wb);
		}
		unsigned i;
		kParam *param = kMethod_GetParam(mtd);
		for(i = 0; i < param->psize; i++) {
			if(i > 0) {
				PLATAPI printf_i(", ");
			}
			KonohaClass *c = CT_(param->paramtypeItems[i].ty);
			c = c->realtype(kctx, c, cThis);
			Kwb_WriteValue(kctx, &wb, c, sfp + i + 1);
			PLATAPI printf_i("%s=(%s) %s", SYM_t(Symbol_Unmask(param->paramtypeItems[i].fn)), CT_t(c), KLIB Kwb_top(kctx, &wb, 1));
			KLIB Kwb_Free(&wb);
		}
		PLATAPI printf_i(")\n");
		sfp = sfp[K_SHIFTIDX].previousStack;
	}
	KLIB Kwb_Free(&wb);
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

static char* myreadline(const char* prompt)
{
	static int checkCTL = 0;
	int ch, pos = 0;
	static char linebuf[1024]; // THREAD-UNSAFE
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
	char *p = (char *)malloc(pos+1);
	memcpy(p, linebuf, pos+1);
	return p;
}

static int myadd_history(const char* line)
{
	return 0;
}

static char* InputUserText(KonohaContext *kctx, const char *message, int flag)
{
	return myreadline(message);
}

static int InputUserApproval(KonohaContext *kctx, const char *message, const char *yes, const char *no, int defval)
{
	char buff[BUFSIZ] = {0};
	const char *ykey = defval ? "Y" : "y";
	const char *nkey = defval ? "n" : "N";
	if(message == NULL || message[strlen(message)] == '\0') message = "Do you approve?";
	if(yes == NULL || yes[0] == '\0') yes = "yes";
	if(no == NULL || no[0] == '\0') no = "no";
	fprintf(stdout, "%s (%s %s, %s %s): ", message, yes, ykey, no, nkey);
	fgets(buff, BUFSIZ, stdin);
	if(defval) {
		return ((buff[0] == 'N' || buff[0] == 'n') && buff[1] == '\n') ? false : true;
	}
	else {
		return ((buff[0] == 'Y' || buff[0] == 'y') && buff[1] == '\n') ? false : true;
	}
}

static char* InputUserPassword(KonohaContext *kctx, const char *message)
{
	char buff[BUFSIZ] = {0};
	struct termios tm, tm_save;
	fprintf(stdout, "%s", message);
	tcgetattr(fileno(stdin), &tm);
	tm_save = tm;
//	tm.c_lflag |= ~ECHO;
	tm.c_lflag &= ~ECHO;
	tm.c_lflag |= ECHONL;
	tcsetattr(fileno(stdin), TCSANOW, &tm);
	fgets(buff, BUFSIZ, stdin);
	tcsetattr(fileno(stdin), TCSANOW, &tm_save);
	size_t len = strlen(buff) + 1;
	char *p = malloc(len);
	if(p != NULL) {
		memcpy(p, buff, len);
	}
	return p;
}

// -------------------------------------------------------------------------

kbool_t LoadConsoleModule(KonohaFactory *factory, ModuleType type)
{
//	void *handler = dlopen("libreadline" K_OSDLLEXT, RTLD_LAZY);
//	if(handler != NULL) {
//		factory->readline_i = (char* (*)(const char *))dlsym(handler, "readline");
//		factory->add_history_i = (int (*)(const char *))dlsym(handler, "add_history");
//	}
	static KModuleInfo ModuleInfo = {
		"Console", "0.1", 0, "term",
	};
	factory->ConsoleInfo     = &ModuleInfo;
	if(factory->readline_i == NULL) {
		factory->readline_i = myreadline;
	}
	if(factory->add_history_i == NULL) {
		factory->add_history_i = myadd_history;
	}
	factory->ReportUserMessage        = UI_ReportUserMessage;
	factory->ReportCompilerMessage    = UI_ReportCompilerMessage;
	factory->ReportCaughtException    = UI_ReportCaughtException;
	factory->ReportDebugMessage       = ReportDebugMessage;

	factory->InputUserApproval        = InputUserApproval;
	factory->InputUserText            = InputUserText;
	factory->InputUserPassword        = InputUserPassword;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

