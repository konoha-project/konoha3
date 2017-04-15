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

#include "konoha3/org_GreenTeaScript_MiniKonohaExecutor.h"
#include "../command/command.c"

#define ExecuteKonoha_Through(in, out) do {\
	KBuffer wb;\
	KonohaContext* kctx = ExecuteKonoha_Init(&wb);\
	ExecuteKonoha_Eval(kctx, &wb, (in), (out));\
	ExecuteKonoha_Kill(kctx, &wb);\
} while(0)
static KonohaContext* ExecuteKonoha_Init(KBuffer* wb);
static void ExecuteKonoha_Eval(KonohaContext* kctx, KBuffer* wb, const char* input, char* output);
static int ExecuteKonoha_Kill(KonohaContext* kctx, KBuffer* wb);

JNIEXPORT jstring JNICALL Java_org_GreenTeaScript_MiniKonohaExecutor_MiniKonohaExecute (JNIEnv *env, jobject me, jstring srcj)
{
	//set input string
	const char *src = (*env)->GetStringUTFChars(env, srcj, NULL);

	//eval script
	char output[256];
	ExecuteKonoha_Through(src, output);

	//release input string
	(*env)->ReleaseStringUTFChars(env, srcj, src);

	//create return_value
	return (*env)->NewStringUTF(env, output);
}

static KonohaContext* ExecuteKonoha_Init(KBuffer* wb)
{
	struct KonohaFactory factory = {};
	if(getenv("KONOHA_DEBUG") != NULL) {
		factory.verbose_debug = 1;
		factory.verbose_sugar = 1;
		factory.verbose_code = 1;
	}
	KonohaFactory_SetDefaultFactory(&factory, PosixFactory, 0, NULL);
	KonohaContext* kctx = KonohaFactory_CreateKonoha(&factory);

	KBaseTrace(trace);
	CommandLine_SetARGV(kctx, 0, NULL, trace);
	interactive_flag = 1;
	KonohaContext_Set(Interactive, kctx);
	CommandLine_Import(kctx, "Konoha.Man", trace);

	KLIB KBuffer_Init(&(kctx->stack->cwb), wb);

	return kctx;
}
static void ExecuteKonoha_Eval(KonohaContext* kctx, KBuffer* wb, const char* input, char* output)
{
	memset(output, '\0', strlen(output)); 
	kstatus_t status = K_CONTINUE;

	KLIB KBuffer_Write(kctx, wb, input, strlen(input));
	//CheckNode(KLIB KBuffer_text(kctx, wb, NonZero), KBuffer_bytesize(wb)) > 0

	if(KBuffer_bytesize(wb) > 0) {
		//PLATAPI add_history_i(KLIB KBuffer_text(kctx, wb, EnsureZero));

		status = (kstatus_t)Konoha_Eval((KonohaContext *)kctx, KLIB KBuffer_text(kctx, wb, EnsureZero), 0);
		KLIB KBuffer_Free(wb);
		if(status != K_FAILED) {
			KRuntimeContextVar *base = kctx->stack;
			ktypeattr_t ty = base->evalty;
			if(ty != KType_void) {
				KonohaStack *lsfp = base->stack + base->evalidx;
				if(!KType_Is(UnboxType, ty)) {
					ty = kObject_typeId(lsfp[0].asObject);
				}   
				KClass_(ty)->format(kctx, lsfp, 0, wb);
				//PLATAPI printf_i("  (%s) %s\n", KType_text(ty), KLIB KBuffer_text(kctx, wb, EnsureZero));
				strcpy(output, KLIB KBuffer_text(kctx, wb, EnsureZero));
				base->evalty = KType_void;
			}   
			KLIB KBuffer_Free(wb);
		}
	}
	return;
}
static int ExecuteKonoha_Kill(KonohaContext* kctx, KBuffer* wb)
{
	KLIB KBuffer_Free(wb);
	//PLATAPI printf_i("\n");

	return Konoha_Destroy(kctx);
}

#ifdef __cplusplus
}
#endif
