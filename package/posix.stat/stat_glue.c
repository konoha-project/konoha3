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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifndef K_PATHMAX
#define K_PATHMAX 1024
#endif

#define LogFileName(S)     LogText("filename", S)
#define LogFileName2(S)    LogText("filename2", S)  // adhoc..
#define LogMode(mode)      LogUint("mode", mode)
#define LogOwner(o)        LogUint("owner", o)
#define LogGroup(o)        LogUint("group", o)

/* ------------------------------------------------------------------------ */
/* stat */

typedef struct kFileStatusVar kFileStatus;
struct kFileStatusVar {
	KonohaObjectHeader h;
	struct stat *stat;
};

static void kFileStatus_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kFileStatus *stat = (kFileStatus *)o;
	if(conf != NULL) {
		stat->stat = (struct stat *)PLATAPI malloc_i(sizeof(struct stat));
		memcpy(stat->stat, conf, sizeof(struct stat));
	}
	else {
		stat->stat = NULL;
	}
}

static void kFileStatus_free(KonohaContext *kctx, kObject *o)
{
	kFileStatus *stat = (kFileStatus *)o;
	if(stat->stat != NULL) {
		PLATAPI free_i(stat->stat);
		stat->stat = NULL;
	}
}

//struct stat { /* when _DARWIN_FEATURE_64_BIT_INODE is NOT defined */
//	dev_t    st_dev;    /* device inode resides on */
//	ino_t    st_ino;    /* inode's number */
//	mode_t   st_mode;   /* inode protection mode */
//	nlink_t  st_nlink;  /* number of hard links to the file */
//	uid_t    st_uid;    /* user-id of owner */
//	gid_t    st_gid;    /* group-id of owner */
//	dev_t    st_rdev;   /* device type, for special file inode */
//	struct timespec st_atimespec;  /* time of last access */
//	struct timespec st_mtimespec;  /* time of last data modification */
//	struct timespec st_ctimespec;  /* time of last file status change */
//	off_t    st_size;   /* file size, in bytes */
//	quad_t   st_blocks; /* blocks allocated for file */
//	u_long   st_blksize;/* optimal file sys I/O ops blocksize */
//	u_long   st_flags;  /* user defined flags for file */
//	u_long   st_gen;    /* file generation number */
//};

static void kFileStatus_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	kFileStatus *stat = (kFileStatus *)v[pos].asObject;
	KLIB Kwb_printf(kctx, wb, "{dev: %d, ", stat->stat->st_dev);
	KLIB Kwb_printf(kctx, wb, "ino: %d, ",  stat->stat->st_ino);
	KLIB Kwb_printf(kctx, wb, "mode: %d, ", stat->stat->st_mode);
	KLIB Kwb_printf(kctx, wb, "nlink: %d, ", stat->stat->st_nlink);
	KLIB Kwb_printf(kctx, wb, "uid: %d, ", stat->stat->st_uid);
	KLIB Kwb_printf(kctx, wb, "uid: %d, ", stat->stat->st_gid);
	KLIB Kwb_printf(kctx, wb, "size: %d}", stat->stat->st_size);
}

// --------------------------------------------------------------------------
/* stat */

//## Stat System.stat(String path)
static KMETHOD System_stat(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	struct stat buf = {}; /* zero */
	int ret = stat(systemPath, &buf);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "stat", LogText("path", S_text(path)), LogErrno);
	}
	KReturn(KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)&buf));
}

//## Stat System.lstat(String path)
static KMETHOD System_lstat(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	struct stat buf = {}; /* zero */
	int ret = lstat(systemPath, &buf);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "lstat", LogText("path", S_text(path)), LogErrno);
	}
	KReturn(KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)&buf));
}

//## Stat System.fstat(int fd)
static KMETHOD System_fstat(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	int fd = (int)sfp[1].intValue;
	struct stat buf = {}; /* zero */
	int ret = fstat(fd, &buf);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault|SoftwareFault, "fstat", LogInt("fildes", fd), LogErrno);
	}
	KReturn(KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)&buf));
}

//## int Stat.getdev()
static KMETHOD Stat_getdev(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_dev);
}

//## int Stat.getino()
static KMETHOD Stat_getino(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_ino);
}

//## int Stat.getmode()
static KMETHOD Stat_getmode(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_mode);
}

//## int Stat.getnlink()
static KMETHOD Stat_getnlink(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_nlink);
}

//## int Stat.getuid()
static KMETHOD Stat_getuid(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_uid);
}

//## int Stat.getgid()
static KMETHOD Stat_getgid(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_gid);
}

//## int Stat.getsize()
static KMETHOD Stat_getsize(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_size);
}

//## int Stat.getatime()
static KMETHOD Stat_getatime(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_atime);
}

//## int Stat.getmtime()
static KMETHOD Stat_getmtime(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_mtime);
}

//## int Stat.getctime()
static KMETHOD Stat_getctime(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_ctime);
}

#ifdef HAVE_STRUCT_STAT_ST_RDEV
//## int Stat.getrdev()
static KMETHOD Stat_getrdev(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_rdev);
}
#endif /* HAVE_STRUCT_STAT_ST_RDEV */

#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
//## int Stat.getblocks()
static KMETHOD Stat_getblocks(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_blocks);
}
#endif /* HAVE_STRUCT_STAT_ST_BLOCKS */

#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
//## int Stat.getblksize()
static KMETHOD Stat_getblksize(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_blksize);
}
#endif /* HAVE_STRUCT_STAT_ST_BLKSIZE */

#ifdef HAVE_STRUCT_STAT_ST_FLAGS
//## int Stat.getflags()
static KMETHOD Stat_getflags(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_flags);
}
#endif /* HAVE_STRUCT_STAT_ST_FLAGS */

#ifdef HAVE_STRUCT_STAT_ST_GEN
//## int Stat.getgen()
static KMETHOD Stat_getgen(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_gen);
}
#endif /* HAVE_STRUCT_STAT_ST_GEN */

#ifdef HAVE_STRUCT_STAT_ST_BIRTHTIME
//## int Stat.getbirthtime()
static KMETHOD Stat_getbirthtime(KonohaContext *kctx, KonohaStack *sfp)
{
	kFileStatus *stat = (kFileStatus *)sfp[0].asObject;
	KReturnUnboxValue(stat->stat->st_birthtime);
}
#endif /* HAVE_STRUCT_STAT_ST_BIRTHTIME */


#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _C        kMethod_CCompatible
#define _Im       kMethod_Immutable
#define _Const    kMethod_Const
#define _F(F)   (intptr_t)(F)
#define _Iter     kMethod_Iterative

static void stat_defineClassAndMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_CLASS defStat = {};
	defStat.structname = "FileStatus";
	defStat.typeId = TY_newid;
	defStat.cstruct_size = sizeof(struct kFileStatusVar);
	defStat.cflag = kClass_Final;
	defStat.init  = kFileStatus_init;
	defStat.free  = kFileStatus_free;
	defStat.p     = kFileStatus_p;
	KonohaClass *cStat = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defStat, trace);
	int TY_Stat = cStat->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(System_stat),  TY_Stat, TY_System, MN_("stat"), 1, TY_String, FN_("path"),
		_Public|_Static, _F(System_lstat), TY_Stat, TY_System, MN_("lstat"), 1, TY_String, FN_("path"),
		_Public|_Static, _F(System_fstat), TY_Stat, TY_System, MN_("fstat"), 1, TY_int, FN_("fd"),
		_Public|_Const|_Im, _F(Stat_getdev), TY_int, TY_Stat, MN_("getdev"), 0,
		_Public|_Const|_Im, _F(Stat_getino), TY_int, TY_Stat, MN_("getino"), 0,
		_Public|_Const|_Im, _F(Stat_getmode), TY_int, TY_Stat, MN_("getmode"), 0,
		_Public|_Const|_Im, _F(Stat_getnlink), TY_int, TY_Stat, MN_("getnlink"), 0,
		_Public|_Const|_Im, _F(Stat_getuid), TY_int, TY_Stat, MN_("getuid"), 0,
		_Public|_Const|_Im, _F(Stat_getgid), TY_int, TY_Stat, MN_("getgid"), 0,
		_Public|_Const|_Im, _F(Stat_getsize), TY_int, TY_Stat, MN_("getsize"), 0,
		_Public|_Const|_Im, _F(Stat_getatime), TY_int, TY_Stat, MN_("getatime"), 0,
		_Public|_Const|_Im, _F(Stat_getmtime), TY_int, TY_Stat, MN_("getmtime"), 0,
		_Public|_Const|_Im, _F(Stat_getctime), TY_int, TY_Stat, MN_("getctime"), 0,
#ifdef HAVE_STRUCT_STAT_ST_RDEV
		_Public|_Const|_Im, _F(Stat_getrdev), TY_int, TY_Stat, MN_("getrdev"), 0,
#endif /* HAVE_STRUCT_STAT_ST_RDEV */
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
		_Public|_Const|_Im, _F(Stat_getblocks), TY_int, TY_Stat, MN_("getblocks"), 0,
#endif /* HAVE_STRUCT_STAT_ST_BLOCKS */
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
		_Public|_Const|_Im, _F(Stat_getblksize), TY_int, TY_Stat, MN_("getblksize"), 0,
#endif /* HAVE_STRUCT_STAT_ST_BLKSIZE */
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
		_Public|_Const|_Im, _F(Stat_getflags), TY_int, TY_Stat, MN_("getflags"), 0,
#endif /* HAVE_STRUCT_STAT_ST_FLAGS */
#ifdef HAVE_STRUCT_STAT_ST_GEN
		_Public|_Const|_Im, _F(Stat_getgen), TY_int, TY_Stat, MN_("getgen"), 0,
#endif /* HAVE_STRUCT_STAT_ST_GEN */
#ifdef HAVE_STRUCT_STAT_ST_BIRTHTIME
		_Public|_Const|_Im, _F(Stat_getbirthtime), TY_int, TY_Stat, MN_("getbirthtime"), 0,
#endif /* HAVE_STRUCT_STAT_ST_BIRTHTIME */

		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}


// --------------------------------------------------------------------------

static kbool_t stat_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	//	KRequireKonohaCommonModule(trace);
	stat_defineClassAndMethod(kctx, ns, trace);
	return true;
}

static kbool_t stat_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* stat_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("posix", "1.0"),
		.PackupNameSpace    = stat_PackupNameSpace,
		.ExportNameSpace   = stat_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
