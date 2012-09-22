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
#include <minikonoha/bytes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include "fdconfig.h"

#ifndef PATHMAX
#define PATHMAX 256
#endif /*PATHMAX*/

/* ------------------------------------------------------------------------ */

typedef const struct _kStat kStat;
struct _kStat {
	KonohaObjectHeader h;
	struct stat *stat;
};

typedef const struct _kDIR kDIR;
struct _kDIR {
	KonohaObjectHeader h;
	DIR *dirp;
};

typedef const struct _kDirent kDirent;
struct _kDirent {
	KonohaObjectHeader h;
	struct dirent *entry;
};

/* ------------------------------------------------------------------------ */

static void kStat_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kStat *stat = (struct _kStat*)o;
	if(conf != NULL) {
		stat->stat = (struct stat *)PLATAPI malloc_i(sizeof(struct stat));
		memcpy(stat->stat, conf, sizeof(struct stat));
	}
	else {
		stat->stat = NULL;
	}
}

static void kStat_free(KonohaContext *kctx, kObject *o)
{
	struct _kStat *stat = (struct _kStat *)o;
	if(stat->stat != NULL) {
		PLATAPI free_i(stat->stat);
		stat->stat = NULL;
	}
}

static void kStat_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
	KLIB Kwb_printf(kctx, wb, "Stat :%p", sfp[pos].asObject);
}

static void kDIR_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kDIR *dir = (struct _kDIR*)o;
	dir->dirp = conf;
}

static void kDIR_free(KonohaContext *kctx, kObject *o)
{
	struct _kDIR *dir = (struct _kDIR*)o;
	if(dir->dirp != NULL) {
		int ret = closedir(dir->dirp);
		if(ret == -1) {
			// TODO: throw
		}
		dir->dirp = NULL;
	}
}

static void kDIR_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
	kDIR *dir = (kDIR*)sfp[pos].asObject;
	DIR *dirp = dir->dirp;
	KLIB Kwb_printf(kctx, wb, "DIR :%p", dirp);
}

static void kDirent_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kDirent *dirent = (struct _kDirent*)o;
	if(conf != NULL) {
		dirent->entry = (struct dirent *)PLATAPI malloc_i(sizeof(struct dirent));
		memcpy(dirent->entry, conf, sizeof(struct dirent));
	}
	else {
		dirent->entry = NULL;
	}
}

static void kDirent_free(KonohaContext *kctx, kObject *o)
{
	struct _kDirent *dirent = (struct _kDirent*)o;
	if(dirent->entry != NULL) {
		PLATAPI free_i(dirent->entry);
		dirent->entry = NULL;
	}
}

static void kDirent_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
	struct _kDirent *dirent = (struct _kDirent *)sfp[pos].asObject;
	struct dirent *entry = dirent->entry;
	KLIB Kwb_printf(kctx, wb, "Dirent :%p", entry);
}

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
	if (ret_offset == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_DataFault,
			   LogText("@", "lseek"),
			   LogUint("offset", offset),
			   LogUint("whence", whence),
			   LogText("errstr", strerror(errno))
			);
	}
	RETURNi_((int)ret_offset);
}

//## @Native String System.getCwd()
static KMETHOD System_getCwd(KonohaContext *kctx, KonohaStack *sfp)
{
	char filepath[256] = {0};
	char *cwd = getcwd(filepath, 256);
	RETURN_(KLIB new_kString(kctx, cwd, strlen(cwd), 0));
}
//## boolean System.ftruncate(int fd, int length)
static KMETHOD System_ftruncate(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int length = sfp[2].intValue;
	int ret = ftruncate(fd, length);
	if (ret != 0) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "ftruncate"),
			   LogUint("length", length),
			   LogText("errstr", strerror(errno))
			);
	}
	RETURNb_(ret == 0);
}

//## boolean System.fchmod(int fd, int length)
static KMETHOD System_fchmod(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int mode = sfp[2].intValue;
	int ret = fchmod(fd, mode);
	if (ret != -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "fchmod"),
			   LogUint("mode", mode),
			   LogText("errstr", strerror(errno))
			);
	}
	RETURNb_(ret == 0);
}

// TODO: isn't ioctl difficult for script users? should we support this?
//## @Native int File.ioctl(int request, String[] args)
//staic KMETHOD File_ioctl(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kFile *file = (kFile*)sfp[0].asObject;
//	FILE *fp = file->fp;
//	int request  = int_to(int, sfp[1]);
//	char *argp = String_to(char*, sfp[2]);
//	if (fp == NULL) RETURNb_(0);
//	int fd = fileno(fp);
//	if (fd == -1) {
//		RETURNb_(0);
//	}
//	int ret = ioctl(fd, request, argp);
//	KNH_NTRACE2(ctx, "ioctl", ret != -1 ? K_OK : K_PERROR, KNH_LDATA(
//				));
//	RETURNb_(ret != -1);
//}

// NOTE: sys_flock can use for a file, only for
//## @Native boolean System.flock(int fd, int opretaion);
static KMETHOD System_flock(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int operation = sfp[2].intValue;
	int ret = flock(fd, operation);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "flock"),
			   LogUint("operation", operation),
			   LogText("errstr", strerror(errno))
			);
	}
	RETURNb_(ret == 0);
}

//## @Native boolean System.sync(int fd);
static KMETHOD System_sync(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int ret =  fsync(fd);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "fsync"),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNb_(ret == 0);
}

static KMETHOD System_link(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s1 = sfp[1].asString;
	kString *s2 = sfp[2].asString;
	const char *oldpath = S_text(s1);
	const char *newpath = S_text(s2);
	int ret = link(oldpath, newpath);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "link"),
			   LogText("oldpath", oldpath),
			   LogText("newpath", newpath),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_unlink(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *pathname = S_text(s);
	int ret = unlink(pathname);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "unlink"),
			   LogText("pathname", pathname),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_rename(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s1 = sfp[1].asString;
	kString *s2 = sfp[2].asString;
	const char *oldpath = S_text(s1);
	const char *newpath = S_text(s2);
	int ret = rename(oldpath, newpath);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "rename"),
			   LogText("oldpath", oldpath),
			   LogText("newpath", newpath),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_rmdir(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *pathname = S_text(s);
	int ret = rmdir(pathname);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "rmdir"),
			   LogText("pathname", pathname),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_symlink(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s1 = sfp[1].asString;
	kString *s2 = sfp[2].asString;
	const char *oldpath = S_text(s1);
	const char *newpath = S_text(s2);
	int ret = symlink(oldpath, newpath);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "symlink"),
			   LogText("oldpath", oldpath),
			   LogText("newpath", newpath),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_readlink(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s1 = sfp[1].asString;
	const char *pathname = S_text(s1);
	char pathbuf[PATHMAX];
	ssize_t ret = readlink(pathname, pathbuf, PATHMAX);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "readlink"),
			   LogText("pathname", pathname),
			   KeyValue_p("errstr", strerror(errno))
			);
		pathbuf[0] = '\0';
	}
	else {
		pathbuf[ret] = '\0';
	}
	RETURN_(KLIB new_kString(kctx, pathbuf, strlen(pathbuf), 0));
}

static KMETHOD System_chown(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *pathname = S_text(s);
	uid_t owner = sfp[2].intValue;
	gid_t group = sfp[3].intValue;
	int ret = chown(pathname, owner, group);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "chown"),
			   LogText("pathname", pathname),
			   LogUint("owner", owner),
			   LogUint("group", group),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_lchown(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *pathname = S_text(s);
	uid_t owner = sfp[2].intValue;
	gid_t group = sfp[3].intValue;
	int ret = lchown(pathname, owner, group);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "lchown"),
			   LogText("pathname", pathname),
			   LogUint("owner", owner),
			   LogUint("group", group),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_fchown(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	uid_t owner = sfp[2].intValue;
	gid_t group = sfp[3].intValue;
	int ret = fchown(fd, owner, group);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "fchown"),
			   LogUint("owner", owner),
			   LogUint("group", group),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_access(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *pathname = S_text(s);
	int mode = sfp[2].intValue;
	int ret = access(pathname, mode);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "access"),
			   LogText("pathname", pathname),
			   LogUint("mode", mode),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_fsync(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int ret = fsync(fd);
	if (ret == -1) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
			   LogText("@", "fsync"),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

//## Stat System.stat(String path)
static KMETHOD System_stat(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = S_text(sfp[1].asString);
	struct stat buf;
	int ret = stat(path, &buf);
	struct _kStat *stat = NULL;
	if(ret != -1) {
		stat = (struct _kStat *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)&buf);
	}
	else {
		// TODO: throw
		RETURN_(KLIB Knull(kctx, O_ct(sfp[K_RTNIDX].o)));
	}
	RETURN_(stat);
}

//## Stat System.lstat(String path)
static KMETHOD System_lstat(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = S_text(sfp[1].asString);
	struct stat buf;
	int ret = lstat(path, &buf);
	struct _kStat *stat = NULL;
	if(ret != -1) {
		stat = (struct _kStat *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)&buf);
	}
	else {
		// TODO: throw
	}
	RETURN_(stat);
}

//## Stat System.fstat(int fd)
static KMETHOD System_fstat(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	struct stat buf;
	int ret = fstat(fd, &buf);
	struct _kStat *stat = NULL;
	if(ret != -1) {
		stat = (struct _kStat *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)&buf);
	}
	else {
		// TODO: throw
	}
	RETURN_(stat);
}

//## int Stat.getst_dev()
static KMETHOD Stat_getst_dev(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_dev);
}

//## int Stat.getst_ino()
static KMETHOD Stat_getst_ino(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_ino);
}

//## int Stat.getst_mode()
static KMETHOD Stat_getst_mode(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_mode);
}

//## int Stat.getst_nlink()
static KMETHOD Stat_getst_nlink(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_nlink);
}

//## int Stat.getst_uid()
static KMETHOD Stat_getst_uid(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_uid);
}

//## int Stat.getst_gid()
static KMETHOD Stat_getst_gid(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_gid);
}

//## int Stat.getst_size()
static KMETHOD Stat_getst_size(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_size);
}

//## int Stat.getst_atime()
static KMETHOD Stat_getst_atime(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_atime);
}

//## int Stat.getst_mtime()
static KMETHOD Stat_getst_mtime(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_mtime);
}

//## int Stat.getst_ctime()
static KMETHOD Stat_getst_ctime(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_ctime);
}

#ifdef HAVE_STRUCT_STAT_ST_RDEV
//## int Stat.getst_rdev()
static KMETHOD Stat_getst_rdev(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_rdev);
}
#endif /* HAVE_STRUCT_STAT_ST_RDEV */

#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
//## int Stat.getst_blocks()
static KMETHOD Stat_getst_blocks(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_blocks);
}
#endif /* HAVE_STRUCT_STAT_ST_BLOCKS */

#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
//## int Stat.getst_blksize()
static KMETHOD Stat_getst_blksize(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_blksize);
}
#endif /* HAVE_STRUCT_STAT_ST_BLKSIZE */

#ifdef HAVE_STRUCT_STAT_ST_FLAGS
//## int Stat.getst_flags()
static KMETHOD Stat_getst_flags(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_flags);
}
#endif /* HAVE_STRUCT_STAT_ST_FLAGS */

#ifdef HAVE_STRUCT_STAT_ST_GEN
//## int Stat.getst_gen()
static KMETHOD Stat_getst_gen(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_gen);
}
#endif /* HAVE_STRUCT_STAT_ST_GEN */

#ifdef HAVE_STRUCT_STAT_ST_BIRTHTIME
//## int Stat.getst_birthtime()
static KMETHOD Stat_getst_birthtime(KonohaContext *kctx, KonohaStack *sfp)
{
	kStat *stat = (kStat *)sfp[0].asObject;
	RETURNi_(stat->stat->st_birthtime);
}
#endif /* HAVE_STRUCT_STAT_ST_BIRTHTIME */

//## DIR System.opendir(String name)
static KMETHOD System_opendir(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *name = S_text(sfp[1].asString);
	DIR *d = opendir(name);
	struct _kDIR *dir;
	if(d != NULL) {
		dir = (struct _kDIR *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)d);
	}
	else {
		// TODO: throw
	}
	RETURN_(dir);
}

//## int DIR.close()
static KMETHOD DIR_close(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDIR *dir = (struct _kDIR *)sfp[0].asObject;
	int ret = closedir(dir->dirp);
	if(ret == -1) {
		// TODO: throw
	}
	dir->dirp = NULL;
	RETURNi_(ret);
}

//## int DIR.getfd()
static KMETHOD DIR_getfd(KonohaContext *kctx, KonohaStack *sfp)
{
	kDIR *dir = (kDIR *)sfp[0].asObject;
	int fd = dirfd(dir->dirp);
	if(fd == -1) {
		// TODO: throw
	}
	RETURNi_(fd);
}

//## Array[Dirent] DIR.read()
static KMETHOD DIR_read(KonohaContext *kctx, KonohaStack *sfp)
{
	kDIR *dir = (kDIR *)sfp[0].asObject;
	DIR *dirp = dir->dirp;
	struct dirent entry;
	struct dirent *result;
	int ret;
	ktype_t TY_Dirent = O_p0(sfp[K_RTNIDX].o);
	KonohaClass *CT_Dirent = CT_(TY_Dirent);
	kArray *a = (kArray*)KLIB new_kObject(kctx, CT_p0(kctx, CT_Array, TY_Dirent), 0);

	while((ret = readdir_r(dirp, &entry, &result)) == 0) {
		if(result == NULL) break;
		KLIB kArray_add(kctx, a, KLIB new_kObject(kctx, CT_Dirent, (uintptr_t)result));
	}
	if(ret != 0) {
		// TODO: throw
	}
	RETURN_(a);
}

//## void DIR.rewind()
static KMETHOD DIR_rewind(KonohaContext *kctx, KonohaStack *sfp)
{
	kDIR *dir = (kDIR *)sfp[0].asObject;
	DIR *dirp = dir->dirp;
	rewinddir(dirp);
	RETURNvoid_();
}

//## void DIR.seek(int loc)
static KMETHOD DIR_seek(KonohaContext *kctx, KonohaStack *sfp)
{
	kDIR *dir = (kDIR *)sfp[0].asObject;
	DIR *dirp = dir->dirp;
	long loc = sfp[1].intValue;
	seekdir(dirp, loc);
	RETURNvoid_();
}

//## int DIR.tell()
static KMETHOD DIR_tell(KonohaContext *kctx, KonohaStack *sfp)
{
	kDIR *dir = (kDIR *)sfp[0].asObject;
	DIR *dirp = dir->dirp;
	long ret = telldir(dirp);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int Dirent.getd_ino()
static KMETHOD Dirent_getd_ino(KonohaContext *kctx, KonohaStack *sfp)
{
	kDirent *dirent = (kDirent *)sfp[0].asObject;
	struct dirent *entry = dirent->entry;
	ino_t d_ino = entry->d_ino;
	RETURNi_(d_ino);
}

#ifdef _DIRENT_HAVE_D_OFF
//## int Dirent.getd_off()
static KMETHOD Dirent_getd_off(KonohaContext *kctx, KonohaStack *sfp)
{
	kDirent *dirent = (kDirent *)sfp[0].asObject;
	struct dirent *entry = dirent->entry;
	off_t d_off = entry->d_off;
	RETURNi_(d_off);
}
#endif /* _DIRENT_HAVE_D_OFF */

//## int Dirent.getd_reclen()
static KMETHOD Dirent_getd_reclen(KonohaContext *kctx, KonohaStack *sfp)
{
	kDirent *dirent = (kDirent *)sfp[0].asObject;
	struct dirent *entry = dirent->entry;
	unsigned short d_reclen = entry->d_reclen;
	RETURNi_(d_reclen);
}

//## int Dirent.getd_type()
static KMETHOD Dirent_getd_type(KonohaContext *kctx, KonohaStack *sfp)
{
	kDirent *dirent = (kDirent *)sfp[0].asObject;
	struct dirent *entry = dirent->entry;
	unsigned char d_type = entry->d_type;
	RETURNi_(d_type);
}

//## String Dirent.getd_name()
static KMETHOD Dirent_getd_name(KonohaContext *kctx, KonohaStack *sfp)
{
	kDirent *dirent = (kDirent *)sfp[0].asObject;
	struct dirent *entry = dirent->entry;
	char *d_name = entry->d_name;
	RETURN_(KLIB new_kString(kctx, d_name, strlen(d_name), 0));
}

//## int System.getdtablesize()
static KMETHOD System_getdtablesize(KonohaContext *kctx, KonohaStack *sfp)
{
	int ret = getdtablesize();
	RETURNi_(ret);
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
	}
	RETURNi_(ret);
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
	}
	RETURNi_(ret);
}

//## int System.fchdir(int fd)
static KMETHOD System_fchdir(KonohaContext *kctx, KonohaStack *sfp)
{
	int ch = fchdir(sfp[1].intValue);
	if(ch == -1) {
		// TODO: throw
	}
	RETURNi_(ch);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Static   kMethod_Static
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define CT_Stat         cStat
#define TY_Stat         cStat->typeId
#define CT_DIR          cDIR
#define TY_DIR          cDIR->typeId
#define CT_Dirent       cDirent
#define TY_Dirent       cDirent->typeId

#define _KVi(T) #T, TY_int, T

static kbool_t fd_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_CLASS defStat = {
		STRUCTNAME(Stat),
		.cflag = kClass_Final,
		.init  = kStat_init,
		.free  = kStat_free,
		.p     = kStat_p
	};
	KonohaClass *cStat = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defStat, pline);

	KDEFINE_CLASS defDIR = {
		STRUCTNAME(DIR),
		.cflag = kClass_Final,
		.init  = kDIR_init,
		.free  = kDIR_free,
		.p     = kDIR_p
	};
	KonohaClass *cDIR = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defDIR, pline);

	KDEFINE_CLASS defDirent = {
		STRUCTNAME(Dirent),
		.cflag = kClass_Final,
		.init  = kDirent_init,
		.free  = kDirent_free,
		.p     = kDirent_p
	};
	KonohaClass *cDirent = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defDirent, pline);
	KonohaClass *CT_DirentArray0 = CT_p0(kctx, CT_Array, cDirent->typeId);
	ktype_t TY_DirentArray0 = CT_DirentArray0->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_Const|_Im, _F(System_lseek), TY_int, TY_System, MN_("lseek"), 3, TY_int, FN_("fd"), TY_int, FN_("offset"), TY_int, FN_("whence"),
		_Public|_Static|_Const|_Im, _F(System_getCwd), TY_String, TY_System, MN_("getCwd"), 0,
		_Public|_Static|_Const|_Im, _F(System_ftruncate), TY_boolean, TY_System, MN_("ftruncate"), 2, TY_int, FN_("fd"), TY_int, FN_("length"),
		_Public|_Static|_Const|_Im, _F(System_fchmod), TY_boolean, TY_System, MN_("fchmod"), 2, TY_int, FN_("fd"), TY_int, FN_("length"),
		_Public|_Static|_Const|_Im, _F(System_flock), TY_boolean, TY_System, MN_("flock"), 2, TY_int, FN_("fd"), TY_int, FN_("operation"),
		_Public|_Static|_Const|_Im, _F(System_sync), TY_boolean, TY_System, MN_("sync"), 1, TY_int, FN_("fd"),
		_Public|_Static|_Const|_Im, _F(System_link), TY_int, TY_System, MN_("link"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Static|_Const|_Im, _F(System_unlink), TY_int, TY_System, MN_("unlink"), 1, TY_String, FN_("pathname"),
		_Public|_Static|_Const|_Im, _F(System_rename), TY_int, TY_System, MN_("rename"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Static|_Const|_Im, _F(System_rmdir), TY_int, TY_System, MN_("rmdir"), 1, TY_String, FN_("pathname"),
		_Public|_Static|_Const|_Im, _F(System_symlink), TY_int, TY_System, MN_("symlink"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Static|_Const|_Im, _F(System_readlink), TY_String, TY_System, MN_("readlink"), 1, TY_String, FN_("pathname"),
		_Public|_Static|_Const|_Im, _F(System_chown), TY_int, TY_System, MN_("chown"), 3, TY_String, FN_("pathname"), TY_int, FN_("owner"), TY_int, FN_("group"),
		_Public|_Static|_Const|_Im, _F(System_lchown), TY_int, TY_System, MN_("lchown"), 3, TY_String, FN_("pathname"), TY_int, FN_("owner"), TY_int, FN_("group"),
		_Public|_Static|_Const|_Im, _F(System_fchown), TY_int, TY_System, MN_("fchown"), 3, TY_int, FN_("pd"), TY_int, FN_("owner"), TY_int, FN_("group"),
		_Public|_Static|_Const|_Im, _F(System_access), TY_int, TY_System, MN_("access"), 2, TY_String, FN_("pathname"), TY_int, FN_("mode"),
		_Public|_Static|_Const|_Im, _F(System_fsync), TY_int, TY_System, MN_("fsync"), 1, TY_int, FN_("fd"),
		_Public|_Static|_Const|_Im, _F(System_stat), TY_Stat, TY_System, MN_("stat"), 1, TY_String, FN_("path"),
		_Public|_Static|_Const|_Im, _F(System_lstat), TY_Stat, TY_System, MN_("lstat"), 1, TY_String, FN_("path"),
		_Public|_Static|_Const|_Im, _F(System_fstat), TY_Stat, TY_System, MN_("fstat"), 1, TY_int, FN_("fd"),
		_Public|_Const|_Im, _F(Stat_getst_dev), TY_int, TY_Stat, MN_("getst_dev"), 0,
		_Public|_Const|_Im, _F(Stat_getst_ino), TY_int, TY_Stat, MN_("getst_ino"), 0,
		_Public|_Const|_Im, _F(Stat_getst_mode), TY_int, TY_Stat, MN_("getst_mode"), 0,
		_Public|_Const|_Im, _F(Stat_getst_nlink), TY_int, TY_Stat, MN_("getst_nlink"), 0,
		_Public|_Const|_Im, _F(Stat_getst_uid), TY_int, TY_Stat, MN_("getst_uid"), 0,
		_Public|_Const|_Im, _F(Stat_getst_gid), TY_int, TY_Stat, MN_("getst_gid"), 0,
		_Public|_Const|_Im, _F(Stat_getst_size), TY_int, TY_Stat, MN_("getst_size"), 0,
		_Public|_Const|_Im, _F(Stat_getst_atime), TY_int, TY_Stat, MN_("getst_atime"), 0,
		_Public|_Const|_Im, _F(Stat_getst_mtime), TY_int, TY_Stat, MN_("getst_mtime"), 0,
		_Public|_Const|_Im, _F(Stat_getst_ctime), TY_int, TY_Stat, MN_("getst_ctime"), 0,
#ifdef HAVE_STRUCT_STAT_ST_RDEV
		_Public|_Const|_Im, _F(Stat_getst_rdev), TY_int, TY_Stat, MN_("getst_rdev"), 0,
#endif /* HAVE_STRUCT_STAT_ST_RDEV */
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
		_Public|_Const|_Im, _F(Stat_getst_blocks), TY_int, TY_Stat, MN_("getst_blocks"), 0,
#endif /* HAVE_STRUCT_STAT_ST_BLOCKS */
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
		_Public|_Const|_Im, _F(Stat_getst_blksize), TY_int, TY_Stat, MN_("getst_blksize"), 0,
#endif /* HAVE_STRUCT_STAT_ST_BLKSIZE */
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
		_Public|_Const|_Im, _F(Stat_getst_flags), TY_int, TY_Stat, MN_("getst_flags"), 0,
#endif /* HAVE_STRUCT_STAT_ST_FLAGS */
#ifdef HAVE_STRUCT_STAT_ST_GEN
		_Public|_Const|_Im, _F(Stat_getst_gen), TY_int, TY_Stat, MN_("getst_gen"), 0,
#endif /* HAVE_STRUCT_STAT_ST_GEN */
#ifdef HAVE_STRUCT_STAT_ST_BIRTHTIME
		_Public|_Const|_Im, _F(Stat_getst_birthtime), TY_int, TY_Stat, MN_("getst_birthtime"), 0,
#endif /* HAVE_STRUCT_STAT_ST_BIRTHTIME */
		_Public|_Static|_Const|_Im, _F(System_opendir), TY_DIR, TY_System, MN_("opendir"), 1, TY_String, FN_("dirname"),
		_Public|_Const|_Im, _F(DIR_close), TY_int, TY_DIR, MN_("close"), 0,
		_Public|_Const|_Im, _F(DIR_getfd), TY_int, TY_DIR, MN_("getfd"), 0,
		_Public|_Const|_Im, _F(DIR_read), TY_DirentArray0, TY_DIR, MN_("read"), 0,
		_Public|_Const|_Im, _F(DIR_rewind), TY_void, TY_DIR, MN_("rewind"), 0,
		_Public|_Const|_Im, _F(DIR_seek), TY_void, TY_DIR, MN_("seek"), 1, TY_int, FN_("loc"),
		_Public|_Const|_Im, _F(DIR_tell), TY_int, TY_DIR, MN_("tell"), 0,
		_Public|_Const|_Im, _F(Dirent_getd_ino), TY_int, TY_Dirent, MN_("getd_ino"), 0,
#ifdef _DIRENT_HAVE_D_OFF
		_Public|_Const|_Im, _F(Dirent_getd_off), TY_int, TY_Dirent, MN_("getd_off"), 0,
#endif /* _DIRENT_HAVE_D_OFF */
		_Public|_Const|_Im, _F(Dirent_getd_reclen), TY_int, TY_Dirent, MN_("getd_reclen"), 0,
		_Public|_Const|_Im, _F(Dirent_getd_type), TY_int, TY_Dirent, MN_("getd_type"), 0,
		_Public|_Const|_Im, _F(Dirent_getd_name), TY_String, TY_Dirent, MN_("getd_name"), 0,
		_Public|_Static|_Const|_Im, _F(System_getdtablesize), TY_int, TY_System, MN_("getdtablesize"), 0,
		_Public|_Static|_Im, _F(System_open), TY_int, TY_System, MN_("open"), 2, TY_String, FN_("pathname"), TY_int, FN_("flags"),
		_Public|_Static|_Im, _F(System_open_mode), TY_int, TY_System, MN_("open"), 3, TY_String, FN_("pathname"), TY_int, FN_("flags"), TY_int, FN_("mode"),
		_Public|_Static|_Im, _F(System_fchdir), TY_int, TY_System, MN_("fchdir"), 1, TY_int, FN_("fd"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	KDEFINE_INT_CONST intData[] = {
		/*for System.access*/
		{_KVi(R_OK)},
		{_KVi(W_OK)},
		{_KVi(X_OK)},
		{_KVi(F_OK)},
		/*for System.lseek*/
		{_KVi(SEEK_SET)},
		{_KVi(SEEK_CUR)},
		{_KVi(SEEK_END)},
		/*for System.flock*/
		{_KVi(LOCK_SH)},
		{_KVi(LOCK_EX)},
		{_KVi(LOCK_UN)},
		{_KVi(LOCK_NB)},
		/*for System.open*/
		{_KVi(O_RDONLY)},
		{_KVi(O_WRONLY)},
		{_KVi(O_RDWR)},
		{_KVi(O_CREAT)},
		{_KVi(O_EXCL)},
		{_KVi(O_TRUNC)},
		{_KVi(O_APPEND)},
		{_KVi(O_NONBLOCK)},
		{_KVi(O_NDELAY)},
		{_KVi(O_NOCTTY)},
		{}
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(intData), 0);
	return true;
}

static kbool_t fd_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t fd_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t fd_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* fd_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("fd", "1.0"),
		.initPackage = fd_initPackage,
		.setupPackage = fd_setupPackage,
		.initNameSpace = fd_initNameSpace,
		.setupNameSpace = fd_setupNameSpace,
	};
	return &d;
}
