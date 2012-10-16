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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct kio_t {
	union {
		int  fd;
		void *handler;
		FILE *fp;
		KGrowingBuffer wb;
	};
	void *handler2; // NULL
	int  isRunning;
	KGrowingArray buffer;
	size_t top; size_t tail;
	kbool_t (*_read)(KonohaContext *kctx, struct kio_t *);
	size_t  (*_write)(KonohaContext *kctx, struct kio_t *, const char *buf, size_t bufsiz);
	void    (*_close)(KonohaContext *kctx, struct kio_t *);
	kbool_t (*_blockread)(KonohaContext *kctx, struct kio_t *);
	kbool_t (*_unblockread)(KonohaContext *kctx, struct kio_t *);
	size_t  (*_blockwrite)(KonohaContext *kctx, struct kio_t *, const char *buf, size_t bufsiz);
	size_t  (*_unblockwrite)(KonohaContext *kctx, struct kio_t *, const char *buf, size_t bufsiz);
	const char *DBG_NAME;  // unnecessary for free
};

typedef void FILE_i;

typedef struct StreamApi {
	size_t (*read_i)(KonohaContext *kctx, FILE_i *fp, char *buf, size_t);
	size_t (*write_i)(KonohaContext *kctx, FILE_i *fp, const char *buf, size_t);
	kbool_t (*close_i)(KonohaContext *kctx, FILE_i *fp);
	kbool_t (*isEndOfStream)(KonohaContext *kctx, FILE_i *fp);
} StreamApi;

static size_t read_NOP(KonohaContext *kctx, FILE_i *fp, char *buf, size_t bufsiz)
{
	return 0;
}

static size_t write_NOP(KonohaContext *kctx, FILE_i *fp, const char *buf, size_t bufsiz)
{
	return 0;
}

static kbool_t close_NOP(KonohaContext *kctx, FILE_i *fp)
{
	return true;
}

static kbool_t isEndOfStreamNOP(KonohaContext *kctx, FILE_i *fp)
{
	return true;
}

static StreamApi defaultStreamApi = {
	read_NOP, write_NOP, close_NOP, isEndOfStreamNOP,
};

static size_t read_FILE(KonohaContext *kctx, FILE_i *fp, char *buf, size_t bufsiz)
{
	return fread(buf, 1, bufsiz, (FILE*)fp);
}

static size_t write_FILE(KonohaContext *kctx, FILE_i *fp, const char *buf, size_t bufsiz)
{
	size_t writtensize = fwrite(buf, 1, bufsiz, (FILE*)fp);
	fflush((FILE*)fp);
	return writtensize;
}

static kbool_t close_FILE(KonohaContext *kctx, FILE_i *fp)
{
	fclose((FILE*)fp);
	return true;
}

static kbool_t isEndOfStreamFILE(KonohaContext *kctx, FILE_i *fp)
{
	return feof((FILE*)fp);
}

static StreamApi FileStreamApi = {
	read_FILE, write_FILE, close_FILE, isEndOfStreamFILE,
};

#define ICONV_NULL       ((uintptr_t)-1)
#define HAS_ICONV(I)     (I != ICONV_NULL)

#ifdef _MSC_VER
#define kInputStream struct kInputStreamVar
#else
typedef struct kInputStreamVar kInputStream;
#endif

struct kInputStreamVar {
	KonohaObjectHeader h;
	FILE_i *fp;
	StreamApi *streamApi;
	uintptr_t iconv;
	KGrowingArray buffer;
	size_t top; size_t tail;
};

#ifdef _MSC_VER
#define kOutputStream struct kOutputStreamVar
#else
typedef struct kOutputStreamVar kOutputStream;
#endif

struct kOutputStreamVar {
	KonohaObjectHeader h;
	FILE *fp;
	StreamApi *streamApi;
	uintptr_t iconv;
	KGrowingArray buffer;
};

#ifdef _MSC_VER
#define kFile struct kFileVar
#else
typedef struct kFileVar kFile;
#endif

struct kFileVar {
	KonohaObjectHeader h;
	kString *path;
};

#define kioshare ((KIOModule *)kctx->modshare[MOD_IO])

typedef struct {
	KonohaModule h;
	kInputStream  *kstdin;
	kOutputStream *kstdout;
	kOutputStream *kstderr;
} KIOModule;

#define OutputStream_isAutoFlush(o)      (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define OutputStream_setAutoFlush(o,B)   TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local1,B)
#define OutputStream_isManualFlush(o)      (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local2))
#define OutputStream_setManualFlush(o,B)   TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local2,B)

static void kInputStream_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kInputStream *in = (kInputStream*)o;
	in->fp = NULL;
	in->streamApi = &defaultStreamApi;
	in->iconv = ICONV_NULL;
	in->buffer.bytesize = 0;
	in->buffer.bytebuf  = NULL;
	in->buffer.bytemax  = 0;
	in->top  = 0;
	in->tail = 0;
}

static void kInputStream_close(KonohaContext *kctx, kInputStream *in)
{
	if(in->fp != NULL) {
		in->streamApi->close_i(kctx, in->fp);
		in->streamApi = &defaultStreamApi;
		in->fp = NULL;
	}
	if(in->buffer.bytemax > 0) {
		KLIB Karray_free(kctx, &in->buffer);
		in->top = 0;
	}
}

static void kInputStream_free(KonohaContext *kctx, kObject *o)
{
	kInputStream_close(kctx, (kInputStream*)o);
}

static size_t kInputStream_readToBuffer(KonohaContext *kctx, kInputStream *in, KGrowingArray *buffer)
{
	if(!(buffer->bytesize + K_PAGESIZE < buffer->bytemax)) {
		KLIB Karray_expand(kctx, buffer, buffer->bytesize + K_PAGESIZE);
	}
	size_t n = in->streamApi->read_i(kctx, in->fp, buffer->bytebuf, K_PAGESIZE);
	buffer->bytesize += n;
	return n;
}

//static uintptr_t kOutputStream_writeBuffer(KonohaContext *kctx, kOutputStream *out, KGrowingArray *buffer, size_t offset)
//{
//	return out->streamApi->write_i(kctx, out->fp, buffer->bytebuf + offset, buffer->bytesize - offset);
//}

static void kOutputStream_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kOutputStream *out = (kOutputStream*)o;
	out->fp = NULL;
	out->buffer.bytesize = 0;
	out->buffer.bytebuf  = NULL;
	out->buffer.bytemax  = 0;
	out->streamApi = &defaultStreamApi;
	out->iconv = ICONV_NULL;
}

static void kOutputStream_flush(KonohaContext *kctx, kOutputStream *out)
{
	if(out->buffer.bytesize > 0) {
		out->streamApi->write_i(kctx, out->fp, out->buffer.bytebuf, out->buffer.bytesize);
		out->buffer.bytesize = 0;
	}
}

static void kOutputStream_close(KonohaContext *kctx, kOutputStream *out)
{
	if(OutputStream_isAutoFlush(out)) {
		kOutputStream_flush(kctx, out);
	}
	if(out->fp != NULL) {
		out->streamApi->close_i(kctx, out->fp);
		out->streamApi = &defaultStreamApi;
		out->fp = NULL;
	}
	if(out->buffer.bytemax > 0) {
		KLIB Karray_free(kctx, &out->buffer);
	}
}

static void kOutputStream_free(KonohaContext *kctx, kObject *o)
{
	kOutputStream_close(kctx, (kOutputStream*)o);
}

static void kOutputStream_write(KonohaContext *kctx, kOutputStream *out, const char *buf, size_t bufsiz)
{
	if(OutputStream_isManualFlush(out) && !(out->buffer.bytesize + bufsiz < out->buffer.bytemax)) {
		KLIB Karray_expand(kctx, &out->buffer, out->buffer.bytesize + bufsiz);
	}
	if(out->buffer.bytesize + bufsiz < out->buffer.bytemax) {
		memcpy(out->buffer.bytebuf + out->buffer.bytesize, buf, bufsiz);
		out->buffer.bytesize += bufsiz;
	}
	else {
		out->streamApi->write_i(kctx, out->fp, out->buffer.bytebuf, out->buffer.bytesize);
		if(bufsiz< out->buffer.bytemax) {
			memcpy(out->buffer.bytebuf, buf, bufsiz);
			out->buffer.bytesize = bufsiz;
		}
		else {
			out->streamApi->write_i(kctx, out->fp, buf, bufsiz);
			out->buffer.bytesize = 0;
		}
	}
}

static void kOutputStream_writeUTF8(KonohaContext *kctx, kOutputStream *out, const char *buf, size_t bufsiz)
{
	// TODO
	kOutputStream_write(kctx, out, buf, bufsiz);
}

static void kFile_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kFile *out = (kFile*)o;
	KFieldInit(out, out->path, K_NULL);
}

static void kFile_free(KonohaContext *kctx, kObject *o)
{

}

static void kFile_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kFile *file = (kFile *)o;
	BEGIN_REFTRACE(1);
	KREFTRACEv(file->path);
	END_REFTRACE();
}

/* ------------------------------------------------------------------------ */
/* Method */

//## method @Throwable InputStream InputStream.new(String urn);
static KMETHOD InputStream_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kInputStream *in = (kInputStream*)sfp[0].asObject;
	kString *path = sfp[1].asString;
	FILE *fp = fopen(S_text(path), "r");
	if(fp == NULL) {
		KMakeTrace(trace, sfp);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), NULL, trace);
	}
	in->fp = (FILE_i*)fp;
	in->streamApi = &FileStreamApi;
	KReturn(in);
}

//## method @public Int InputStream.getByte()
static KMETHOD InputStream_getByte(KonohaContext *kctx, KonohaStack *sfp)
{
	kInputStream *in = (kInputStream*)sfp[0].asObject;
	if(!(in->top < in->buffer.bytemax) && !(in->streamApi->isEndOfStream(kctx, in->fp))) {
		size_t readbyte = kInputStream_readToBuffer(kctx, in, &in->buffer);
		if(readbyte == 0) {
			kInputStream_close(kctx, in);
			KReturnUnboxValue(-1);
		}
	}
	int ch = in->buffer.bytebuf[in->top];
	in->top += 1;
	KReturnUnboxValue(ch);
}

static intptr_t findEndOfLine(KonohaContext *kctx, char *buf, size_t offset, size_t max, int *hasMultiByteChar, int *hasCarrigeReturn)
{
	size_t i;
	int ch, prev = 0;
	for(i = offset; i < max; i++) {
		ch = buf[i];
		if(ch == '\n') {
			if(prev == '\r') *hasCarrigeReturn = true;
			return i;
		}
		if(ch < 0) *hasMultiByteChar = true;
		prev = ch;
	}
	return -1; // not found
}

static kString* kInputStream_readLine(KonohaContext *kctx,  kInputStream *in)
{
	int hasMultiByteChar = false, hasCarrigeReturn = false;
	intptr_t endOfLineIdx;
	int isClosed = false;
	while((endOfLineIdx = findEndOfLine(kctx, in->buffer.bytebuf, in->top, in->buffer.bytesize, &hasMultiByteChar, &hasCarrigeReturn)) == -1) {
		size_t readbyte = kInputStream_readToBuffer(kctx, in, &in->buffer);
		if(readbyte == 0) {
			endOfLineIdx = in->buffer.bytesize;
			isClosed = true;
			break;
		}
	}
	kString *lineString;
	size_t len = hasCarrigeReturn ? endOfLineIdx - in->top - 1 : endOfLineIdx - in->top;
	if(!hasMultiByteChar || !HAS_ICONV(in->iconv)) {
		lineString = KLIB new_kString(kctx, OnStack, in->buffer.bytebuf + in->top, len, StringPolicy_ASCII);
	}
	else {
		//TODO;
		lineString = KLIB new_kString(kctx, OnStack, in->buffer.bytebuf + in->top, len, StringPolicy_UTF8);
	}
	in->top = endOfLineIdx + 1;
	if(in->top > K_PAGESIZE) {  // compaction
		size_t newsize = in->buffer.bytesize - in->top;
		memcpy(in->buffer.bytebuf, in->buffer.bytebuf + in->top, newsize);
		in->top = 0;
		in->buffer.bytesize = newsize;
	}
	if(isClosed) {
		kInputStream_close(kctx, in);
	}
	return lineString;
}

//## method @Iterative String InputStream.readLine();
static KMETHOD InputStream_readLine(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kInputStream_readLine(kctx, (kInputStream*)sfp[0].asObject));
}

//## method void InputStream.close();
static KMETHOD InputStream_close(KonohaContext *kctx, KonohaStack *sfp)
{
	kInputStream_close(kctx, (kInputStream*)sfp[0].asObject);
}

//## method @public boolean InputStream.isClosed()
static KMETHOD InputStream_isClosed(KonohaContext *kctx, KonohaStack *sfp)
{
	kInputStream *in = (kInputStream*)sfp[0].asObject;
	KReturnUnboxValue(in->top == 0 && in->streamApi->isEndOfStream(kctx, in->fp));
}

/* ------------------------------------------------------------------------ */

//## method @Throwable OutputStream OutputStream.new(String path, String mode);
static KMETHOD OutputStream_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kOutputStream *out = (kOutputStream*)sfp[0].asObject;
	kString *path = sfp[1].asString;
	const char *mode = IS_NULL(sfp[2].asString) ? "w" : S_text(sfp[2].asString);
	FILE *fp = fopen(S_text(path), mode);
	if(fp == NULL) {
		KMakeTrace(trace, sfp);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), NULL, trace);
	}
	out->fp = (FILE*)fp;
	out->streamApi = &FileStreamApi;
	KReturn(out);
}

//## method @public void OutputStream.putByte(int ch)
static KMETHOD OutputStream_putByte(KonohaContext *kctx, KonohaStack *sfp)
{
	kOutputStream *out = (kOutputStream*)sfp[0].asObject;
	char byte = (char)(sfp[1].intValue);
	kOutputStream_write(kctx, out, &byte, 1);
	KReturnVoid();
}

//## method void OutputStream.print(String value);
static KMETHOD OutputStream_print(KonohaContext *kctx, KonohaStack *sfp)
{
	kOutputStream *out = (kOutputStream*)sfp[0].asObject;
	kString *text = sfp[1].asString;
	if(!HAS_ICONV(out->iconv) || kString_is(ASCII, text)) {
		kOutputStream_write(kctx, out, S_text(text), S_size(text));
	}
	else {
		kOutputStream_writeUTF8(kctx, out, S_text(text), S_size(text));
	}
}

//## method void OutputStream.println(String value);
static KMETHOD OutputStream_println(KonohaContext *kctx, KonohaStack *sfp)
{
	kOutputStream *out = (kOutputStream*)sfp[0].asObject;
	kString *text = sfp[1].asString;
	if(!HAS_ICONV(out->iconv) || kString_is(ASCII, text)) {
		kOutputStream_write(kctx, out, S_text(text), S_size(text));
	}
	else {
		kOutputStream_writeUTF8(kctx, out, S_text(text), S_size(text));
	}
	kOutputStream_write(kctx, out, "\n", 1);  // FIXME: System based
	if(OutputStream_isAutoFlush(out)) {
		kOutputStream_flush(kctx, out);
	}
}

//## method void OutputStream.flush();
static KMETHOD OutputStream_flush(KonohaContext *kctx, KonohaStack *sfp)
{
	kOutputStream_flush(kctx, (kOutputStream*)sfp[0].asObject);
	KReturnVoid();
}

//## method void OutputStream.close();
static KMETHOD OutputStream_close(KonohaContext *kctx, KonohaStack *sfp)
{
	kOutputStream_close(kctx, (kOutputStream*)sfp[0].asObject);
}

//## method @public boolean OutputStream.isClosed()
static KMETHOD OutputStream_isClosed(KonohaContext *kctx, KonohaStack *sfp)
{
	kOutputStream *in = (kOutputStream*)sfp[0].asObject;
	KReturnUnboxValue(!(in->streamApi->isEndOfStream(kctx, in->fp)));
}

// --------------------------------------------------------------------------
//## method @public @static InputStream System.getIn()
static KMETHOD System_getIn(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kioshare->kstdin);
}

//## method @public @static OutputStream System.getOut()
static KMETHOD System_getOut(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kioshare->kstdout);
}

//## method @public @static OutputStream System.getErr()
static KMETHOD System_getErr(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kioshare->kstderr);
}

//## method @public @static boolean System.isDir(String x)
static KMETHOD System_isDir(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *filename = S_text(sfp[1].asString);
#ifdef _MSC_VER
	DWORD attr = GetFileAttributesA(filename);
	KReturnUnboxValue(attr != -1 && ((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat st;
	KReturnUnboxValue(stat(filename, &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

//## method @public @static boolean System.isFile(String x)
static KMETHOD System_isFile(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *filename = S_text(sfp[1].asString);
#ifdef _MSC_VER
	DWORD attr = GetFileAttributesA(filename);
	KReturnUnboxValue(attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat st;
	KReturnUnboxValue(stat(filename, &st) == 0 && S_ISREG(st.st_mode));
#endif
}

// --------------------------------------------------------------------------
//## method @public File File.new(String x)
static KMETHOD File_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *f = (kFile*)sfp[0].asObject;
	KFieldSet(f, f->path, sfp[1].asString);
	KReturn(f);
}

//## method @public boolean File.getPath()
static KMETHOD File_getPath(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *f = (kFile*)sfp[0].asObject;
	KReturn(f->path);
}

//## method @public boolean File.exists()
static KMETHOD File_exists(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *f = (kFile*)sfp[0].asObject;
	const char *filename = S_text(f->path);
#ifdef _MSC_VER
	KReturnUnboxValue(GetFileAttributes(filename) != -1);
#else
	struct stat st;
	KReturnUnboxValue(stat(filename, &st) == 0);
#endif
}

#ifdef _MSC_VER
kbool_t GetLargeFileSize(const char *filename, unsigned long long *size)
{
    DWORD fileSizeLow, fileSizeHigh;
	HANDLE file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE) return false;
    fileSizeLow = GetFileSize(file, &fileSizeHigh);
    *size = ((unsigned long long)fileSizeHigh << 32) + fileSizeLow;
    CloseHandle(file);
    if(fileSizeLow == -1 && (GetLastError() != NO_ERROR)) {
        return false;
    }
    return true;
}
#endif

//## method @public boolean File.getSize()
static KMETHOD File_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *f = (kFile*)sfp[0].asObject;
	const char *filename = S_text(f->path);
#ifdef _MSC_VER
	unsigned long long size;
	KReturnUnboxValue(GetLargeFileSize(filename, &size) ? size : 0);
#else
	struct stat st;
	KReturnUnboxValue(stat(filename, &st) == 0 ? st.st_size : 0);
#endif
}

//## method @public boolean File.isDir()
static KMETHOD File_isDir(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *f = (kFile*)sfp[0].asObject;
	const char *filename = S_text(f->path);
#ifdef _MSC_VER
	DWORD attr = GetFileAttributesA(filename);
	KReturnUnboxValue(attr != -1 && ((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat st;
	KReturnUnboxValue(stat(filename, &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

//## method @public boolean File.isFile()
static KMETHOD File_isFile(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *f = (kFile*)sfp[0].asObject;
	const char *filename = S_text(f->path);
#ifdef _MSC_VER
	DWORD attr = GetFileAttributesA(filename);
	KReturnUnboxValue(attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat st;
	KReturnUnboxValue(stat(filename, &st) == 0 && S_ISREG(st.st_mode));
#endif
}

//## method @public boolean File.rename(String newName)
static KMETHOD File_rename(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *f = (kFile*)sfp[0].asObject;
	const char *oldName = S_text(f->path);
	const char *newName = S_text(sfp[1].asString);
	KReturnUnboxValue(rename(oldName, newName) == 0); // success: true
}

//## method @public boolean File.remove()
static KMETHOD File_remove(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *f = (kFile*)sfp[0].asObject;
	const char *filename = S_text(f->path);
	KReturnUnboxValue(remove(filename) == 0); // success: true
}

//## method @public Array[String] File.list()
static KMETHOD File_list(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kFile *f = (kFile*)sfp[0].asObject;
	const char *dirname = S_text(f->path);
	kArray *resultArray = (kArray*)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
#ifdef _MSC_VER
	HANDLE findhandle;
	WIN32_FIND_DATA fileinfo;
	findhandle = FindFirstFile(dirname, &fileinfo);
	if(findhandle == INVALID_HANDLE_VALUE){
		KReturn(a);
	}
	do {
		KLIB new_kString(kctx, a, fileinfo.cFileName, strlen(fileinfo.cFileName), 0);
	} while (FindNextFile(findhandle, &fileinfo));

	FindClose(findhandle);
#else
	DIR *dir = opendir(dirname);
	if(dir != NULL) {
		struct dirent *e;
		while((e = readdir(dir)) != NULL) {
			const char *fname = e->d_name;
			if(strcmp(fname, ".") != 0 && strcmp(fname, "..") != 0) {
				KLIB new_kString(kctx, resultArray, fname, strlen(fname), 0);
			}
		}
		closedir(dir);
	}
#endif
	KReturnWith(resultArray, RESET_GCSTACK());
}

// --------------------------------------------------------------------------

static void kioshare_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
}

static void kioshare_reftrace(KonohaContext *kctx, struct KonohaModule *baseh, KObjectVisitor *visitor)
{
	KIOModule *base = (KIOModule *)baseh;
	BEGIN_REFTRACE(3);
	KREFTRACEv(base->kstdin);
	KREFTRACEv(base->kstdout);
	KREFTRACEv(base->kstderr);
	END_REFTRACE();
}

static void kioshare_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFree(baseh, sizeof(KIOModule));
}

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t io_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KIOModule *mod = (KIOModule*)KCalloc_UNTRACE(sizeof(KIOModule), 1);
	mod->h.name     = "io";
	mod->h.setup    = kioshare_setup;
	mod->h.reftrace = kioshare_reftrace;
	mod->h.free     = kioshare_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_IO, &mod->h, trace);

	KDEFINE_CLASS defInputStream = {0};
	SETSTRUCTNAME(defInputStream, InputStream);
	defInputStream.cflag = kClass_Final;
	defInputStream.init  = kInputStream_init;
	defInputStream.free  = kInputStream_free;
	
	KDEFINE_CLASS defOutputStream = {0};
	SETSTRUCTNAME(defOutputStream, OutputStream);
	defOutputStream.cflag = kClass_Final;
	defOutputStream.init  = kOutputStream_init;
	defOutputStream.free  = kOutputStream_free;
	
	KDEFINE_CLASS defFile = {0};
	SETSTRUCTNAME(defFile, File);
	defFile.cflag    = kClass_Final;
	defFile.init     = kFile_init;
	defFile.reftrace = kFile_reftrace;
	defFile.free     = kFile_free;
	

	KonohaClass *cInputStream  = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defInputStream, trace);
	KonohaClass *cOutputStream = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defOutputStream, trace);
	KonohaClass *cFile         = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defFile, trace);
	int TY_InputStream = cInputStream->typeId;
	int TY_OutputStream = cOutputStream->typeId;
	int TY_File = cFile->typeId;

#define CT_OutputStream CT_(TY_OutputStream)
#define CT_InputStream  CT_(TY_InputStream)
	// system.in
	mod->kstdin = new_(InputStream, NULL, OnGlobalConstList);
	kInputStream_init(kctx, (kObject *)mod->kstdin, NULL);
	mod->kstdin->fp = (FILE_i*)stdin;
	mod->kstdin->streamApi = &FileStreamApi;
	// system.out
	mod->kstdout = new_(OutputStream, NULL, OnGlobalConstList);
	kOutputStream_init(kctx, (kObject *)mod->kstdout, NULL);
	mod->kstdout->fp = (FILE*)stdout;
	mod->kstdout->streamApi = &FileStreamApi;
	// system.err
	mod->kstderr = new_(OutputStream, NULL, OnGlobalConstList);
	kOutputStream_init(kctx, (kObject *)mod->kstderr, NULL);
	mod->kstderr->fp = (FILE*)stderr;
	mod->kstderr->streamApi = &FileStreamApi;

	kparamtype_t p = {0};
	p.ty = TY_String;
	KonohaClass *cStrArray = KLIB KonohaClass_Generics(kctx, CT_(TY_Array), TY_void, 1, &p);
#define TY_StrArray (cStrArray->typeId)

	int FN_path = FN_("path");
	int FN_mode = FN_("mode");
	int FN_value = FN_("value");
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(InputStream_getByte),  TY_int,         TY_InputStream,  MN_("getByte"), 0,
		_Public, _F(InputStream_isClosed), TY_boolean,     TY_InputStream,  MN_("isClosed"), 0,
		_Public, _F(InputStream_new),      TY_InputStream, TY_InputStream,  MN_("new"), 1, TY_String, FN_path,
		_Public, _F(InputStream_close),    TY_void,        TY_InputStream,  MN_("close"), 0,
		_Public, _F(InputStream_readLine), TY_String,      TY_InputStream,  MN_("readLine"), 0,
		_Public, _F(OutputStream_putByte),  TY_void,         TY_OutputStream, MN_("putByte"), 1, TY_int, FN_x,
		_Public, _F(OutputStream_isClosed), TY_boolean,      TY_OutputStream, MN_("isClosed"), 0,
		_Public, _F(OutputStream_new),      TY_OutputStream, TY_OutputStream, MN_("new"), 2, TY_String, FN_path, TY_String, FN_mode,
		_Public, _F(OutputStream_print),    TY_void,         TY_OutputStream, MN_("print"), 1,   TY_String, FN_value|_Coercion,
		_Public, _F(OutputStream_println),  TY_void,         TY_OutputStream, MN_("println"), 1, TY_String, FN_value|_Coercion,
		_Public, _F(OutputStream_flush),    TY_void,         TY_OutputStream, MN_("flush"), 0,
		_Public, _F(OutputStream_close),    TY_void,         TY_OutputStream, MN_("close"), 0,

		_Public, _F(File_new),     TY_File,        TY_File, MN_("new"), 1, TY_String, FN_x,
		_Public, _F(File_getPath), TY_String,      TY_File, MN_("getPath"), 0,
		_Public, _F(File_exists),  TY_boolean,     TY_File, MN_("exists"), 0,
		_Public, _F(File_isDir),   TY_boolean,     TY_File, MN_("isDir"), 0,
		_Public, _F(File_isFile),  TY_boolean,     TY_File, MN_("isFile"), 0,
		_Public, _F(File_getSize), TY_int,         TY_File, MN_("getSize"), 0,
		_Public, _F(File_getSize), TY_int,         TY_File, MN_("length"), 0,
		_Public, _F(File_rename),  TY_boolean,     TY_File, MN_("rename"), 1, TY_String, FN_x,
		_Public, _F(File_remove),  TY_boolean,     TY_File, MN_("remove"), 0,
		_Public, _F(File_list),    TY_StrArray,    TY_File, MN_("list"), 0,

		_Public|_Static, _F(System_isDir),  TY_boolean,      TY_System, MN_("isDir"), 1, TY_String, FN_x,
		_Public|_Static, _F(System_isFile), TY_boolean,      TY_System, MN_("isFile"), 1, TY_String, FN_x,
		_Public|_Static, _F(System_getIn ), TY_InputStream,  TY_System, MN_("getIn"), 0,
		_Public|_Static, _F(System_getOut), TY_OutputStream, TY_System, MN_("getOut"), 0,
		_Public|_Static, _F(System_getErr), TY_OutputStream, TY_System, MN_("getErr"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, NULL, MethodData);
	return true;
}

static kbool_t io_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

static kbool_t io_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

static kbool_t io_setNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* io_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "io", "1.0");
	d.initPackage    = io_initPackage;
	d.setupPackage   = io_setupPackage;
	d.initNameSpace  = io_initNameSpace;
	d.setupNameSpace = io_setNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
