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

#ifndef FILE_GLUE_H_
#define FILE_GLUE_H_

#include<stdio.h>

typedef const struct _kFILE kFILE;
struct _kFILE {
	KonohaObjectHeader h;
	FILE *fp;
	const char *realpath;
};

/* ------------------------------------------------------------------------ */

static void File_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kFILE *file = (struct _kFILE*)o;
	file->fp = (conf != NULL) ? conf : NULL;
	file->realpath = NULL;
}

static void File_free(KonohaContext *kctx, kObject *o)
{
	struct _kFILE *file = (struct _kFILE*)o;
	if (file->fp != NULL) {
		int ret = fclose(file->fp);
		if (ret != 0) {
			ktrace(_SystemFault,
					KEYVALUE_s("@", "fclose"),
					KEYVALUE_s("errstr", strerror(errno))
			);
		}
		file->fp = NULL;
	}
	if(file->realpath != NULL) {
		free((void*)file->realpath); // free path
		file->realpath = NULL;
	}
}

static void File_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
	kFILE *file = (kFILE*)sfp[pos].asObject;
	FILE *fp = file->fp;
	KLIB Kwb_printf(kctx, wb, "FILE :%p, path=%s", fp, file->realpath);
}

/* ------------------------------------------------------------------------ */
//## @Native @Throwable FILE System.fopen(String path, String mode);
static KMETHOD System_fopen(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *mode = IS_NULL(sfp[2].asString) ? "r" : S_text(sfp[2].asString);
	FILE *fp = fopen(S_text(s), mode);
	DBG_P("fp=%p, filepath=%s", fp, S_text(s));
	if (fp == NULL) {
		ktrace(_SystemFault|_ScriptFault,
				KEYVALUE_s("@", "fopen"),
				KEYVALUE_s("path", S_text(s)),
				KEYVALUE_u("mode", mode),
				KEYVALUE_s("errstr", strerror(errno))
		);
	}
	struct _kFILE *file = (struct _kFILE*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), (uintptr_t)fp);
	file->realpath = realpath(S_text(s), NULL);
	RETURN_(file);
}

//## @Native int File.read(Bytes buf, int offset, int len);
static KMETHOD File_read(KonohaContext *kctx, KonohaStack *sfp)
{
	kFILE *file = (kFILE*)sfp[0].asObject;
	FILE *fp = file->fp;
	size_t size = 0;
	if(fp != NULL) {
		kBytes *ba = sfp[1].ba;
		size_t offset = (size_t)sfp[2].intValue;
		size_t len = (size_t)sfp[3].intValue;
		size = ba->bytesize;
		if(!(offset < size)) {
			kfileline_t uline = sfp[K_RTNIDX].uline;
			kreportf(CritTag, uline, "OutOfRange!!, offset=%d, size=%d", offset, size);
		}
		if(len == 0) len = size - offset;
		size = fread(ba->buf + offset, 1, len, fp);
		DBG_P("SHINPEI: size=%d", size);
		if (size == 0 && ferror(fp) != 0){
			ktrace(_SystemFault,
					KEYVALUE_s("@", "fread"),
					KEYVALUE_s("errstr", strerror(errno))
			);
			clearerr(fp);
		}
	}
	RETURNi_(size);
}

//## @Native int File.write(Bytes buf, int offset, int len);
static KMETHOD File_write(KonohaContext *kctx , KonohaStack *sfp)
{
	kFILE *file = (kFILE*)sfp[0].asObject;
	FILE *fp = file->fp;
	size_t size = 0;
	if(fp != NULL) {
		kBytes *ba = sfp[1].ba;
		size_t offset = (size_t)sfp[2].intValue;
		size_t len = (size_t)sfp[3].intValue;
		size = ba->bytesize;
		if(len == 0) len = size - offset;
		size = fwrite(ba->buf + offset, 1, len, fp);
		if (size < len) {
			// error
			ktrace(_SystemFault,
					KEYVALUE_s("@", "fwrite"),
					KEYVALUE_s("errstr", strerror(errno))
			);
		}
	}
	RETURNi_(size);
}

//## @Native void File.close();
static KMETHOD File_close(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kFILE *file = (struct _kFILE*)sfp[0].asObject;
	FILE *fp = file->fp;
	if(fp != NULL) {
		int ret = fclose(fp);
		if (ret != 0) {
			ktrace(_SystemFault,
					KEYVALUE_s("@", "fclose"),
					KEYVALUE_s("errstr", strerror(errno))
			);
		}
		file->fp = NULL;
	}
	RETURNvoid_();
}

//## @Native int File.getC();
static KMETHOD File_getC(KonohaContext *kctx, KonohaStack *sfp)
{
	FILE *fp = ((kFILE*)sfp[0].asObject)->fp;
	int ch = EOF;
	if (fp != NULL) {
		ch = fgetc(fp);
		if (ch == EOF && ferror(fp) != 0) {
			ktrace(LOGPOL_DEBUG | _DataFault,
					KEYVALUE_s("@", "fgetc"),
					KEYVALUE_s("errstr", strerror(errno))
			);
		}
	}
	RETURNi_(ch);
}

//## @Native boolean File.putC(int ch);
static KMETHOD File_putC(KonohaContext *kctx, KonohaStack *sfp)
{
	FILE *fp = ((kFILE*)sfp[0].asObject)->fp;
	if (fp != NULL) {
		int ch = fputc(sfp[1].intValue, fp);
		if (ch == EOF) {
			ktrace(LOGPOL_DEBUG | _DataFault,
					KEYVALUE_s("@", "fputc"),
					KEYVALUE_s("errstr", strerror(errno))
			);
		}
		RETURNb_(ch != EOF);
	}
	RETURNb_(0);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define CT_File         cFile
#define TY_File         cFile->typeId
#define IS_File(O)      ((O)->h.ct == CT_File)

static kbool_t file_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_CLASS defFile = {
		STRUCTNAME(FILE),
		.cflag = kClass_Final,
		.init  = File_init,
		.free  = File_free,
		.p     = File_p,
	};

	KonohaClass *cFile = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defFile, pline);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(System_fopen), TY_File, TY_System, MN_("fopen"), 2, TY_String, FN_("path"), TY_String, FN_("mode"),
		_Public|_Const|_Im, _F(File_close), TY_void, TY_File, MN_("close"), 0,
		_Public|_Const|_Im, _F(File_getC), TY_Int, TY_File, MN_("getC"), 0,
		_Public|_Const|_Im, _F(File_putC), TY_Boolean, TY_File, MN_("putC"), 1, TY_Int, FN_("ch"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	if (IS_defineBytes()) {
		// the function below uses Bytes
		KDEFINE_METHOD MethodData2[] = {
			_Public|_Const, _F(File_write), TY_Int, TY_File, MN_("write"), 3, TY_Bytes, FN_("buf"), TY_Int, FN_("offset"), TY_Int, FN_("len"),
			_Public|_Const, _F(File_read), TY_Int, TY_File, MN_("read"), 3, TY_Bytes, FN_("buf"), TY_Int, FN_("offset"), TY_Int, FN_("len"),
			DEND,
		};
		KLIB kNameSpace_loadMethodData(kctx, ns, MethodData2);
	} else {
		kreportf(InfoTag, pline, "konoha.bytes package hasn't imported. Some features are still disabled.");
	}
	return true;
}

static kbool_t file_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t file_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t file_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

#endif /* FILE_GLUE_H_ */
