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

static int checkstmt(const char *t, size_t len)
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

static kstatus_t readstmt(KonohaContext *kctx, KUtilsWriteBuffer *wb, kfileline_t *uline)
{
	int line = 1;
	kstatus_t status = K_CONTINUE;
//	fputs(TERM_BBOLD(kctx), stdout);
	while(1) {
		int check;
		char *ln = PLATAPI readline_i(line == 1 ? ">>> " : "    ");
		if(ln == NULL) {
			KLIB Kwb_free(wb);
			status = K_BREAK;
			break;
		}
		if(line > 1) {
			KLIB Kwb_write(kctx, wb, "\n", 1);
		}
		KLIB Kwb_write(kctx, wb, ln, strlen(ln));
		free(ln);
		if((check = checkstmt(KLIB Kwb_top(kctx, wb, 0), Kwb_bytesize(wb))) > 0) {
			uline[0]++;
			line++;
			continue;
		}
		if(check < 0) {
			PLATAPI printf_i("(Cancelled)...\n");
			KLIB Kwb_free(wb);
		}
		break;
	}
	if(Kwb_bytesize(wb) > 0) {
		PLATAPI add_history_i(KLIB Kwb_top(kctx, wb, 1));
	}
//	fputs(TERM_EBOLD(kctx), stdout);
	fflush(stdout);
	uline[0]++;
	return status;
}

static void dumpEval(KonohaContext *kctx, KUtilsWriteBuffer *wb)
{
	KonohaStackRuntimeVar *base = kctx->stack;
	ktype_t ty = base->evalty;
	if(ty != TY_void) {
		KonohaStack *lsfp = base->stack + base->evalidx;
		CT_(ty)->p(kctx, lsfp, 0, wb);
		fflush(stdout);
		PLATAPI printf_i("TYPE=%s EVAL=%s\n", TY_t(ty), KLIB Kwb_top(kctx, wb,1));
	}
}

static void shell(KonohaContext *kctx)
{
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	kfileline_t uline = FILEID_("(shell)") | 1;
	while(1) {
		kfileline_t inc = 0;
		kstatus_t status = readstmt(kctx, &wb, &inc);
		if(status == K_BREAK) break;
		if(status == K_CONTINUE && Kwb_bytesize(&wb) > 0) {
			status = konoha_eval((KonohaContext*)kctx, KLIB Kwb_top(kctx, &wb, 1), uline);
			uline += inc;
			KLIB Kwb_free(&wb);
			if(status != K_FAILED) {
				dumpEval(kctx, &wb);
				KLIB Kwb_free(&wb);
			}
		}
	}
	KLIB Kwb_free(&wb);
	PLATAPI printf_i("\n");
	return;
}

static void show_version(KonohaContext *kctx)
{
	int i;
	PLATAPI printf_i(K_PROGNAME " " K_VERSION " (%s) (%x, %s)\n", K_CODENAME, K_REVISION, __DATE__);
	PLATAPI printf_i("[gcc %s]\n", __VERSION__);
	PLATAPI printf_i("options:");
	for(i = 0; i < KonohaModule_MAXSIZE; i++) {
		if(kctx->modshare[i] != NULL) {
			PLATAPI printf_i(" %s", kctx->modshare[i]->name);
		}
	}
	PLATAPI printf_i("\n");
}

static kbool_t konoha_shell(KonohaContext* konoha)
{
	show_version(konoha);
	shell(konoha);
	return true;
}


#endif /* MINISHELL_H_ */
