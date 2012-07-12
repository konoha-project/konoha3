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

#include<konoha2/konoha2.h>
#include<konoha2/sugar.h>
#include <unistd.h>
#include <signal.h>

//## @Static @Public Int System.getpid();

static KMETHOD System_getpid(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(getpid());
}

static KMETHOD System_getppid(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(getppid());
}

static KMETHOD System_getuid(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(getuid());
}

static KMETHOD System_geteuid(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(geteuid());
}

static KMETHOD System_getgid(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(getgid());
}

static KMETHOD System_getegid(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(getegid());
}

static KMETHOD System_getpgid(CTX, ksfp_t *sfp _RIX)
{
	int pid = sfp[1].ivalue;
	int ret = getpgid(pid);
	RETURNi_(ret);
}

static KMETHOD System_setpgid(CTX, ksfp_t *sfp _RIX)
{
	int pid = sfp[1].ivalue;
	int pgid = sfp[2].ivalue;
	int ret = setpgid(pid, pgid);
	RETURNi_(ret);
}

static KMETHOD System_chdir(CTX, ksfp_t *sfp _RIX)
{
	kString *s = sfp[1].s;
	const char *dir = S_text(s);
	int ret = chdir(dir);
	RETURNi_(ret);
}

static KMETHOD System_fchdir(CTX, ksfp_t *sfp _RIX)
{
	int ch = fchdir(sfp[1].ivalue);
	RETURNi_(ch);
}

static KMETHOD System_chroot(CTX, ksfp_t *sfp _RIX)
{
	kString *s = sfp[1].s;
	const char *root = S_text(s);
	int ret = chroot(root);
	RETURNi_(ret);
}

static KMETHOD System_getpriority(CTX, ksfp_t *sfp _RIX)
{

	int which = sfp[1].ivalue;
	int who = sfp[2].ivalue;
	int ret = getpriority(which, who);
	RETURNi_(ret);
}

static KMETHOD System_setpriority(CTX, ksfp_t *sfp _RIX)
{
	int which = sfp[1].ivalue;
	int who = sfp[2].ivalue;
	int priority = sfp[3].ivalue;
	int ret = setpriority(which, who, priority);
	RETURNi_(ret);
}

static KMETHOD System_getgroups(CTX, ksfp_t *sfp _RIX)
{
	int size = kArray_size(sfp[2].a);
	kArray *list = sfp[2].a;
	int ret = getgroups(size, list);
	RETURNi_(ret);
}

static KMETHOD System_setgroups(CTX, ksfp_t *sfp _RIX)
{
	int size = kArray_size(sfp[2].a);
	kArray *list = sfp[2].a;
	int ret = setgroups(size, list);
	RETURNi_(ret);
}

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Static   kMethod_Static
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

#define _KVi(T) #T, TY_Int, T

static	kbool_t process_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(System_getpid), TY_Int, TY_System, MN_("getpid"), 0,
		_Public|_Static, _F(System_getppid), TY_Int, TY_System, MN_("getppid"), 0,
		_Public|_Static, _F(System_getuid), TY_Int, TY_System, MN_("getuid"), 0,
		_Public|_Static, _F(System_geteuid), TY_Int, TY_System, MN_("geteuid"), 0,
		_Public|_Static, _F(System_getgid), TY_Int, TY_System, MN_("getgid"), 0,
		_Public|_Static, _F(System_getegid), TY_Int, TY_System, MN_("getegid"), 0,
		_Public|_Static, _F(System_getpgid), TY_Int, TY_System, MN_("getpgid"), 1, TY_Int, FN_("pid"),
		_Public|_Static, _F(System_setpgid), TY_Int, TY_System, MN_("setpgid"), 2, TY_Int, FN_("pid"), TY_Int, FN_("pgid"),
		_Public|_Static, _F(System_chdir), TY_Int, TY_System, MN_("chdir"), 1, TY_String, FN_("pathname"),
		_Public|_Static, _F(System_fchdir), TY_Int, TY_System, MN_("fchdir"), 1, TY_Int, FN_("fd"),
		_Public|_Static, _F(System_chroot), TY_Int, TY_System, MN_("chroot"), 1, TY_String, FN_("pathname"),
		_Public|_Static, _F(System_getpriority), TY_Int, TY_System, MN_("getpriority"), 2, TY_Int, FN_("which"), TY_Int, FN_("who"),
		_Public|_Static, _F(System_setpriority), TY_Int, TY_System, MN_("setpriority"), 3, TY_Int, FN_("which"), TY_Int, FN_("who"), TY_Int, FN_("priority"),
		_Public|_Static, _F(System_getgroups), TY_Int, TY_System, MN_("getgroups"), 2, TY_Int, FN_("size"), TY_Array, FN_("list[]"),
		_Public|_Static, _F(System_setgroups), TY_Int, TY_System, MN_("setgroups"), 2, TY_Int, FN_("size"), TY_Array, FN_("*list"),
		DEND,
	};
	kNameSpace_loadMethodData(ks, MethodData);
	KDEFINE_INT_CONST IntData[] = {
		{_KVi(SIGHUP)},
		{_KVi(SIGINT)},
		{_KVi(SIGABRT)},
		{_KVi(SIGKILL)},
		{}
	};
	kNameSpace_loadConstData(ks, IntData, 0);
	return true;
}

static kbool_t process_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t process_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t process_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* process_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("package.posix", "1.0"),
		KPACKLIB("POSIX.1", "1.0"),
		.initPackage = process_initPackage,
		.setupPackage = process_setupPackage,
		.initNameSpace = process_initNameSpace,
		.setupNameSpace = process_setupNameSpace,
	};
	return &d;
}

