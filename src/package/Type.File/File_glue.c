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

#define USE_FILE 1
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifndef K_PATHMAX
#define K_PATHMAX 1024
#endif

#define I18NAPI PLATAPI I18NModule.

#define LogFileName(S)     LogText("filename", S)
#define LogFile(F) LogText("filename", kFile_textPath(kctx, F))
#define LogWrittenByte(size)  LogUint("WrittenByteSize", size)

static const char* kFile_textPath(KonohaContext *kctx, kFile *file)
{
	return (file->PathInfoNULL == NULL) ? "unknown" : kString_text(file->PathInfoNULL);
}

static int TRACE_fgetc(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
{
	int ch = fgetc(file->fp);
	if(ferror(file->fp) != 0) {
		KTraceErrorPoint(trace, SystemFault, "fgetc", LogFile(file), LogErrno);
		KLIB KRuntime_raise(kctx, KException_("IO"), SystemFault, NULL, trace->baseStack);
	}
	return ch;
}

static int TRACE_fputc(KonohaContext *kctx, kFile *file, int ch, KTraceInfo *trace)
{
	if(fputc(ch, file->fp) == EOF) {
		KTraceErrorPoint(trace, SystemFault, "fputc", LogFile(file), LogErrno);
		KLIB KRuntime_raise(kctx, KException_("IO"), SystemFault, NULL, trace->baseStack);
	}
	else if(!kFile_is(ChangeLessStream, file)) {
		KTraceChangeSystemPoint(trace, "fputc", LogFile(file), LogWrittenByte(1));
	}
	return ch;
}

static void kFile_close(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
{
	int ret = 0;
	DBG_ASSERT(file->fp != NULL);
	if(!(file->fp == stdin || file->fp == stdout || file->fp == stderr)) {
		ret = fclose(file->fp);
	}
	if(ret != 0) {
		KTraceErrorPoint(trace, SoftwareFault|SystemFault, "fclose", LogErrno);
	}
	file->fp = NULL;
	if(file->readerIconv != ICONV_NULL) {
		I18NAPI iconv_close_i(kctx, file->readerIconv);
		file->readerIconv = ICONV_NULL;
	}
	if(file->writerIconv != ICONV_NULL) {
		I18NAPI iconv_close_i(kctx, file->writerIconv);
		file->writerIconv = ICONV_NULL;
	}
}

static void kFile_CheckEOF(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
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
		KLIB KRuntime_raise(kctx, KException_("IO"), SystemFault, NULL, trace->baseStack);
	}
	kFile_CheckEOF(kctx, file, trace);
	return size;
}

static size_t TRACE_fwrite(KonohaContext *kctx, kFile *file, const char *buf, size_t bufsiz, KTraceInfo *trace)
{
	size_t size = fwrite(buf, 1, bufsiz, file->fp);
	if(ferror(file->fp) != 0){
		KTraceErrorPoint(trace, SystemFault, "fwrite", LogFile(file), LogErrno);
		KLIB KRuntime_raise(kctx, KException_("IO"), SystemFault, NULL, trace->baseStack);
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

static void kFile_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kFileVar *file = (struct kFileVar *)o;
	file->fp = (conf != NULL) ? conf : NULL;
	file->PathInfoNULL = NULL;
	file->readerIconv = ICONV_NULL;
	file->writerIconv = ICONV_NULL;
}

static void kFile_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kFile *file = (kFile *)o;
	KRefTraceNullable(file->PathInfoNULL);
}

static void kFile_Free(KonohaContext *kctx, kObject *o)
{
	struct kFileVar *file = (struct kFileVar *)o;
	if(file->fp != NULL) {
		kFile_close(kctx, file, NULL/*trace*/);
	}
}

static void kFile_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kFile *file = (kFile *)v[pos].asObject;
	if(file->PathInfoNULL != NULL) {
		KLIB KBuffer_Write(kctx, wb, kString_text(file->PathInfoNULL), kString_size(file->PathInfoNULL));
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "FILE:%p", file->fp);
	}
}

/* ------------------------------------------------------------------------ */

//## FILE FILE.new(String path, String mode);
static KMETHOD File_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = I18NAPI formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	const char *mode = kString_text(sfp[2].asString);
	FILE *fp = fopen(systemPath, mode);
	kFile *file = (kFile *) sfp[0].asObject;
	if(fp == NULL) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "fopen",
			LogText("filename", kString_text(path)), LogText("mode", mode), LogErrno);
		KLIB KRuntime_raise(kctx, KException_("IO"), fault, NULL, sfp);
	}
	if(mode[0] == 'w' || mode[0] == 'a' || mode[1] == '+') {
		KTraceChangeSystemPoint(trace, "fopen", LogFileName(kString_text(path)), LogText("mode", mode));
	}
	file->fp = fp;
	KFieldInit(file, file->PathInfoNULL, path);
	if(!I18NAPI isSystemCharsetUTF8(kctx)) {
		if(mode[0] == 'w' || mode[0] == 'a' || mode[1] == '+') {
			file->writerIconv = I18NAPI iconvUTF8ToSystemCharset(kctx, trace);
		}
		else {
			file->readerIconv = I18NAPI iconvSystemCharsetToUTF8(kctx, trace);
		}
	}
	KReturn(file);
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
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
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
					KLIB KBuffer_iconv(kctx, &wb, file->readerIconv, buffer, pos, trace);
				}
				else {
					KLIB KBuffer_Write(kctx, &wb, buffer, pos);
				}
				bufferCount++;
				hasUTF8 = false;
				pos = 0;
			}
		}
		if(pos > 0) {
			if(hasUTF8 && file->readerIconv != ICONV_NULL) {
				KLIB KBuffer_iconv(kctx, &wb, file->readerIconv, buffer, pos, trace);
			}
			else {
				KLIB KBuffer_Write(kctx, &wb, buffer, pos);
			}
		}
		kFile_CheckEOF(kctx, file, trace);
		KReturn(KLIB KBuffer_Stringfy(kctx, &wb, OnStack, policy | StringPolicy_FreeKBuffer));
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
	if(file->writerIconv == ICONV_NULL || kString_Is(ASCII, line)) {
		TRACE_fwrite(kctx, file, kString_text(line), kString_size(line), trace);
	}
	else {
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KLIB KBuffer_iconv(kctx, &wb, file->writerIconv, kString_text(line), kString_size(line), trace);
		TRACE_fwrite(kctx, file, KLIB KBuffer_text(kctx, &wb, NonZero), KBuffer_bytesize(&wb), trace);
		KLIB KBuffer_Free(&wb);
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
static KMETHOD File_Write(KonohaContext *kctx , KonohaStack *sfp)
{
	kFile *file = sfp[0].asFile;
	kBytes *ba  = sfp[1].asBytes;
	KMakeTrace(trace, sfp);
	size_t writtenbyte = TRACE_fwrite(kctx, file, ba->buf, ba->bytesize, trace);
	KReturnUnboxValue(writtenbyte);
}

//## @Native int File.write(Bytes buf, int offset, int len);
static KMETHOD File_Write3(KonohaContext *kctx , KonohaStack *sfp)
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
static KMETHOD File_SetReaderCharset(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile   *file = sfp[0].asFile;
	KMakeTrace(trace, sfp);
	file->readerIconv = I18NAPI iconv_open_i(kctx, "UTF-8", kString_text(sfp[1].asString), trace);
}

//## void setWriterCharset(String charset);
static KMETHOD File_SetWriterCharset(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile   *file = sfp[0].asFile;
	KMakeTrace(trace, sfp);
	file->writerIconv = I18NAPI iconv_open_i(kctx, kString_text(sfp[1].asString), "UTF-8", trace);
}

/* ------------------------------------------------------------------------ */

//## @Const String File.scriptPath(String path);
static KMETHOD File_scriptPath(KonohaContext *kctx, KonohaStack *sfp)
{
	char scriptPathBuf[K_PATHMAX];
	const char *scriptPath = PLATAPI formatTransparentPath(scriptPathBuf, sizeof(scriptPathBuf), KFileLine_textFileName(sfp[K_RTNIDX].calledFileLine), kString_text(sfp[1].asString));
	kStringVar *resultValue = (kStringVar *)KLIB new_kString(kctx, OnStack, scriptPath, strlen(scriptPath), 0);
	if(kString_Is(Literal, sfp[1].asString)) {
		kString_Set(Literal, resultValue, true);
	}
	KReturn(resultValue);
}

// --------------------------------------------------------------------------

static void file_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_Const, _F(File_scriptPath), KType_String, KType_File, KMethodName_("scriptPath"), 1, KType_String, KFieldName_("filename"),
		_Public, _F(File_SetWriterCharset), KType_void, KType_File, KMethodName_("setWriterCharset"), 1, KType_String, KFieldName_("charset"),
		_Public, _F(File_SetReaderCharset), KType_void, KType_File, KMethodName_("setReaderCharset"), 1, KType_String, KFieldName_("charset"),

		_Public, _F(File_new), KType_File, KType_File, KMethodName_("new"), 2, KType_String, KFieldName_("filename"), KType_String, KFieldName_("mode"),
		_Public, _F(File_close), KType_void, KType_File, KMethodName_("close"), 0,
		_Public, _F(File_getc), KType_Int, KType_File, KMethodName_("getc"), 0,
		_Public, _F(File_putc), KType_void, KType_File, KMethodName_("putc"), 1, KType_Int, KFieldName_("char"),
		_Public, _F(File_readLine), KType_String, KType_File, KMethodName_("readLine"), 0,
		_Public, _F(File_print), KType_String, KType_File, KMethodName_("print"), 1, KType_String | KTypeAttr_Coercion, KFieldName_("str"),
		_Public, _F(File_println), KType_void, KType_File, KMethodName_("println"), 1, KType_String | KTypeAttr_Coercion, KFieldName_("str"),
		_Public, _F(File_println0), KType_void, KType_File, KMethodName_("println"), 0,
		_Public, _F(File_flush), KType_void, KType_File, KMethodName_("flush"), 0,

		_Public|_Const|_Im, _F(FILE_isatty), KType_Boolean, KType_File, KMethodName_("isatty"), 0,
		_Public|_Const|_Im, _F(FILE_getfileno), KType_Int, KType_File, KMethodName_("getfileno"), 0,

		_Public, _F(File_read),   KType_Int, KType_File, KMethodName_("read"), 1, KType_Bytes, KFieldName_("buf"),
		_Public, _F(File_read3),  KType_Int, KType_File, KMethodName_("read"), 3, KType_Bytes, KFieldName_("buf"), KType_Int, KFieldName_("offset"), KType_Int, KFieldName_("len"),
		_Public, _F(File_Write),  KType_Int, KType_File, KMethodName_("write"), 1, KType_Bytes, KFieldName_("buf"),
		_Public, _F(File_Write3), KType_Int, KType_File, KMethodName_("write"), 3, KType_Bytes, KFieldName_("buf"), KType_Int, KFieldName_("offset"), KType_Int, KFieldName_("len"),
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
	kFile_Set(ChangeLessStream, file, true);
	if(!I18NAPI isSystemCharsetUTF8(kctx)) {
		if(fp == stdin) {
			file->readerIconv = I18NAPI iconvSystemCharsetToUTF8(kctx, trace);
		}
		else {
			file->writerIconv = I18NAPI iconvUTF8ToSystemCharset(kctx, trace);
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
		{"stdin", KType_File,  KFileStdIn},
		{"stdout", KType_File, KFileStdOut},
		{"stderr", KType_File, KFileStdErr},
		{NULL}, /* sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(FileData), trace);
	KDEFINE_INT_CONST IntData[] = {
		{"EOF", KType_Int,  EOF},
		{NULL}, /* sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);

}

static kbool_t file_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	KRequirePackage("Type.Bytes", trace);
	if(KClass_File == NULL) {
		KDEFINE_CLASS defFile = {
			.structname = "File",
			.typeId = KTypeAttr_NewId,
			.cstruct_size = sizeof(kFile),
			.cflag = KClassFlag_Final,
			.init  = kFile_Init,
			.reftrace = kFile_Reftrace,
			.free   = kFile_Free,
			.format = kFile_format,
		};
		KClass_File = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defFile, trace);
	}
	file_defineMethod(kctx, ns, trace);
	file_defineConst(kctx, ns, trace);
	return true;
}

static kbool_t file_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"FILE", VirtualType_KClass, (uintptr_t)KClass_File},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, exportNS, KConst_(ClassData), trace);
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *File_Init(void)
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
