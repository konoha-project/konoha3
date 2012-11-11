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

#define USE_FILE 1
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/konoha_common.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifndef K_PATHMAX
#define K_PATHMAX 1024
#endif

#define LogFileName(S)     LogText("filename", S)
#define LogFile(F) LogText("filename", kFile_textPath(kctx, F))
#define LogWrittenByte(size)  LogUint("WrittenByteSize", size)

static const char* kFile_textPath(KonohaContext *kctx, kFile *file)
{
	return (file->PathInfoNULL == NULL) ? "unknown" : S_text(file->PathInfoNULL);
}

static int TRACE_fgetc(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
{
	int ch = fgetc(file->fp);
	if(ferror(file->fp) != 0) {
		KTraceErrorPoint(trace, SystemFault, "fgetc", LogFile(file), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, trace->baseStack);
	}
	return ch;
}

static int TRACE_fputc(KonohaContext *kctx, kFile *file, int ch, KTraceInfo *trace)
{
	if(fputc(ch, file->fp) == EOF) {
		KTraceErrorPoint(trace, SystemFault, "fputc", LogFile(file), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, trace->baseStack);
	}
	else if(!kFile_is(ChangeLessStream, file)) {
		KTraceChangeSystemPoint(trace, "fputc", LogFile(file), LogWrittenByte(1));
	}
	return ch;
}

static void kFile_close(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
{
	DBG_ASSERT(file->fp != NULL);
	int ret = fclose(file->fp);
	if(ret != 0) {
		KTraceErrorPoint(trace, SoftwareFault|SystemFault, "fclose", LogErrno);
	}
	//file->fp = NULL;
	if(file->readerIconv != ICONV_NULL) {
		PLATAPI iconv_close_i(kctx, file->readerIconv);
		file->readerIconv = ICONV_NULL;
	}
	if(file->writerIconv != ICONV_NULL) {
		PLATAPI iconv_close_i(kctx, file->writerIconv);
		file->writerIconv = ICONV_NULL;
	}
}

static void kFile_checkEOF(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
{
	DBG_ASSERT(file->fp != NULL);
	if(feof(file->fp) != 0) {
		kFile_close(kctx, file, trace);
		file->fp = NULL;  /* input stream is automatically closed */
	}
}

static size_t TRACE_fread(KonohaContext *kctx, kFile *file, char *buf, size_t bufsiz, KTraceInfo *trace)
{
	size_t size = fread(buf, 1, bufsiz, file->fp);
	if(ferror(file->fp) != 0){
		KTraceErrorPoint(trace, SystemFault, "fread", LogFile(file), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, trace->baseStack);
	}
	kFile_checkEOF(kctx, file, trace);
	return size;
}

static size_t TRACE_fwrite(KonohaContext *kctx, kFile *file, const char *buf, size_t bufsiz, KTraceInfo *trace)
{
	size_t size = fwrite(buf, 1, bufsiz, file->fp);
	if(ferror(file->fp) != 0){
		KTraceErrorPoint(trace, SystemFault, "fwrite", LogFile(file), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, trace->baseStack);
	}
	if(size > 0 && !kFile_is(ChangeLessStream, file)) {
		KTraceChangeSystemPoint(trace, "fwrite", LogFile(file), LogWrittenByte(size));
	}
	return size;
}

static void TRACE_fflush(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
{
	fflush(file->fp);
	if(ferror(file->fp) != 0){
		KTraceErrorPoint(trace, SystemFault, "fflush", LogFile(file), LogErrno);
	}
}

/* ------------------------------------------------------------------------ */

static void kFile_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kFileVar *file = (struct kFileVar *)o;
	file->fp = (conf != NULL) ? conf : NULL;
	file->PathInfoNULL = NULL;
	file->readerIconv = ICONV_NULL;
	file->writerIconv = ICONV_NULL;
}

static void kFile_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kFile *file = (kFile *)o;
	KREFTRACEn(file->PathInfoNULL);
}

static void kFile_free(KonohaContext *kctx, kObject *o)
{
	struct kFileVar *file = (struct kFileVar *)o;
	if(file->fp != NULL) {
		kFile_close(kctx, file, NULL/*trace*/);
	}
}

static void kFile_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	kFile *file = (kFile *)v[pos].asObject;
	if(file->PathInfoNULL != NULL) {
		KLIB Kwb_write(kctx, wb, S_text(file->PathInfoNULL), S_size(file->PathInfoNULL));
	}
	else {
		KLIB Kwb_printf(kctx, wb, "FILE:%p", file->fp);
	}
}

/* ------------------------------------------------------------------------ */

//## FILE FILE.new(String path, String mode);
static KMETHOD File_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	const char *mode = S_text(sfp[2].asString);
	FILE *fp = fopen(systemPath, mode);
	if(fp == NULL) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "fopen",
			LogText("filename", S_text(path)), LogText("mode", mode), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), fault, NULL, sfp);
	}
	if(mode[0] == 'w' || mode[0] == 'a' || mode[1] == '+') {
		KTraceChangeSystemPoint(trace, "fopen", LogFileName(S_text(path)), LogText("mode", mode));
	}
	{
		INIT_GCSTACK();
		struct kFileVar *file = (struct kFileVar *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), (uintptr_t)fp);
		KFieldInit(file, file->PathInfoNULL, path);
		if(!PLATAPI isSystemCharsetUTF8(kctx)) {
			if(mode[0] == 'w' || mode[0] == 'a' || mode[1] == '+') {
				file->writerIconv = PLATAPI iconvUTF8ToSystemCharset(kctx, trace);
			}
			else {
				file->readerIconv = PLATAPI iconvSystemCharsetToUTF8(kctx, trace);
			}
		}
		KReturnWith(file, RESET_GCSTACK());
	}
}

//## @Native void File.close();
static KMETHOD File_close(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *file = (kFile *)sfp[0].asObject;
	if(file->fp != NULL) {
		KMakeTrace(trace, sfp);
		kFile_close(kctx, file, trace);
	}
	KReturnVoid();
}

//## @Native int File.getc();
static KMETHOD File_getc(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *file = (kFile *)sfp[0].asObject;
	KMakeTrace(trace, sfp);
	int ch = file->fp == NULL ? EOF : TRACE_fgetc(kctx, file, trace);
	KReturnUnboxValue(ch);
}

//## @Native void File.putc(int ch);
static KMETHOD File_putc(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *file = (kFile *)sfp[0].asObject;
	KMakeTrace(trace, sfp);
	TRACE_fputc(kctx, file, sfp[1].intValue, trace);
	KReturnVoid();
}

//## String File.readLine();
static KMETHOD File_readLine(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *file = (kFile *)sfp[0].asObject;
	if(file->fp != NULL) {
		KGrowingBuffer wb;
		KLIB Kwb_init(&(kctx->stack->cwb), &wb);
		int ch, pos = 0, hasUTF8 = false, bufferCount = 0, policy = StringPolicy_ASCII;
		char buffer[K_PAGESIZE];
		KMakeTrace(trace, sfp);
		while((ch = TRACE_fgetc(kctx, file, trace)) != EOF) {
			//DBG_P("ch='%c', pos=%d", ch, pos);
			if(ch == '\r') continue;
			if(ch == '\n') {
				if(bufferCount == 0 && (!hasUTF8 || file->readerIconv == ICONV_NULL)) {
					KReturn(KLIB new_kString(kctx, OnStack, buffer, pos, policy));
				}
				break;
			}
			if(ch > 127) {
				hasUTF8 = true;
				policy = StringPolicy_UTF8;
			}
			buffer[pos] = ch; pos++;
			if(!(pos < K_PAGESIZE)) {
				if(hasUTF8 && file->readerIconv != ICONV_NULL) {
					KLIB Kwb_iconv(kctx, &wb, file->readerIconv, buffer, pos, trace);
				}
				else {
					KLIB Kwb_write(kctx, &wb, buffer, pos);
				}
				bufferCount++;
				hasUTF8 = false;
				pos = 0;
			}
		}
		if(pos > 0) {
			if(hasUTF8 && file->readerIconv != ICONV_NULL) {
				KLIB Kwb_iconv(kctx, &wb, file->readerIconv, buffer, pos, trace);
			}
			else {
				KLIB Kwb_write(kctx, &wb, buffer, pos);
			}
		}
		kFile_checkEOF(kctx, file, trace);
		KReturnWith(
			KLIB new_kString(kctx, OnStack, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), policy),
			KLIB Kwb_free(&wb)
		);
	}
	else {
		KReturn(KNULL(String));
	}
}

//## void File.flush();
static KMETHOD File_flush(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile   *file = sfp[0].asFile;
	KMakeTrace(trace, sfp);
	TRACE_fflush(kctx, file, trace);
}

//## void File.print(String line);
static KMETHOD File_print(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile   *file = sfp[0].asFile;
	kString *line = sfp[1].asString;
	KMakeTrace(trace, sfp);
	if(file->writerIconv == ICONV_NULL || kString_is(ASCII, line)) {
		TRACE_fwrite(kctx, file, S_text(line), S_size(line), trace);
	}
	else {
		KGrowingBuffer wb;
		KLIB Kwb_init(&(kctx->stack->cwb), &wb);
		KLIB Kwb_iconv(kctx, &wb, file->writerIconv, S_text(line), S_size(line), trace);
		TRACE_fwrite(kctx, file, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), trace);
		KLIB Kwb_free(&wb);
	}
}

//## void File.println();
static KMETHOD File_println0(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile   *file = sfp[0].asFile;
	KMakeTrace(trace, sfp);
	TRACE_fwrite(kctx, file, TEXTSIZE("\n"), trace);
}

//## void File.println(String line);
static KMETHOD File_println(KonohaContext *kctx, KonohaStack *sfp)
{
	File_print(kctx, sfp);
	File_println0(kctx, sfp);
}

//## int File.read(Bytes buf);
static KMETHOD File_read(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *file = sfp[0].asFile;
	kBytes *ba  = sfp[1].asBytes;
	KMakeTrace(trace, sfp);
	size_t readbyte = TRACE_fread(kctx, file, ba->buf, ba->bytesize, trace);
	KReturnUnboxValue(readbyte);
}

//## int File.read(Bytes buf, int offset, int len);
static KMETHOD File_read3(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *file = sfp[0].asFile;
	kBytes *ba  = sfp[1].asBytes;
	size_t offset = (size_t) sfp[2].intValue;
	KCheckIndex(offset, ba->bytesize);
	size_t len    = (size_t) sfp[3].intValue;
	if(len == 0) {
		len = ba->bytesize - offset;
	}
	else {
		KCheckIndex(offset + len - 1, ba->bytesize);
	}
	KMakeTrace(trace, sfp);
	size_t readbyte = TRACE_fread(kctx, file, ba->buf + offset, len, trace);
	KReturnUnboxValue(readbyte);
}

//## @Native int File.write(Bytes buf);
static KMETHOD File_write(KonohaContext *kctx , KonohaStack *sfp)
{
	kFile *file = sfp[0].asFile;
	kBytes *ba  = sfp[1].asBytes;
	KMakeTrace(trace, sfp);
	size_t writtenbyte = TRACE_fwrite(kctx, file, ba->buf, ba->bytesize, trace);
	KReturnUnboxValue(writtenbyte);
}

//## @Native int File.write(Bytes buf, int offset, int len);
static KMETHOD File_write3(KonohaContext *kctx , KonohaStack *sfp)
{
	kFile *file = sfp[0].asFile;
	kBytes *ba  = sfp[1].asBytes;
	size_t offset = (size_t) sfp[2].intValue;
	KCheckIndex(offset, ba->bytesize);
	size_t len    = (size_t) sfp[3].intValue;
	if(len == 0) {
		len = ba->bytesize - offset;
	}
	else {
		KCheckIndex(offset + len - 1, ba->bytesize);
	}
	KMakeTrace(trace, sfp);
	size_t writtenbyte = TRACE_fwrite(kctx, file, ba->buf + offset, len, trace);
	KReturnUnboxValue(writtenbyte);
}

//## boolean FILE.isatty();
static KMETHOD FILE_isatty(KonohaContext *kctx , KonohaStack *sfp)
{
	kFile *file = sfp[0].asFile;
	int fd = fileno(file->fp);
	KReturnUnboxValue(isatty(fd) == 1);
}

//## int FILE.getfileno();
static KMETHOD FILE_getfileno(KonohaContext *kctx , KonohaStack *sfp)
{
	kFile *file = sfp[0].asFile;
	int fd = fileno(file->fp);
	// If 'fp' is an invalid stream, fileno() returns -1.
	// However it is checked when FILE.new() is called.
	// Therefore an error should not occur here.
	DBG_ASSERT(fd != -1);
	KReturnUnboxValue(fd);
}

/* ------------------------------------------------------------------------ */

//## void setReaderCharset(String charset);
static KMETHOD File_setReaderCharset(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile   *file = sfp[0].asFile;
	KMakeTrace(trace, sfp);
	file->readerIconv = PLATAPI iconv_open_i(kctx, "UTF-8", S_text(sfp[1].asString), trace);
}

//## void setWriterCharset(String charset);
static KMETHOD File_setWriterCharset(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile   *file = sfp[0].asFile;
	KMakeTrace(trace, sfp);
	file->writerIconv = PLATAPI iconv_open_i(kctx, S_text(sfp[1].asString), "UTF-8", trace);
}

/* ------------------------------------------------------------------------ */

//## @Const String File.scriptPath(String path);
static KMETHOD File_scriptPath(KonohaContext *kctx, KonohaStack *sfp)
{
	char scriptPathBuf[K_PATHMAX];
	const char *scriptPath = PLATAPI formatTransparentPath(scriptPathBuf, sizeof(scriptPathBuf), FileId_t(sfp[K_RTNIDX].calledFileLine), S_text(sfp[1].asString));
	kStringVar *resultValue = (kStringVar *)KLIB new_kString(kctx, OnStack, scriptPath, strlen(scriptPath), 0);
	if(kString_is(Literal, sfp[1].asString)) {
		kString_set(Literal, resultValue, true);
	}
	KReturn(resultValue);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Static   kMethod_Static
//#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static void file_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_Const, _F(File_scriptPath), TY_String, TY_File, MN_("scriptPath"), 1, TY_String, FN_("filename"),
		_Public, _F(File_setWriterCharset), TY_void, TY_File, MN_("setWriterCharset"), 1, TY_String, FN_("charset"),
		_Public, _F(File_setReaderCharset), TY_void, TY_File, MN_("setReaderCharset"), 1, TY_String, FN_("charset"),

		_Public, _F(File_new), TY_File, TY_File, MN_("new"), 2, TY_String, FN_("filename"), TY_String, FN_("mode"),
		_Public, _F(File_close), TY_void, TY_File, MN_("close"), 0,
		_Public, _F(File_getc), TY_int, TY_File, MN_("getc"), 0,
		_Public, _F(File_putc), TY_void, TY_File, MN_("putc"), 1, TY_int, FN_("char"),
		_Public, _F(File_readLine), TY_String, TY_File, MN_("readLine"), 0,
		_Public, _F(File_print), TY_String, TY_File, MN_("print"), 1, TY_String, FN_("str")|FN_COERCION,
		_Public, _F(File_println), TY_void, TY_File, MN_("println"), 1, TY_String, FN_("str")|FN_COERCION,
		_Public, _F(File_println0), TY_void, TY_File, MN_("println"), 0,
		_Public, _F(File_flush), TY_void, TY_File, MN_("flush"), 0,

		_Public|_Const|_Im, _F(FILE_isatty), TY_boolean, TY_File, MN_("isatty"), 0,
		_Public|_Const|_Im, _F(FILE_getfileno), TY_int, TY_File, MN_("getfileno"), 0,

		_Public, _F(File_read),   TY_int, TY_File, MN_("read"), 1, TY_Bytes, FN_("buf"),
		_Public, _F(File_read3),  TY_int, TY_File, MN_("read"), 3, TY_Bytes, FN_("buf"), TY_int, FN_("offset"), TY_int, FN_("len"),
		_Public, _F(File_write),  TY_int, TY_File, MN_("write"), 1, TY_Bytes, FN_("buf"),
		_Public, _F(File_write3), TY_int, TY_File, MN_("write"), 3, TY_Bytes, FN_("buf"), TY_int, FN_("offset"), TY_int, FN_("len"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

typedef struct {
	const char *key;
	uintptr_t ty;
	kFile *value;
} KDEFINE_FILE_CONST;

static kFile* new_File(KonohaContext *kctx, kArray *gcstack, FILE *fp, const char *pathInfo, size_t len, KTraceInfo *trace)
{
	kFile *file = new_(File, fp, gcstack);
	file->fp = fp;
	KFieldInit(file, file->PathInfoNULL, KLIB new_kString(kctx, OnField, pathInfo, len, StringPolicy_ASCII|StringPolicy_TEXT));
	kFile_set(ChangeLessStream, file, true);
	if(!PLATAPI isSystemCharsetUTF8(kctx)) {
		if(fp == stdin) {
			file->readerIconv = PLATAPI iconvSystemCharsetToUTF8(kctx, trace);
		}
		else {
			file->writerIconv = PLATAPI iconvUTF8ToSystemCharset(kctx, trace);
		}
	}
	return file;
}

static void file_defineConst(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KFileStdIn  = new_File(kctx, OnGlobalConstList, stdin, TEXTSIZE("/dev/stdin"), trace);
	KFileStdOut = new_File(kctx, OnGlobalConstList, stdout, TEXTSIZE("/dev/stdout"), trace);
	KFileStdErr = new_File(kctx, OnGlobalConstList, stderr, TEXTSIZE("/dev/stderr"), trace);
	KDEFINE_FILE_CONST FileData[] = {
		{"stdin", TY_File,  KFileStdIn},
		{"stdout", TY_File, KFileStdOut},
		{"stderr", TY_File, KFileStdErr},
		{NULL}, /* sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(FileData), trace);
	KDEFINE_INT_CONST IntData[] = {
		{"EOF", TY_int,  EOF},
		{NULL}, /* sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(IntData), trace);

}

static kbool_t file_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	KRequirePackage("konoha.bytes", trace);
	if(CT_File == NULL) {
		KDEFINE_CLASS defFile = {
			.structname = "File",
			.typeId = TY_newid,
			.cstruct_size = sizeof(kFile),
			.cflag = kClass_Final,
			.init  = kFile_init,
			.reftrace = kFile_reftrace,
			.free  = kFile_free,
			.p     = kFile_p,
		};
		CT_File = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defFile, trace);
	}
	file_defineMethod(kctx, ns, trace);
	file_defineConst(kctx, ns, trace);
	return true;
}

static kbool_t file_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"FILE", VirtualType_KonohaClass, (uintptr_t)CT_File},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, exportNS, KonohaConst_(ClassData), 0);
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* file_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("konoha", "1.0"),
		.PackupNameSpace    = file_PackupNameSpace,
		.ExportNameSpace   = file_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
