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

#if 0
#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>
#include <unistd.h>
#define MOD_IO 20/*FIXME*/
#define KNH_TODO(msg) do {\
	fprintf(stderr, "TODO(%s) : %s at %s:%d",\
			msg, __FUNCTION__, __FILE__, __LINE__);\
	abort();\
} while (0)
#define KNH_NTRACE2(...) KNH_TODO("ntrace")
 #define knh_stack_argc(_ctx, sfp)   (_ctx->esp - (sfp))

static kString *kwb_newString(CTX, kwb_t *wb, int flg)
{
	return new_kString(kwb_top(wb, flg), kwb_bytesize(wb), SPOL_POOL);
}

/* ------------------------------------------------------------------------ */
/* io module */
#define kiomod        ((kiomod_t*)_ctx->mod[MOD_IO])
#define kioshare      ((kioshare_t*)_ctx->modshare[MOD_IO])
#define CT_InputStream       kioshare->cInputStream
#define TY_InputStream       kioshare->cInputStream->cid
#define CT_OutputStream      kioshare->cOutputStream
#define TY_OutputStream      kioshare->cOutputStream->cid

#define IS_io(O)      ((O)->h.ct == CT_io)

typedef struct {
	kmodshare_t h;
	kclass_t *cInputStream;
	kclass_t *cOutputStream;
} kioshare_t;

typedef struct {
	kmodlocal_t h;
} kiomod_t;

/* ------------------------------------------------------------------------ */
//## class Path Object;
//## flag Path Trusted        1 - is set * *;
//## flag Path Temporary      2 - is set * *;

#ifdef PATH_MAX
#define K_PATHMAX PATH_MAX
#else
#define K_PATHMAX 256
#endif

typedef struct kPath kPath;
#define IO_NULL   ((kuintptr_t)0)
#define IO_BUF    ((kuintptr_t)1)
#define K_STREAM_BUFSIZ  K_PAGESIZE
#define K_OUTBUF_MAXSIZ      (512L * 1024 * 1024)  // 512Mb

#define K_STREAM_NULL      0
#define K_STREAM_INMEMORY  1
#define K_STREAM_STDIO     2
#define K_STREAM_FILE      3
#define K_STREAM_FD        4
#define K_STREAM_NET       5

typedef struct kio_t kio_t;

struct kio_t {
	union {
		int  fd;
		void *handler;
		FILE *fp;
		kwb_t wb;
	};
	void *handler2; // NULL
	int  isRunning;
	karray_t buffer;
	size_t top; size_t tail;
	kbool_t (*_read)(CTX, struct kio_t *);
	size_t  (*_write)(CTX, struct kio_t *, const char *buf, size_t bufsiz);
	void    (*_close)(CTX, struct kio_t *);
	kbool_t (*_blockread)(CTX, struct kio_t *);
	kbool_t (*_unblockread)(CTX, struct kio_t *);
	size_t  (*_blockwrite)(CTX, struct kio_t *, const char *buf, size_t bufsiz);
	size_t  (*_unblockwrite)(CTX, struct kio_t *, const char *buf, size_t bufsiz);
	const char *DBG_NAME;  // unnecessary for free
};

struct kNameSpace;
typedef struct kDictMap kDictMap;

typedef struct knh_PathDPI_t {
	int type;
	const char   *name;
	size_t       wbufsiz;  // write bufsize
	kbool_t     (*existsSPI)(CTX, struct kPath *);
	void        (*ospath)(CTX, struct kPath *, struct kNameSpace *);
	// stream
	kio_t*   (*io2openNULL)(CTX, struct kPath*, const char *, kDictMap *);
	//knh_Fitrnext  qnextData;
} knh_PathDPI_t;

struct kPath {
	kObjectHeader h;
	const char              *ospath;
	size_t                   asize;
	struct kString          *urn;
	const struct knh_PathDPI_t   *dpi;
};

/* ------------------------------------------------------------------------ */
//## class InputStream Object;

#define KNH_STDIN          (ctx->in)
#define KNH_STDOUT         (ctx->out)
#define KNH_STDERR         (ctx->err)

#define knh_InputStream_getc(ctx, in)          io2_getc(ctx, (in)->io2)
#define knh_InputStream_readLine(ctx, in)      io2_readLine(ctx, (in)->io2, (in)->decNULL)
#define knh_InputStream_close(ctx, in)         io2_close(ctx, (in)->io2)

typedef struct kInputStream kInputStream;
struct kInputStream {
	kObjectHeader h;
	kio_t *io2;
	kString *path;
	struct kStringDecoder* decNULL;
};

/* ------------------------------------------------------------------------ */
//## class OutputStream Object;
//## flag OutputStream BOL            1 - is set * *;
//## flag OutputStream AutoFlush      2 - is set is set;
//## flag OutputStream UTF8           3 - has set * *;
#define FLAG_OutputStream_BOL kObject_Local1
#define OutputStream_isBOL(o)  (TFLAG_is(uintptr_t,(o)->h.magicflag,FLAG_OutputStream_BOL))
#define OutputStream_setBOL(o,b) TFLAG_set(uintptr_t,(o)->h.magicflag,FLAG_OutputStream_BOL,b)
#define FLAG_OutputStream_AutoFlush kObject_Local2
#define OutputStream_isAutoFlush(o)  (TFLAG_is(uintptr_t,(o)->h.magicflag,FLAG_OutputStream_AutoFlush))
#define OutputStream_setAutoFlush(o,b) TFLAG_set(uintptr_t,(o)->h.magicflag,FLAG_OutputStream_AutoFlush,b)
#define FLAG_OutputStream_UTF8 kObject_Local3
#define OutputStream_hasUTF8(o)  (TFLAG_is(uintptr_t,(o)->h.magicflag,FLAG_OutputStream_UTF8))
#define OutputStream_setUTF8(o,b) TFLAG_set(uintptr_t,(o)->h.magicflag,FLAG_OutputStream_UTF8,b)

typedef struct kOutputStream kOutputStream;

struct kOutputStream {
	kObjectHeader h;
	kio_t *io2;
	struct kString *path;
	struct kBytes *bufferNULL;
	struct kStringEncoder* encNULL;
};

#define knh_putc(ctx, w, ch)       knh_OutputStream_putc(ctx, w, ch)
#define knh_write(ctx, w, s)       knh_OutputStream_write(ctx, w, s)
#define knh_flush(ctx, w)          knh_OutputStream_flush(ctx, w)
#define knh_write_delim(ctx, w)    knh_write_ascii(ctx, w, ", ")
#define knh_write_dots(ctx, w)     knh_write_ascii(ctx, w, "...")
#define knh_write_delimdots(ctx, w)     knh_write_ascii(ctx, w, ", ...")
#define knh_write_fn(ctx, w, fn)   knh_write_ascii(ctx, w, FN__(fn))
#define knh_write__O(ctx, w, o)    knh_write_Object(ctx, w, MN__k, o)

/* ------------------------------------------------------------------------ */

// io
static void io2_free(CTX, kio_t *io2);
static void Stream_init(CTX, kObject *o, void *conf)
{
	kInputStream *in = (kInputStream *) o;
	in->io2 = (kio_t*) conf;;
}

static void Stream_free(CTX, kObject *o)
{
	kInputStream *in = (kInputStream*)o;
	if (in->io2) {
		io2_free(_ctx, in->io2);
		in->io2 = NULL;
	}
}


static void OutputStream_free(CTX, kObject *o)
{
	kInputStream *in = (kInputStream*)o;
	if (in->io2) {
		io2_free(_ctx, in->io2);
		in->io2 = NULL;
	}
}

static KDEFINE_CLASS InputStreamDef = {
	STRUCTNAME(InputStream),
	.cflag = 0,
	.init = Stream_init,
	.free = Stream_free,
};

static KDEFINE_CLASS OutputStreamDef = {
	STRUCTNAME(OutputStream),
	.cflag = 0,
	.init = Stream_init,
	.free = OutputStream_free,
};


static void kioshare_setup(CTX, struct kmodshare_t *def)
{
}

static void kioshare_reftrace(CTX, struct kmodshare_t *baseh)
{
}

static void kioshare_free(CTX, struct kmodshare_t *baseh)
{
	KFREE(baseh, sizeof(kioshare_t));
}

/* ------------------------------------------------------------------------ */
/* K_DPI_STREAM */

static size_t io2_writeNOP(CTX, kio_t *io2, const char *buf, size_t bufsiz)
{
	return 0;
}
static kbool_t io2_readNOP(CTX, kio_t *io2)
{
	return 0;
}
static void io2_closeNOP(CTX, kio_t *io2)
{
}

static void io2_flush(CTX, kio_t *io2)
{
	if(io2->buffer.max > 0 && io2->tail > 0) {
		io2->_write(_ctx, io2, io2->buffer.buf, io2->tail);
		io2->tail = 0;
	}
}

static void io2_close(CTX, kio_t *io2)
{
	if(io2->isRunning == 1) {
		io2_flush(_ctx, io2);
		io2->_close(_ctx, io2);
		io2->top  = 0;
		io2->tail = 0;
	}
}

static void io2_check_buffer_inited(CTX, kio_t *io2, size_t bufsiz)
{
	if(io2->buffer.max == 0) {
		KARRAY_INIT(io2->buffer, bufsiz, char);
	}
}

static kbool_t io2_readFILE(CTX, kio_t *io2)
{
	io2_check_buffer_inited(_ctx, io2, K_PAGESIZE);
	size_t size = fread(io2->buffer.buf, 1, io2->buffer.max, io2->fp);
	if(size == 0) {
		int tf = 1;
		if(ferror(io2->fp)) {
			KNH_NTRACE2(_ctx, "fread", K_PERROR, KNH_LDATA(LOG_p("fp", io2->fp), LOG_s("path", io2->DBG_NAME)));
			tf = 0;
		}
		io2->_close(_ctx, io2);
		return 0;
	}
	else {
		io2->top = 0;
		io2->tail = size;
		return 1;
	}
}

static size_t io2_writeFILE(CTX, kio_t *io2, const char *buf, size_t bufsiz)
{
	size_t size;

	fflush(io2->fp);
	size = fwrite(buf, 1, bufsiz, io2->fp);
	if(size == 0) {
		if(ferror(io2->fp)) {
			KNH_NTRACE2(_ctx, "fwrite", K_PERROR, KNH_LDATA(LOG_i("fp", io2->fp), LOG_s("path", io2->DBG_NAME)));
			io2->_close(_ctx, io2);
		}
		return 0;
	}
	fflush(io2->fp);
	return size;
}

static void io2_closeFILE(CTX, kio_t *io2)
{
	DBG_ASSERT(io2->isRunning == 1);
	fclose(io2->fp);
	io2->isRunning = 0;
}

static kio_t* new_FILE(CTX, FILE *fp, size_t bufsiz)
{
	kio_t *io2 = KCALLOC(sizeof(kio_t));
	io2->fp  = fp;
	io2->isRunning = 1;
	if(bufsiz > 0) {
		KARRAY_INIT(io2->buffer, bufsiz, char);
	}
	io2->top  = 0;
	io2->tail = 0;
	io2->_close         = io2_closeFILE;
	io2->_blockread     = io2_readFILE;
	io2->_unblockread   = io2_readFILE;
	io2->_read          = io2_readFILE;
	io2->_blockwrite    = io2_writeFILE;
	io2->_unblockwrite  = io2_writeFILE;
	io2->_write         = io2_writeFILE;
	return io2;
}

//static kbool_t io2_blockread(CTX, kio_t *io2)
//{
//	io2_check_buffer_inited(_ctx, io2, K_PAGESIZE);
//	int fd = io2->fd;
//	ssize_t size = read(fd, io2->buffer.buf, io2->buffer.max);
//	if(size == -1) {
//		KNH_NTRACE2(_ctx, "read", K_PERROR, KNH_LDATA(LOG_i("fd", fd), LOG_s("path", io2->DBG_NAME)));
//		io2->_close(_ctx, io2);
//		return 0;
//	}
//	else {
//		io2->top = 0;
//		io2->tail = size;
//		if(size == 0) {
//			io2->_close(_ctx, io2);
//		}
//		return 1;
//	}
//}
//
//static kbool_t io2_unblockread(CTX, kio_t *io2)
//{
//	int fd = io2->fd;
//	fd_set fds;
//	FD_ZERO(&fds);
//	FD_SET(fd, &fds);
//	int rc = select(fd + 1, &fds, NULL, NULL, NULL);
//	if(rc != -1) {
//		if(FD_ISSET((int)fd, &fds)) {
//			return io2->_blockread(_ctx, io2);
//		}
//		bzero(io2->buffer.buf, io2->buffer.max);
//		io2->top  = 0;
//		io2->tail = 0;
//		return 1;
//	}
//	{
//		KNH_NTRACE2(_ctx, "select", K_PERROR, KNH_LDATA(LOG_i("fd", fd), LOG_s("path", io2->DBG_NAME)));
//	}
//	return 0;
//}
//
//static size_t io2_blockwrite(CTX, kio_t *io2, const char *buf, size_t bufsiz)
//{
//	int fd = io2->fd;
//	ssize_t size = write(fd, buf, bufsiz);
//	if(size == -1) {
//		KNH_NTRACE2(_ctx, "write", K_PERROR, KNH_LDATA(LOG_i("fd", fd), LOG_s("path", io2->DBG_NAME)));
//		io2->_close(_ctx, io2);
//	}
//	return 0;
//}
//
//static size_t io2_unblockwrite(CTX, kio_t *io2, const char *buf, size_t size)
//{
//	int fd = io2->fd;
//	fd_set fds;
//	FD_ZERO(&fds);
//	FD_SET(fd, &fds);
//	int rc = select(fd + 1, NULL, &fds, NULL, NULL);
//	if(rc != -1) {
//		if(FD_ISSET((int)fd,&fds)) {
//			return io2->_blockwrite(_ctx, io2, buf, size);
//		}
//		return 0;
//	}
//	{
//		KNH_NTRACE2(_ctx, "select", K_PERROR, KNH_LDATA(LOG_i("fd", fd), LOG_s("path", io2->DBG_NAME)));
//	}
//	return 0;
//}
//
//static void io2_closeFD(CTX, kio_t *io2)
//{
//	DBG_ASSERT(io2->isRunning == 1);
//	close(io2->fd);
//	io2->isRunning = 0;
//}
//
//static void io2_closeFD_stdio(CTX, kio_t *io2)
//{
//	DBG_ASSERT(io2->isRunning == 1);
//	io2->isRunning = 0;
//}
//
//static kio_t* new_io2_(CTX, int fd, size_t bufsiz, void (*_close)(CTX, struct kio_t *))
//{
//	kio_t *io2 = KCALLOC(sizeof(kio_t));
//	io2->handler  = NULL;
//	io2->handler2 = NULL;
//	io2->fd = fd;
//	io2->isRunning = 1;
//	if(bufsiz > 0) {
//		KARRAY_INIT(io2->buffer, K_PAGESIZE, char);
//	}
//	io2->top  = 0;
//	io2->tail = 0;
//	io2->_close         = _close;
//	io2->_blockread     = io2_blockread;
//	io2->_unblockread   = io2_unblockread;
//	io2->_read          = io2_blockread;
//	io2->_blockwrite    = io2_blockwrite;
//	io2->_unblockwrite  = io2_unblockwrite;
//	io2->_write         = io2_blockwrite;
//	return io2;
//}
//
//static kio_t* new_io2(CTX, int fd, size_t bufsiz)
//{
//	return new_io2_(_ctx, fd, bufsiz, io2_closeFD);
//}
//
//static kio_t* new_io2_stdio(CTX, int fd, size_t bufsiz)
//{
//	return new_io2_(_ctx, fd, bufsiz, io2_closeFD_stdio);
//}
//
//static kio_t* new_io2ReadBuffer(CTX, const char *buf, size_t bufsiz)
//{
//	kio_t *io2 = KCALLOC(sizeof(kio_t));
//	io2->handler  = NULL;
//	io2->handler2 = NULL;
//	io2->fd = -1;
//	io2->isRunning = 0;
//	KARRAY_INIT(io2->buffer, bufsiz, char);
//	memcpy(io2->buffer.body, buf, bufsiz);
//	io2->buffer.size = bufsiz;
//	io2->top  = 0;
//	io2->tail = bufsiz;
//	io2->_close         = io2_close;
//	io2->_blockread     = io2_readNOP;
//	io2->_unblockread   = io2_readNOP;
//	io2->_read          = io2_readNOP;
//	io2->_blockwrite    = io2_writeNOP;
//	io2->_unblockwrite  = io2_writeNOP;
//	io2->_write         = io2_writeNOP;
//	return io2;
//}
//
//static void io2_closeBytes(CTX, kio_t *io2)
//{
//	io2->_blockwrite    = io2_writeNOP;
//	io2->_unblockwrite  = io2_writeNOP;
//	io2->_write         = io2_writeNOP;
//	io2->isRunning = 0;
//}
//
static kio_t *io2_null(void)
{
	static kio_t io2_dummy = {
			{0}, NULL
	};
	io2_dummy._close = io2_closeNOP;
	io2_dummy._read  = io2_readNOP;
	io2_dummy._write = io2_writeNOP;
	return &io2_dummy;
}

static void io2_free(CTX, kio_t *io2)
{
	if(io2->isRunning == 1) {
		io2->_close(_ctx, io2);
	}
	if(io2->buffer.max > 0) {
		KARRAY_FREE(io2->buffer, char);
		io2->top = 0;
		io2->tail = 0;
	}
	if(io2 != io2_null()) {
		KFREE(io2, sizeof(kio_t));
	}
}

static kbool_t io2_isClosed(CTX, kio_t *io2)
{
	return (io2->isRunning == 0 && io2->top >= io2->tail);
}

static int io2_getc(CTX, kio_t *io2)
{
	int ch = EOF;
	if(io2->top < io2->tail) {
		ch = io2->buffer.buf[io2->top];
		io2->top += 1;
	}
	else if(io2->isRunning) {
		io2->_read(_ctx, io2);
		if(io2->top < io2->tail) {
			ch = io2->buffer.buf[io2->top];
			io2->top += 1;
		}
	}
	return ch;
}

size_t io2_read(CTX, kio_t *io2, char *buf, size_t bufsiz)
{
	size_t rsize = 0;
	while(bufsiz > 0) {
		long remsiz = io2->tail - io2->top;
		if(remsiz > 0) {
			if(remsiz <= bufsiz) {
				memcpy(buf, io2->buffer.buf + io2->top, bufsiz);
				io2->top += bufsiz;
				rsize += bufsiz;
				return rsize;
			}
			else {
				memcpy(buf, io2->buffer.buf + io2->top, remsiz);
				buf    += remsiz;
				rsize  += remsiz;
				bufsiz -= remsiz;
			}
		}
		if(!io2->isRunning) break;
		io2->_read(_ctx, io2);
	}
	return rsize;
}

static kString *kwb_newLine(CTX, kwb_t *wb)
{
	if(kwb_bytesize(wb) > 0) {
		if(wb->w->buf[wb->w->size - 1] == '\r') {
			wb->w->buf[wb->w->size - 1] = 0;
			wb->w->size -= 1;
			if(kwb_bytesize(wb) == 0) return TS_EMPTY;
		}
		return kwb_newString(_ctx, wb, 0/*SPOL_POOLNEVER*/);
	}
	return TS_EMPTY;
}

kString* io2_readLine(CTX, kio_t *io2)
{
	kwb_t wb;
	kwb_init(&_ctx->stack->cwb, &wb);
	while(io2->isRunning) {
		size_t i, start, hasUTF8 = 0;
		if(!(io2->top < io2->tail)) {
			io2->_read(_ctx, io2);
		}
		start = io2->top;
		for(i = io2->top; i < io2->tail; i++) {
			int ch = ((unsigned char*)io2->buffer.buf)[i];
			if(ch == '\n') {
				kwb_write(&wb, (const char*)io2->buffer.buf + start, i - start);
				io2->top = i + 1;
				return kwb_newLine(_ctx, &wb);
			}
			if(ch > 127) hasUTF8 = 1;
		}
		kwb_write(&wb, (const char*)io2->buffer.buf + start, io2->tail - start);
		io2->top = i;
	}
	if(io2->top < io2->tail) {
		kwb_write(&wb, (const char*)io2->buffer.buf + io2->top, io2->tail - io2->top);
		io2->top  = 0;
		io2->tail = 0;
		return kwb_newLine(_ctx, &wb);
	}
	return (kString*) K_NULL/* TODO KNH_TNULL(String)*/;
}

//void io2_readAll(CTX, kio_t *io2, kBytes *ba)
//{
//	while(io2->isRunning == 1) {
//		if(!(io2->top < io2->tail)) {
//			io2->_read(_ctx, io2);
//		}
//		if(io2->tail > io2->top) {
//			knh_Bytes_write2(_ctx, ba, (const char*)io2->buffer + io2->top, io2->tail - io2->top);
//			io2->top  = 0;
//			io2->tail = 0;
//		}
//	}
//}
//
size_t io2_write(CTX, kio_t *io2, const char *buf, size_t bufsiz)
{
	if(io2->buffer.max > 0) {
		if(io2->tail + bufsiz < io2->buffer.max) {
			memcpy(io2->buffer.buf + io2->tail, buf, bufsiz);
			io2->tail += bufsiz;
			return bufsiz;
		}
		io2->_write(_ctx, io2, io2->buffer.buf, io2->tail);
		io2->tail = 0;
		if(bufsiz < io2->buffer.max) {
			memcpy(io2->buffer.buf, buf, bufsiz);
			io2->tail += bufsiz;
			return bufsiz;
		}
	}
	return io2->_write(_ctx, io2, buf, bufsiz);
}

size_t io2_writeMultiByteChar(CTX, kio_t *io2, const char *buf, size_t bufsiz)
{
	KNH_TODO("enc");
	return io2->_write(_ctx, io2, buf, bufsiz);
}

static kio_t* FILE_openNULL(CTX, kString *path, const char *mode, kDictMap *conf)
{
	FILE *fp = fopen(S_text(path), mode);
	if(fp != NULL) return new_FILE(_ctx, fp, 4096);
	return NULL;
}
/* ------------------------------------------------------------------------ */
/* [knh_write] */
#ifndef K_OSLINEFEED
#define K_OSLINEFEED "\n"
#endif

static void knh_write_EOL(CTX, kOutputStream *w)
{
	io2_write(_ctx, w->io2, K_OSLINEFEED, sizeof(K_OSLINEFEED) - 1);
	if(OutputStream_isAutoFlush(w)) {
		io2_flush(_ctx, w->io2);
	}
	//OutputStream_setBOL(w, 1);
}

//static void knh_write_TAB(CTX, kOutputStream *w)
//{
//	io2_write(_ctx, w->io2, "\t", 1);
//}
//
/* ------------------------------------------------------------------------ */

static kInputStream *new_InputStream(CTX, kio_t *io2, kString *path)
{
	kInputStream* in = new_(InputStream, io2);
	if(path != NULL) {
		KSETv(in->path, path);
		io2->DBG_NAME = S_text(path);
		if(io2 == NULL) {
			io2 = FILE_openNULL(_ctx, path, "r", NULL);
			if(io2 == NULL) {
				io2 = io2_null();
				kObject_setNullObject(in, 1);
			}
			in->io2 = io2;
		}
	}
	return in;
}

static kOutputStream *new_OutputStream(CTX,  kio_t *io2, kString *path)
{
	kOutputStream* w = new_(OutputStream, io2);
	if(path != NULL) {
		KSETv(w->path, path);
		io2->DBG_NAME = S_text(path);
		if(io2 == NULL) {
			io2 = FILE_openNULL(_ctx, path, "a", NULL);
			if(io2 == NULL) {
				io2 = io2_null();
				kObject_setNullObject(w, 1);
			}
			w->io2 = io2;
		}
	}
	return w;
}

/* ------------------------------------------------------------------------ */
static void knh_OutputStream_flush(CTX, kOutputStream *w)
{
	io2_flush(_ctx, w->io2);
}

static void knh_OutputStream_putc(CTX, kOutputStream *w, int ch)
{
	char buf[8] = {ch};
	io2_write(_ctx, w->io2, buf, 1);
}

//static void knh_OutputStream_write(CTX, kOutputStream *w, kbytes_t buf)
//{
//	io2_write(_ctx, w->io2, buf.text, buf.len);
//}
//
//static void knh_OutputStream_p(CTX, kOutputStream *w, kbytes_t buf)
//{
//	if(w->encNULL != NULL) {
//		size_t i;
//		for(i = 0; i < buf.len; i++) {
//			int ch = buf.ubuf[i];
//			if(ch > 127) {
//				io2_writeMultiByteChar(_ctx, w->io2, buf.text, buf.len);
//				return;
//			}
//		}
//	}
//	io2_write(_ctx, w->io2, buf.text, buf.len);
//}
//
/* ------------------------------------------------------------------------ */
//## method @public Int InputStream.getByte()
static KMETHOD InputStream_getByte(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(io2_getc(_ctx, (sfp[0].in)->io2));
}

/* ------------------------------------------------------------------------ */
//## method @public boolean InputStream.isClosed()
static KMETHOD InputStream_isClosed(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(io2_isClosed(_ctx, (sfp[0].in)->io2));
}

/* ------------------------------------------------------------------------ */
//## method @public void OutputStream.putByte(int ch)
static KMETHOD OutputStream_putByte(CTX, ksfp_t *sfp _RIX)
{
	kOutputStream *w = sfp[0].w;
	knh_OutputStream_putc(_ctx, w, (int)(sfp[1].ivalue));
	RETURNvoid_();
}

/* ------------------------------------------------------------------------ */
//## method @public boolean OutputStream.isClosed()
static KMETHOD OutputStream_isClosed(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(io2_isClosed(_ctx, (sfp[0].in)->io2));
}
/* ------------------------------------------------------------------------ */
//## method @Throwable InputStream InputStream.new(String urn, String mode);

static KMETHOD InputStream_new(CTX, ksfp_t *sfp _RIX)
{
	//kInputStream *in = sfp[0].in;
	kString *pth = sfp[1].s;
	const char *mode = IS_NULL(sfp[2].s) ? "r" : S_text(sfp[2].s);
	//KSETv(in->path, pth);
	kio_t *io2 = FILE_openNULL(_ctx, pth, mode, NULL);
	if(io2 != NULL) {
		RETURN_(new_InputStream(_ctx, io2, pth));
	}
	RETURN_(K_NULL/*TODO in*/);
}

/* ------------------------------------------------------------------------ */
//## method void InputStream.close();

static KMETHOD InputStream_close(CTX, ksfp_t *sfp _RIX)
{
	io2_close(_ctx, (sfp[0].in)->io2);
}

/* ------------------------------------------------------------------------ */
//## method @Iterative String InputStream.readLine();

static KMETHOD InputStream_readLine(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(io2_readLine(_ctx, (sfp[0].in)->io2));
}
/* ------------------------------------------------------------------------ */
/* [OutputStream] */

//## method @Throwable OutputStream OutputStream.new(String path, String mode);
static KMETHOD OutputStream_new(CTX, ksfp_t *sfp _RIX)
{
	//kOutputStream *w = sfp[0].w;
	kString *pth = sfp[1].s;
	const char *mode = IS_NULL(sfp[2].s) ? "w" : S_text(sfp[2].s);
	//KSETv(w->path, pth);
	kio_t *io2 = FILE_openNULL(_ctx, pth, mode, NULL);
	if(io2 != NULL) {
		RETURN_(new_OutputStream(_ctx, io2, pth));
	}
	RETURN_(K_NULL/*TODO w*/);
}

/* ------------------------------------------------------------------------ */
//## method void OutputStream.print(Object value, ...);

static KMETHOD OutputStream_print(CTX, ksfp_t *sfp _RIX)
{
	kOutputStream *w = sfp[0].w;
	ksfp_t *v = sfp + 1;
	size_t i, ac = knh_stack_argc(_ctx, v);
	kwb_t wb;
	kwb_init(&_ctx->stack->cwb, &wb);
	for(i = 0; i < ac; i++) {
		O_ct(v[i].o)->p(_ctx, v, i, &wb, 0);
		io2_write(_ctx, w->io2, kwb_top(&wb, 1), kwb_bytesize(&wb));
	}
	RETURNvoid_();
}

/* ------------------------------------------------------------------------ */
//## method void OutputStream.println(Object value, ...);

static KMETHOD OutputStream_println(CTX, ksfp_t *sfp _RIX)
{
	kOutputStream *w = sfp[0].w;
	ksfp_t *v = sfp + 1;
	size_t i, ac = knh_stack_argc(_ctx, v);
	kwb_t wb;
	kwb_init(&_ctx->stack->cwb, &wb);
	for(i = 0; i < ac; i++) {
		O_ct(v[i].o)->p(_ctx, v, i, &wb, 0);
		io2_write(_ctx, w->io2, kwb_top(&wb, 1), kwb_bytesize(&wb));
	}
	knh_write_EOL(_ctx, w);
	RETURNvoid_();
}

/* ------------------------------------------------------------------------ */
//## method void OutputStream.flush();

static KMETHOD OutputStream_flush(CTX, ksfp_t *sfp _RIX)
{
	knh_OutputStream_flush(_ctx, sfp[0].w);
	RETURNvoid_();
}

/* ------------------------------------------------------------------------ */
//## method void OutputStream.close();

static KMETHOD OutputStream_close(CTX, ksfp_t *sfp _RIX)
{
	io2_close(_ctx, sfp[0].w->io2);
	RETURNvoid_();
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t io_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	kioshare_t *base = (kioshare_t*)KCALLOC(sizeof(kioshare_t));
	base->h.name     = "io";
	base->h.setup    = kioshare_setup;
	base->h.reftrace = kioshare_reftrace;
	base->h.free     = kioshare_free;
	Konoha_setModule(MOD_IO, &base->h, pline);
	base->cInputStream  = Konoha_addClassDef(NULL, &InputStreamDef, pline);
	base->cOutputStream = Konoha_addClassDef(NULL, &OutputStreamDef, pline);

	int FN_x = FN_("x");
	int FN_path = FN_("path");
	int FN_mode = FN_("mode");
	int FN_value = FN_("value");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(InputStream_getByte),  TY_Int,         TY_InputStream,  MN_("getByte"), 0,
		_Public, _F(InputStream_isClosed), TY_Boolean,     TY_InputStream,  MN_("isClosed"), 0,
		_Public, _F(InputStream_new),      TY_InputStream, TY_InputStream,  MN_("new"), 2, TY_String, FN_path, TY_String, FN_mode,
		_Public, _F(InputStream_close),    TY_void,        TY_InputStream,  MN_("close"), 0,
		_Public, _F(InputStream_readLine), TY_String,      TY_InputStream,  MN_("readLine"), 0,
		_Public, _F(OutputStream_putByte),  TY_void,         TY_OutputStream, MN_("putByte"), 1, TY_Int, FN_x,
		_Public, _F(OutputStream_isClosed), TY_Boolean,      TY_OutputStream, MN_("isClosed"), 0,
		_Public, _F(OutputStream_new),      TY_OutputStream, TY_OutputStream, MN_("new"), 2, TY_String, FN_path, TY_String, FN_mode,
		_Public, _F(OutputStream_print),    TY_void,         TY_OutputStream, MN_("print"), 1, TY_Object, FN_value,
		_Public, _F(OutputStream_println),  TY_void,         TY_OutputStream, MN_("println"), 1, TY_Object, FN_value,
		_Public, _F(OutputStream_flush),    TY_void,         TY_OutputStream, MN_("flush"), 0,
		_Public, _F(OutputStream_close),     TY_void,         TY_OutputStream, MN_("close"), 0,
		DEND,
	};
	kNameSpace_loadMethodData(NULL, MethodData);
	return true;
}

static kbool_t io_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t io_initNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t io_setupLingo(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* io_init(void)
{
	static const KDEFINE_PACKAGE d = {
		KPACKNAME("io", "1.0"),
		.initPackage  = io_initPackage,
		.setupPackage = io_setupPackage,
		.initNameSpace = io_initNameSpace,
		.setupPackage = io_setupLingo,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
#endif
