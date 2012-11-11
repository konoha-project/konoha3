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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
//#include "fdconfig.h"

//#ifndef PATHMAX
//#define PATHMAX 256
//#endif /*PATHMAX*/

#ifdef __cplusplus
extern "C"{
#endif
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/* FILE low-level*/

// TODO: functions below will return integer which indecates file descriptor

//## @Native int System.lseek(int fd, int offset, int whence)
static KMETHOD System_lseek(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int offset = sfp[2].intValue;
	int whence = sfp[3].intValue;
	off_t ret_offset = lseek(fd, offset, whence);
	if(ret_offset == -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "lseek",
						LogUint("offset", offset),
						LogUint("whence", whence)
			);
	}
	KReturnUnboxValue((int)ret_offset);
}

//## boolean System.ftruncate(int fd, int length)
static KMETHOD System_ftruncate(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int length = sfp[2].intValue;
	int ret = ftruncate(fd, length);
	if(ret != 0) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "ftruncate",
						LogUint("fd", fd),
						LogUint("length", length)
			);
	}
	KReturnUnboxValue(ret == 0);
}

//## boolean System.fchmod(int fd, int length)
static KMETHOD System_fchmod(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int mode = sfp[2].intValue;
	int ret = fchmod(fd, mode);
	if(ret != -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "fchmod",
						LogUint("fd", fd),
						LogUint("mode", mode)
			);
	}
	KReturnUnboxValue(ret == 0);
}

// TODO: isn't ioctl difficult for script users? should we support this?
//## @Native int File.ioctl(int request, String[] args)
//staic KMETHOD File_ioctl(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kFile *file = (kFile *)sfp[0].asObject;
//	FILE *fp = file->fp;
//	int request  = int_to(int, sfp[1]);
//	char *argp = String_to(char*, sfp[2]);
//	if(fp == NULL) KReturnUnboxValue(0);
//	int fd = fileno(fp);
//	if(fd == -1) {
//		KReturnUnboxValue(0);
//	}
//	int ret = ioctl(fd, request, argp);
//	KNH_NTRACE2(ctx, "ioctl", ret != -1 ? K_OK : K_PERROR, KNH_LDATA(
//				));
//	KReturnUnboxValue(ret != -1);
//}

// NOTE: sys_flock can use for a file, only for
//## @Native boolean System.flock(int fd, int opretaion);
static KMETHOD System_flock(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int operation = sfp[2].intValue;
	int ret = flock(fd, operation);
	if(ret == -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "flock",
						LogUint("fd", fd),
						LogUint("operation", operation)
			);
	}
	KReturnUnboxValue(ret == 0);
}

//## @Native boolean System.sync(int fd);
static KMETHOD System_sync(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int ret =  fsync(fd);
	if(ret == -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "sync", LogUint("fd", fd));
	}
	KReturnUnboxValue(ret == 0);
}

static KMETHOD System_fchown(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	uid_t owner = sfp[2].intValue;
	gid_t group = sfp[3].intValue;
	int ret = fchown(fd, owner, group);
	if(ret == -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "fchown",
						LogUint("fd", fd),
						LogUint("owner", owner),
						LogUint("group", group)
			);
	}
	KReturnUnboxValue(ret == 0);
}

static KMETHOD System_fsync(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int ret = fsync(fd);
	if(ret == -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "fsync", LogUint("fd", fd));
	}
	KReturnUnboxValue(ret == 0);
}


//## int System.getdtablesize()
static KMETHOD System_getdtablesize(KonohaContext *kctx, KonohaStack *sfp)
{
	int ret = getdtablesize();
	KReturnUnboxValue(ret);
}

//## int System.open(String pathname, int flags)
static KMETHOD System_open(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *pathname = S_text(s);
	int flags = sfp[2].intValue;
	int ret = open(pathname, flags);
	if(ret == -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(s)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "open",
						LogText("pathname", pathname),
						LogUint("flags", flags)
			);
	}
	KReturnUnboxValue(ret);
}

//## int System.open(String pathname, int flags, int mode)
static KMETHOD System_open_mode(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *pathname = S_text(s);
	int flags = sfp[2].intValue;
	mode_t mode = sfp[3].intValue;
	int ret = open(pathname, flags, mode);
	if(ret == -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(s)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "open_mode",
						LogText("pathname", pathname),
						LogUint("flags", flags),
						LogUint("mode", mode)
			);
	}
	KReturnUnboxValue(ret);
}

//## int System.fchdir(int fd)
static KMETHOD System_fchdir(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = fchdir(sfp[1].intValue);
	if(fd == -1) {
		// TODO: throw
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "fchdir", LogUint("fd", fd));
	}
	KReturnUnboxValue(fd == 0);
}

//## boolean System.isatty(int fd);
static KMETHOD System_isatty(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	KReturnUnboxValue(isatty(fd) == 1);
}

//## String System.ttyname(int fd);
static KMETHOD System_ttyname(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	char buf[K_PAGESIZE];
	int ret = ttyname_r(fd, buf, sizeof(buf));
	if(ret != 0) {
		KMakeTrace(trace, sfp);
		int fault = KLIB DiagnosisFaultType(kctx, SystemError, trace);
		KTraceErrorPoint(trace, fault, "ttyname", LogUint("fd", fd), LogErrno);
	}
	KReturn(KLIB new_kString(kctx, OnStack, buf, strlen(buf), 0));
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Static   kMethod_Static
//#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

//#define CT_Stat         cStat
//#define TY_Stat         cStat->typeId
//#define CT_DIR          cDIR
//#define TY_DIR          cDIR->typeId
//#define CT_Dirent       cDirent
//#define TY_Dirent       cDirent->typeId

static kbool_t fd_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(System_lseek),     TY_int,     TY_System, MN_("lseek"),     3, TY_int, FN_("fd"), TY_int, FN_("offset"), TY_int, FN_("whence"),
		_Public|_Static, _F(System_ftruncate), TY_boolean, TY_System, MN_("ftruncate"), 2, TY_int, FN_("fd"), TY_int, FN_("length"),
		_Public|_Static, _F(System_fchmod),    TY_boolean, TY_System, MN_("fchmod"),    2, TY_int, FN_("fd"), TY_int, FN_("length"),
		_Public|_Static, _F(System_flock),     TY_boolean, TY_System, MN_("flock"),     2, TY_int, FN_("fd"), TY_int, FN_("operation"),
		_Public|_Static, _F(System_sync),      TY_boolean, TY_System, MN_("sync"),      1, TY_int, FN_("fd"),
		_Public|_Static, _F(System_fchown),    TY_boolean, TY_System, MN_("fchown"),    3, TY_int, FN_("pd"), TY_int, FN_("owner"),  TY_int, FN_("group"),
		_Public|_Static, _F(System_fsync),     TY_boolean, TY_System, MN_("fsync"),     1, TY_int, FN_("fd"),
		_Public|_Static|_Const|_Im, _F(System_getdtablesize), TY_int, TY_System, MN_("getdtablesize"), 0,
		_Public|_Static|_Im, _F(System_open),      TY_int,     TY_System, MN_("open"),   2, TY_String, FN_("pathname"), TY_int, FN_("flags"),
		_Public|_Static|_Im, _F(System_open_mode), TY_int,     TY_System, MN_("open"),   3, TY_String, FN_("pathname"), TY_int, FN_("flags"), TY_int, FN_("mode"),
		_Public|_Static|_Im, _F(System_fchdir),    TY_boolean, TY_System, MN_("fchdir"), 1, TY_int,    FN_("fd"),
		_Public|_Static|_Im, _F(System_isatty),    TY_boolean, TY_System, MN_("isatty"), 1, TY_int,    FN_("fd"),
		_Public|_Static|_Im, _F(System_ttyname),   TY_String, TY_System, MN_("ttyname"), 1, TY_int,    FN_("fd"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	KDEFINE_INT_CONST intData[] = {
		/*for System.lseek*/
		{KDefineConstInt(SEEK_SET)},
		{KDefineConstInt(SEEK_CUR)},
		{KDefineConstInt(SEEK_END)},
		/*for System.flock*/
		{KDefineConstInt(LOCK_SH)},
		{KDefineConstInt(LOCK_EX)},
		{KDefineConstInt(LOCK_UN)},
		{KDefineConstInt(LOCK_NB)},
		/*for System.open*/
		{KDefineConstInt(O_RDONLY)},
		{KDefineConstInt(O_WRONLY)},
		{KDefineConstInt(O_RDWR)},
		{KDefineConstInt(O_CREAT)},
		{KDefineConstInt(O_EXCL)},
		{KDefineConstInt(O_TRUNC)},
		{KDefineConstInt(O_APPEND)},
		{KDefineConstInt(O_NONBLOCK)},
		{KDefineConstInt(O_NDELAY)},
		{KDefineConstInt(O_NOCTTY)},
		{NULL}, /* sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(intData), 0);
	return true;
}

static kbool_t fd_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* fd_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("posix", "1.0"),
		.PackupNameSpace    = fd_PackupNameSpace,
		.ExportNameSpace   = fd_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
