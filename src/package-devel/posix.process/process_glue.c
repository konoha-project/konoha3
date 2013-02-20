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

#ifdef __linux__
#define _XOPEN_SOURCE 500 /* Needed to get getpgid and getsid */
#define _BSD_SOURCE       /* Needed to get setgroups */
#endif
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <grp.h>
#ifdef __linux__
#include <sys/wait.h>
#endif

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C"{
#endif

/* ------------------------------------------------------------------------ */

static KMETHOD System_getpid(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(getpid());
}

static KMETHOD System_getppid(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(getppid());
}

static KMETHOD System_getuid(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(getuid());
}

static KMETHOD System_geteuid(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(geteuid());
}

static KMETHOD System_getgid(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(getgid());
}

static KMETHOD System_getegid(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(getegid());
}

static KMETHOD System_getpgid(KonohaContext *kctx, KonohaStack *sfp)
{
	int pid = sfp[1].intValue;
	int ret = getpgid(pid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

static KMETHOD System_Setpgid(KonohaContext *kctx, KonohaStack *sfp)
{
	int pid = sfp[1].intValue;
	int pgid = sfp[2].intValue;
	int ret = setpgid(pid, pgid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

static KMETHOD System_getpriority(KonohaContext *kctx, KonohaStack *sfp)
{

	int which = sfp[1].intValue;
	int who = sfp[2].intValue;
	int ret = getpriority(which, who);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

static KMETHOD System_Setpriority(KonohaContext *kctx, KonohaStack *sfp)
{
	int which = sfp[1].intValue;
	int who = sfp[2].intValue;
	int priority = sfp[3].intValue;
	int ret = setpriority(which, who, priority);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

static KMETHOD System_getgroups(KonohaContext *kctx, KonohaStack *sfp)
{
	int size = sfp[1].intValue;
	kArray *list = sfp[2].asArray;
	int ret = getgroups(size, (gid_t *)(list->kintItems));
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

static KMETHOD System_Setgroups(KonohaContext *kctx, KonohaStack *sfp)
{
	int size = sfp[1].intValue;
	kArray *list = sfp[2].asArray;
	int ret = setgroups(size, (const gid_t *)(list->kintItems));
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.fork()
static KMETHOD System_fork(KonohaContext *kctx, KonohaStack *sfp)
{
	pid_t pid = fork();
	if(pid == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(pid);
}

//## int System.wait()
static KMETHOD System_wait(KonohaContext *kctx, KonohaStack *sfp)
{
	int status = 0;
	pid_t pid = wait(&status);
	if(pid == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(pid);
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
	KReturnUnboxValue(pid);
}

//## int System.setuid(int uid)
static KMETHOD System_Setuid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t uid = sfp[1].intValue;
	int ret = setuid(uid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.seteuid(int euid)
static KMETHOD System_Seteuid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t euid = sfp[1].intValue;
	int ret = seteuid(euid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.setreuid(int ruid, int euid)
static KMETHOD System_Setreuid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t ruid = sfp[1].intValue;
	uid_t euid = sfp[2].intValue;
	int ret = setreuid(ruid, euid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.setgid(int gid)
static KMETHOD System_Setgid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t gid = sfp[1].intValue;
	int ret = setgid(gid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.setegid(int euid)
static KMETHOD System_Setegid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t egid = sfp[1].intValue;
	int ret = setegid(egid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.setregid(int rgid, int egid)
static KMETHOD System_Setregid(KonohaContext *kctx, KonohaStack *sfp)
{
	uid_t rgid = sfp[1].intValue;
	uid_t egid = sfp[2].intValue;
	int ret = setregid(rgid, egid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.setsid()
static KMETHOD System_Setsid(KonohaContext *kctx, KonohaStack *sfp)
{
	pid_t ret = setsid();
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.getsid(int pid)
static KMETHOD System_getsid(KonohaContext *kctx, KonohaStack *sfp)
{
	pid_t pid = sfp[1].intValue;
	pid_t ret = getsid(pid);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.sleep(int sec)
static KMETHOD System_sleep(KonohaContext *kctx, KonohaStack *sfp)
{
	unsigned int sec = sfp[1].intValue;
	unsigned int left = sleep(sec);
	KReturnUnboxValue(left);
}

//## boolean System.usleep(int usec)
static KMETHOD System_usleep(KonohaContext *kctx, KonohaStack *sfp)
{
	unsigned int usec = sfp[1].intValue;
	int ret = usleep(usec);
	if(ret != 0) {
		// TODO KTraceApi
	}
	KReturnUnboxValue(ret == 0);
}

//## int System.system(String command)
static KMETHOD System_system(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *command = kString_text(sfp[1].asString);
	int ret = system(command);
	if(ret != 0) {
		// TODO KTraceApi
	}
	KReturnUnboxValue(ret);
}

//## String System.getenv(String name)
static KMETHOD System_getenv(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *name = kString_text(sfp[1].asString);
	char *ret = getenv(name);
	if(ret == NULL) {
		KReturn(KNULL(String));
	}
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), 0));
}

/* ------------------------------------------------------------------------ */

#define KDefineConstInt(T) #T, KType_Int, T

static kbool_t process_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	kparamtype_t p = {KType_Int};
	KClass *cintArray = KLIB KClass_Generics(kctx, KClass_(KType_Array), KType_void, 1, &p);
#define KType_IntArray (cintArray->typeId)

	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(System_getpid), KType_Int, KType_System, KMethodName_("getpid"), 0,
		_Public|_Static, _F(System_getppid), KType_Int, KType_System, KMethodName_("getppid"), 0,
		_Public|_Static, _F(System_getuid), KType_Int, KType_System, KMethodName_("getuid"), 0,
		_Public|_Static, _F(System_geteuid), KType_Int, KType_System, KMethodName_("geteuid"), 0,
		_Public|_Static, _F(System_getgid), KType_Int, KType_System, KMethodName_("getgid"), 0,
		_Public|_Static, _F(System_getegid), KType_Int, KType_System, KMethodName_("getegid"), 0,
		_Public|_Static, _F(System_getpgid), KType_Int, KType_System, KMethodName_("getpgid"), 1, KType_Int, KFieldName_("pid"),
		_Public|_Static, _F(System_Setpgid), KType_Int, KType_System, KMethodName_("setpgid"), 2, KType_Int, KFieldName_("pid"), KType_Int, KFieldName_("pgid"),
		_Public|_Static, _F(System_getpriority), KType_Int, KType_System, KMethodName_("getpriority"), 2, KType_Int, KFieldName_("which"), KType_Int, KFieldName_("who"),
		_Public|_Static, _F(System_Setpriority), KType_Int, KType_System, KMethodName_("setpriority"), 3, KType_Int, KFieldName_("which"), KType_Int, KFieldName_("who"), KType_Int, KFieldName_("priority"),
		_Public|_Static, _F(System_getgroups), KType_Int, KType_System, KMethodName_("getgroups"), 2, KType_Int, KFieldName_("size"), KType_IntArray, KFieldName_("list[]"),
		_Public|_Static, _F(System_Setgroups), KType_Int, KType_System, KMethodName_("setgroups"), 2, KType_Int, KFieldName_("size"), KType_IntArray, KFieldName_("*list"),
		_Public|_Static, _F(System_fork), KType_Int, KType_System, KMethodName_("fork"), 0,
		_Public|_Static, _F(System_wait), KType_Int, KType_System, KMethodName_("wait"), 0,
		_Public|_Static, _F(System_waitpid), KType_Int, KType_System, KMethodName_("wait"), 2, KType_Int, KFieldName_("pid"), KType_Int, KFieldName_("options"),
		_Public|_Static, _F(System_Setuid), KType_Int, KType_System, KMethodName_("setuid"), 1, KType_Int, KFieldName_("uid"),
		_Public|_Static, _F(System_Seteuid), KType_Int, KType_System, KMethodName_("seteuid"), 1, KType_Int, KFieldName_("euid"),
		_Public|_Static, _F(System_Setreuid), KType_Int, KType_System, KMethodName_("setreuid"), 2, KType_Int, KFieldName_("ruid"), KType_Int, KFieldName_("euid"),
		_Public|_Static, _F(System_Setgid), KType_Int, KType_System, KMethodName_("setgid"), 1, KType_Int, KFieldName_("gid"),
		_Public|_Static, _F(System_Setegid), KType_Int, KType_System, KMethodName_("setguid"), 1, KType_Int, KFieldName_("egid"),
		_Public|_Static, _F(System_Setregid), KType_Int, KType_System, KMethodName_("setrguid"), 2, KType_Int, KFieldName_("rgid"), KType_Int, KFieldName_("egid"),
		_Public|_Static, _F(System_Setsid), KType_Int, KType_System, KMethodName_("setsid"), 0,
		_Public|_Static, _F(System_getsid), KType_Int, KType_System, KMethodName_("getsid"), 1, KType_Int, KFieldName_("pid"),
		_Public|_Static, _F(System_sleep), KType_Int, KType_System, KMethodName_("sleep"), 1, KType_Int, KFieldName_("sec"),
		_Public|_Static, _F(System_usleep), KType_Boolean, KType_System, KMethodName_("usleep"), 1, KType_Int, KFieldName_("usec"),
		_Public|_Static, _F(System_system), KType_Int, KType_System, KMethodName_("system"), 1, KType_String, KFieldName_("command"),
		_Public|_Static, _F(System_getenv), KType_String, KType_System, KMethodName_("getenv"), 1, KType_String, KFieldName_("name"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	KDEFINE_INT_CONST intData[] = {
		{KDefineConstInt(SIGHUP)},
		{KDefineConstInt(SIGINT)},
		{KDefineConstInt(SIGABRT)},
		{KDefineConstInt(SIGKILL)},
		/*for System.setpriority*/
		{KDefineConstInt(PRIO_PROCESS)},
		{KDefineConstInt(PRIO_PGRP)},
		{KDefineConstInt(PRIO_USER)},
		{}
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(intData), trace);
	return true;
}

static kbool_t process_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *process_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("process", "1.0"),
		KPACKLIB("POSIX.1", "1.0"),
		.PackupNameSpace    = process_PackupNameSpace,
		.ExportNameSpace   = process_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
