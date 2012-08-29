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


#ifndef FD_GLUE_H_
#define FD_GLUE_H_

#include<stdio.h>

/* ======================================================================== */
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
		ktrace(_DataFault,
			   KeyValue_s("@", "lseek"),
			   KeyValue_u("offset", offset),
			   KeyValue_u("whence", whence),
			   KeyValue_s("errstr", strerror(errno))
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
//## boolean System.truncate(int fd, int length)
static KMETHOD System_truncate(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int length = sfp[2].intValue;
	int ret = ftruncate(fd, length);
	if (ret != 0) {
		ktrace(_SystemFault,
			   KeyValue_s("@", "truncate"),
			   KeyValue_u("length", length),
			   KeyValue_s("errstr", strerror(errno))
			);
	}
	RETURNb_(ret == 0);
}

//## boolean System.chmod(int fd, int length)
static KMETHOD System_chmod(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	int mode = sfp[2].intValue;
	int ret = fchmod(fd, mode);
	if (ret != -1) {
		ktrace(_SystemFault,
			   KeyValue_s("@", "chmod"),
			   KeyValue_u("mode", mode),
			   KeyValue_s("errstr", strerror(errno))
			);
	}
	RETURNb_(ret == 0);
}

//## boolean System.chown(int fd, int owner, int group)
static KMETHOD System_chown(KonohaContext *kctx, KonohaStack *sfp)
{
	int fd = sfp[1].intValue;
	uid_t owner = (uid_t)sfp[2].intValue;
	gid_t group = (gid_t)sfp[3].intValue;

	int ret = fchown(fd, owner, group);
	if (ret != -1) {
		ktrace(_SystemFault,
			   KeyValue_s("@", "chown"),
			   KeyValue_u("owner", owner),
			   KeyValue_u("group", group),
			   KeyValue_s("errstr", strerror(errno))
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
//	int request  = Int_to(int, sfp[1]);
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "flock"),
			   KeyValue_u("operation", operation),
			   KeyValue_s("errstr", strerror(errno))
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "fsync"),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "link"),
			   KeyValue_s("oldpath", oldpath),
			   KeyValue_s("newpath", newpath),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "unlink"),
			   KeyValue_s("pathname", pathname),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "rename"),
			   KeyValue_s("oldpath", oldpath),
			   KeyValue_s("newpath", newpath),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "rmdir"),
			   KeyValue_s("pathname", pathname),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "symlink"),
			   KeyValue_s("oldpath", oldpath),
			   KeyValue_s("newpath", newpath),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}

static KMETHOD System_readlink(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s1 = sfp[1].asString;
	kString *s2 = sfp[2].asString;
	const char *pathname = S_text(s1);
	const char *buf = S_text(s2);
	size_t bufsize = strlen(buf);
	ssize_t ret = readlink(pathname, (char *)buf, bufsize);
	if (ret == -1) {
		ktrace(_SystemFault,
			   KeyValue_s("@", "readlink"),
			   KeyValue_s("pathname", pathname),
			   KeyValue_s("buf", buf),
			   KeyValue_u("bufsize", bufsize),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "lchown"),
			   KeyValue_s("pathname", pathname),
			   KeyValue_u("owner", owner),
			   KeyValue_u("group", group),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "fchown"),
			   KeyValue_u("owner", owner),
			   KeyValue_u("group", group),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "access"),
			   KeyValue_s("pathname", pathname),
			   KeyValue_u("mode", mode),
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
		ktrace(_SystemFault,
			   KeyValue_s("@", "fsync"),
			   KeyValue_p("errstr", strerror(errno))
			);
	}
	RETURNi_(ret);
}
// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t fd_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(System_lseek), TY_Int, TY_System, MN_("lseek"), 3, TY_Int, FN_("fd"), TY_Int, FN_("offset"), TY_Int, FN_("whence"),
		_Public|_Const|_Im, _F(System_getCwd), TY_String, TY_System, MN_("getCwd"), 0,
		_Public|_Const|_Im, _F(System_truncate), TY_Boolean, TY_System, MN_("truncate"), 2, TY_Int, FN_("fd"), TY_Int, FN_("length"),
		_Public|_Const|_Im, _F(System_chmod), TY_Boolean, TY_System, MN_("chmod"), 2, TY_Int, FN_("fd"), TY_Int, FN_("length"),
		_Public|_Const|_Im, _F(System_chown), TY_Boolean, TY_System, MN_("chown"), 3, TY_Int, FN_("fd"), TY_Int, FN_("owner"), TY_Int, FN_("group"),
		_Public|_Const|_Im, _F(System_flock), TY_Boolean, TY_System, MN_("flock"), 2, TY_Int, FN_("fd"), TY_Int, FN_("operation"),
		_Public|_Const|_Im, _F(System_sync), TY_Boolean, TY_System, MN_("sync"), 1, TY_Int, FN_("fd"),
		_Public|_Const|_Im, _F(System_link), TY_Int, TY_System, MN_("link"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Const|_Im, _F(System_unlink), TY_Int, TY_System, MN_("unlink"), 1, TY_String, FN_("pathname"),
		_Public|_Const|_Im, _F(System_rename), TY_Int, TY_System, MN_("rename"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Const|_Im, _F(System_rmdir), TY_Int, TY_System, MN_("rmdir"), 1, TY_String, FN_("pathname"),
		_Public|_Const|_Im, _F(System_symlink), TY_Int, TY_System, MN_("symlink"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Const|_Im, _F(System_readlink), TY_Int, TY_System, MN_("readlink"), 3, TY_String, FN_("pathname"), TY_String, FN_("buf"), TY_Int, FN_("bufsize"),
		_Public|_Const|_Im, _F(System_lchown), TY_Int, TY_System, MN_("lchown"), 3, TY_String, FN_("pathname"), TY_Int, FN_("owner"), TY_Int, FN_("group"),
		_Public|_Const|_Im, _F(System_fchown), TY_Int, TY_System, MN_("fchown"), 3, TY_Int, FN_("pd"), TY_Int, FN_("owner"), TY_Int, FN_("group"),
		_Public|_Const|_Im, _F(System_access), TY_Int, TY_System, MN_("access"), 2, TY_String, FN_("pathname"), TY_Int, FN_("mode"),
		_Public|_Const|_Im, _F(System_fsync), TY_Int, TY_System, MN_("fsync"), 1, TY_Int, FN_("fd"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t fd_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t fd_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t fd_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}



#endif /* FD_GLUE_H_ */
