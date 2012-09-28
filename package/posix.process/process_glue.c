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
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <grp.h>
#ifdef __linux__
#include <sys/wait.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif


/* ------------------------------------------------------------------------ */

static KMETHOD System_getpid(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(getpid());
}

static KMETHOD System_getppid(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(getppid());
}

static KMETHOD System_getuid(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(getuid());
}

static KMETHOD System_geteuid(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(geteuid());
}

static KMETHOD System_getgid(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(getgid());
}

static KMETHOD System_getegid(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(getegid());
}

static KMETHOD System_getpgid(KonohaContext *kctx, KonohaStack *sfp)
{
	int pid = sfp[1].intValue;
	int ret = getpgid(pid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

static KMETHOD System_setpgid(KonohaContext *kctx, KonohaStack *sfp)
{
	int pid = sfp[1].intValue;
	int pgid = sfp[2].intValue;
	int ret = setpgid(pid, pgid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

static KMETHOD System_chdir(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *dir = S_text(s);
	int ret = chdir(dir);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

static KMETHOD System_chroot(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *root = S_text(s);
	int ret = chroot(root);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

static KMETHOD System_getpriority(KonohaContext *kctx, KonohaStack *sfp)
{

	int which = sfp[1].intValue;
	int who = sfp[2].intValue;
	int ret = getpriority(which, who);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

static KMETHOD System_setpriority(KonohaContext *kctx, KonohaStack *sfp)
{
	int which = sfp[1].intValue;
	int who = sfp[2].intValue;
	int priority = sfp[3].intValue;
	int ret = setpriority(which, who, priority);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

static KMETHOD System_getgroups(KonohaContext *kctx, KonohaStack *sfp)
{
	int size = sfp[1].intValue;
	kArray *list = sfp[2].asArray;
	int ret = getgroups(size, (gid_t *)(list->kintItems));
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

static KMETHOD System_setgroups(KonohaContext *kctx, KonohaStack *sfp)
{
	int size = sfp[1].intValue;
	kArray *list = sfp[2].asArray;
	int ret = setgroups(size, (const gid_t *)(list->kintItems));
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int System.fork()
static KMETHOD System_fork(KonohaContext *kctx, KonohaStack *sfp)
{
	pid_t pid = fork();
	if(pid == -1) {
		// TODO: throw
	}
	RETURNi_(pid);
}

//## int System.wait()
static KMETHOD System_wait(KonohaContext *kctx, KonohaStack *sfp)
{
	int status = 0;
	pid_t pid = wait(&status);
	if(pid == -1) {
		// TODO: throw
	}
	RETURNi_(pid);
}

//## int System.waitpid(int pid, int options)
static KMETHOD System_waitpid(KonohaContext *kctx, KonohaStack *sfp)
{
	int status = 0;
	pid_t target_pid = sfp[1].intValue;
	int options = sfp[2].intValue;
	pid_t pid = waitpid(target_pid, &status, options);
	if(pid == -1) {
		// TODO: throw
	}
	RETURNi_(pid);
}

//## int System.setuid(int uid)
static KMETHOD System_setuid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t uid = sfp[1].intValue;
	int ret = setuid(uid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int System.seteuid(int euid)
static KMETHOD System_seteuid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t euid = sfp[1].intValue;
	int ret = seteuid(euid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int System.setreuid(int ruid, int euid)
static KMETHOD System_setreuid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t ruid = sfp[1].intValue;
	uid_t euid = sfp[2].intValue;
	int ret = setreuid(ruid, euid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int System.setgid(int gid)
static KMETHOD System_setgid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t gid = sfp[1].intValue;
	int ret = setgid(gid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int System.setegid(int euid)
static KMETHOD System_setegid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t egid = sfp[1].intValue;
	int ret = setegid(egid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int System.setregid(int rgid, int egid)
static KMETHOD System_setregid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t rgid = sfp[1].intValue;
	uid_t egid = sfp[2].intValue;
	int ret = setregid(rgid, egid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int System.setsid()
static KMETHOD System_setsid(KonohaContext *kctx, KonohaStack *sfp)
{
	pid_t ret = setsid();
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

//## int System.getsid(int pid)
static KMETHOD System_getsid(KonohaContext *kctx, KonohaStack *sfp)
{
	pid_t pid = sfp[1].intValue;
	pid_t ret = getsid(pid);
	if(ret == -1) {
		// TODO: throw
	}
	RETURNi_(ret);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Static   kMethod_Static
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

#define _KVi(T) #T, TY_int, T

static kbool_t process_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	kparamtype_t p = { .ty = TY_int,  };
	KonohaClass *cintArray = KLIB KonohaClass_Generics(kctx, CT_(TY_Array), TY_void, 1, &p);
#define TY_intArray (cintArray->typeId)


	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(System_getpid), TY_int, TY_System, MN_("getpid"), 0,
		_Public|_Static, _F(System_getppid), TY_int, TY_System, MN_("getppid"), 0,
		_Public|_Static, _F(System_getuid), TY_int, TY_System, MN_("getuid"), 0,
		_Public|_Static, _F(System_geteuid), TY_int, TY_System, MN_("geteuid"), 0,
		_Public|_Static, _F(System_getgid), TY_int, TY_System, MN_("getgid"), 0,
		_Public|_Static, _F(System_getegid), TY_int, TY_System, MN_("getegid"), 0,
		_Public|_Static, _F(System_getpgid), TY_int, TY_System, MN_("getpgid"), 1, TY_int, FN_("pid"),
		_Public|_Static, _F(System_setpgid), TY_int, TY_System, MN_("setpgid"), 2, TY_int, FN_("pid"), TY_int, FN_("pgid"),
		_Public|_Static, _F(System_chdir), TY_int, TY_System, MN_("chdir"), 1, TY_String, FN_("pathname"),
		_Public|_Static, _F(System_chroot), TY_int, TY_System, MN_("chroot"), 1, TY_String, FN_("pathname"),
		_Public|_Static, _F(System_getpriority), TY_int, TY_System, MN_("getpriority"), 2, TY_int, FN_("which"), TY_int, FN_("who"),
		_Public|_Static, _F(System_setpriority), TY_int, TY_System, MN_("setpriority"), 3, TY_int, FN_("which"), TY_int, FN_("who"), TY_int, FN_("priority"),
		_Public|_Static, _F(System_getgroups), TY_int, TY_System, MN_("getgroups"), 2, TY_int, FN_("size"), TY_intArray, FN_("list[]"),
		_Public|_Static, _F(System_setgroups), TY_int, TY_System, MN_("setgroups"), 2, TY_int, FN_("size"), TY_intArray, FN_("*list"),
		_Public|_Static, _F(System_fork), TY_int, TY_System, MN_("fork"), 0,
		_Public|_Static, _F(System_wait), TY_int, TY_System, MN_("wait"), 0,
		_Public|_Static, _F(System_waitpid), TY_int, TY_System, MN_("wait"), 2, TY_int, FN_("pid"), TY_int, FN_("options"),
		_Public|_Static, _F(System_setuid), TY_int, TY_System, MN_("setuid"), 1, TY_int, FN_("uid"),
		_Public|_Static, _F(System_seteuid), TY_int, TY_System, MN_("seteuid"), 1, TY_int, FN_("euid"),
		_Public|_Static, _F(System_setreuid), TY_int, TY_System, MN_("setreuid"), 2, TY_int, FN_("ruid"), TY_int, FN_("euid"),
		_Public|_Static, _F(System_setgid), TY_int, TY_System, MN_("setgid"), 1, TY_int, FN_("gid"),
		_Public|_Static, _F(System_setegid), TY_int, TY_System, MN_("setguid"), 1, TY_int, FN_("egid"),
		_Public|_Static, _F(System_setregid), TY_int, TY_System, MN_("setrguid"), 2, TY_int, FN_("rgid"), TY_int, FN_("egid"),
		_Public|_Static, _F(System_setsid), TY_int, TY_System, MN_("setsid"), 0,
		_Public|_Static, _F(System_getsid), TY_int, TY_System, MN_("getsid"), 1, TY_int, FN_("pid"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	KDEFINE_INT_CONST intData[] = {
		{_KVi(SIGHUP)},
		{_KVi(SIGINT)},
		{_KVi(SIGABRT)},
		{_KVi(SIGKILL)},
		/*for System.setpriority*/
		{_KVi(PRIO_PROCESS)},
		{_KVi(PRIO_PGRP)},
		{_KVi(PRIO_USER)},
		{}
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(intData), 0);
	return true;
}

static kbool_t process_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t process_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t process_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* process_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("process", "1.0"),
		KPACKLIB("POSIX.1", "1.0"),
		.initPackage = process_initPackage,
		.setupPackage = process_setupPackage,
		.initNameSpace = process_initNameSpace,
		.setupNameSpace = process_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif

