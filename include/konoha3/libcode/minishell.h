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

#ifndef MINISHELL_H_
#define MINISHELL_H_

static int CheckNode(const char *t, size_t len)
{
	size_t i = 0;
	int ch, quote = 0, nest = 0;
	L_NORMAL:
	for(; i < len; i++) {
		ch = t[i];
		if(ch == '{' || ch == '[' || ch == '(') nest++;
		if(ch == '}' || ch == ']' || ch == ')') nest--;
		if(ch == '\'' || ch == '"' || ch == '`') {
			if(t[i+1] == ch && t[i+2] == ch) {
				quote = ch; i+=2;
				goto L_TQUOTE;
			}
		}
	}
	return nest;
	L_TQUOTE:
	DBG_ASSERT(i > 0);
	for(; i < len; i++) {
		ch = t[i];
		if(t[i-1] != '\\' && ch == quote) {
			if(t[i+1] == ch && t[i+2] == ch) {
				i+=2;
				goto L_NORMAL;
			}
		}
	}
	return 1;
}

static kstatus_t ReadNode(KonohaContext *kctx, KBuffer *wb, kfileline_t *uline)
{
	int line = 1;
	kstatus_t status = K_CONTINUE;
//	fputs(TERM_BBOLD(kctx), stdout);
	while(1) {
		int check;
		char *ln = PLATAPI readline_i(line == 1 ? ">>> " : "    ");
		if(ln == NULL) {
			KLIB KBuffer_Free(wb);
			status = K_BREAK;
			break;
		}
		if(line > 1) {
			KLIB KBuffer_Write(kctx, wb, "\n", 1);
		}
		KLIB KBuffer_Write(kctx, wb, ln, strlen(ln));
		free(ln);
		if((check = CheckNode(KLIB KBuffer_text(kctx, wb, NonZero), KBuffer_bytesize(wb))) > 0) {
			uline[0]++;
			line++;
			continue;
		}
		if(check < 0) {
			PLATAPI printf_i("(Cancelled)...\n");
			KLIB KBuffer_Free(wb);
		}
		break;
	}
	if(KBuffer_bytesize(wb) > 0) {
		PLATAPI add_history_i(KLIB KBuffer_text(kctx, wb, EnsureZero));
	}
//	fputs(TERM_EBOLD(kctx), stdout);
	fflush(stdout);
	uline[0]++;
	return status;
}

static void DumpEval(KonohaContext *kctx, KBuffer *wb)
{
	KRuntimeContextVar *base = kctx->stack;
	ktypeattr_t ty = base->evalty;
	if(ty != KType_void) {
		KonohaStack *lsfp = base->stack + base->evalidx;
		if(!KType_Is(UnboxType, ty)) {
			ty = kObject_typeId(lsfp[0].asObject);
		}
		KClass_(ty)->format(kctx, lsfp, 0, wb);
		fflush(stdout);
		PLATAPI printf_i("  (%s) %s\n", KType_text(ty), KLIB KBuffer_text(kctx, wb, EnsureZero));
		base->evalty = KType_void;
	}
}

static void RunShell(KonohaContext *kctx)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	kfileline_t uline = FILEID_("(shell)") | 1;
	while(1) {
		kfileline_t inc = 0;
		kstatus_t status = ReadNode(kctx, &wb, &inc);
		if(status == K_BREAK) break;
		if(status == K_CONTINUE && KBuffer_bytesize(&wb) > 0) {
			status = (kstatus_t)Konoha_Eval((KonohaContext *)kctx, KLIB KBuffer_text(kctx, &wb, EnsureZero), uline);
			uline += inc;
			KLIB KBuffer_Free(&wb);
			if(status != K_FAILED) {
				DumpEval(kctx, &wb);
				KLIB KBuffer_Free(&wb);
			}
		}
	}
	KLIB KBuffer_Free(&wb);
	PLATAPI printf_i("\n");
	return;
}

static void ShowModule(KonohaContext *kctx, const KModuleInfo *info)
{
	if(info != NULL && info->desc != NULL) {
		PLATAPI printf_i(" %s", info->desc);
	}
}

static void ShowVersion(KonohaContext *kctx)
{
	PLATAPI printf_i(K_PROGNAME " " K_VERSION " (%s) (%s, %lu, %s)\n", K_CODENAME, K_REVISION, K_DATE, __DATE__);
#if defined(__GNUC__)
	PLATAPI printf_i("[gcc %s]\n", __VERSION__);
#elif defined(_MSC_VER)
	PLATAPI printf_i("[Microsoft Visual C++ %d]\n", _MSC_VER);
#elif defined(__clang__)
	PLATAPI printf_i("[clang %s]\n", __VERSION__);
#endif
	PLATAPI printf_i("modules:");
	ShowModule(kctx, PLATAPI ExecutionEngineModule.ExecutionEngineInfo);
	ShowModule(kctx, PLATAPI GCModule.GCInfo);
	ShowModule(kctx, PLATAPI ConsoleModule.ConsoleInfo);
	ShowModule(kctx, PLATAPI EventModule.EventInfo);
	ShowModule(kctx, PLATAPI I18NModule.I18NInfo);
	PLATAPI printf_i("\n");
}

static kbool_t konoha_shell(KonohaContext* konoha)
{
	ShowVersion(konoha);
	RunShell(konoha);
	return true;
}

#endif /* MINISHELL_H_ */
