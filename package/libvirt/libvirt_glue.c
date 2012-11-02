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
#include <minikonoha/konoha_common.h>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kLibvirt {
	KonohaObjectHeader h;
	virConnectPtr conn;
} kLibvirt;

typedef struct kDomain {
	KonohaObjectHeader h;
	virDomainPtr domain;
} kDomain;

static void Domain_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kDomain *domainObject = (kDomain *)o;
	domainObject->domain = NULL;
}

static void Domain_free(KonohaContext *kctx, kObject *o)
{
	(void)kctx;(void)o; //TODO
}

static void Libvirt_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kLibvirt *libvirt = (kLibvirt *)o;
	libvirt->conn = NULL;
}

static void Libvirt_free(KonohaContext *kctx, kObject *o)
{
	(void)kctx;(void)o; //TODO
}

static KMETHOD Libvirt_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kLibvirt* libvirt = (kLibvirt *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	const char* uri = S_text(sfp[1].asString);
	libvirt->conn = virConnectOpen(uri);
	KReturn(libvirt);
}

// ["virDomainCreate", "int", "virDomainPtr"]
// int Domain.boot()
static KMETHOD Domain_Boot(KonohaContext *kctx,  KonohaStack *sfp)
{
	kDomain* domainObject = (kDomain *)(sfp[0].asObject);
	int ret = virDomainCreate(domainObject->domain);
	KReturnUnboxValue(ret);
}

// ["virDomainShutdown", "int", "virDomainPtr"]
// int Domain.shutdown()
static KMETHOD Domain_Shutdown(KonohaContext *kctx,  KonohaStack *sfp)
{
	kDomain* domainObject = (kDomain *)(sfp[0].asObject);
	int ret = virDomainShutdown(domainObject->domain);
	KReturnUnboxValue(ret);
}

// ["virDomainReboot", "int", "virDomainPtr", "unsigned int"]
// int Domain.reboot(int flag)
static KMETHOD Domain_Reboot(KonohaContext *kctx,  KonohaStack *sfp)
{
	kDomain* domainObject = (kDomain *)(sfp[0].asObject);
	unsigned int flag = sfp[1].unboxValue;
	int ret = virDomainReboot(domainObject->domain, flag);
	KReturnUnboxValue(ret);
}

// ["virDomainLookupByName", "virDomainPtr", "virConnectPtr", "const char *"]
// Domain Libvirt.lookUpDomain(String name)
static KMETHOD Libvirt_DomainLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	kLibvirt *libvirt = (kLibvirt *)sfp[0].asObject;
	const char *name = S_text(sfp[1].asString);
	virDomainPtr domain = virDomainLookupByName(libvirt->conn, name);
	kDomain* returnValue = (kDomain *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	returnValue->domain = domain;
	KReturn(returnValue);
}

// ["virDomainLookupByID", "virDomainPtr", "virConnectPtr", "int"]
// Domain Libvirt.lookUpDomain(int id)
static KMETHOD Libvirt_DomainLookupByID(KonohaContext *kctx,  KonohaStack *sfp)
{
	kLibvirt *libvirt = (kLibvirt *)sfp[0].asObject;
	int id = sfp[1].intValue;
	virDomainPtr domain = virDomainLookupByID(libvirt->conn, id);
	kDomain* returnValue = (kDomain *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	returnValue->domain = domain;
	KReturn(returnValue);
}

// ["virConnectListDomains", "int", "virConnectPtr", "int *", "int"]
// int[] Libvirt.listDomainIDs()
static KMETHOD Libvirt_listDomainIDs(KonohaContext *kctx,  KonohaStack *sfp)
{
	INIT_GCSTACK();
	kLibvirt *libvirt = (kLibvirt *)sfp[0].asObject;
	const int ids_max = 16; //FIXME
	int *ids = ALLOCA(int, ids_max);
	int ids_size = virConnectListDomains(libvirt->conn, ids, ids_max);
	if(ids_size < 0 ) {
		(void)ids_size; //TODO ERROR handling
	}
	KonohaClass *CT_ArrayInt = CT_p0(kctx, CT_Array, TY_int);
	kArrayVar *returnValue = (kArrayVar *)KLIB new_kObject(kctx, _GcStack, CT_ArrayInt, ids_size);
	int i;
	for(i = 0; i < ids_size; i++) {
		returnValue->unboxItems[i] = ids[i];
	}
	kArray_setsize(returnValue, ids_size);
	KReturnWith(returnValue, RESET_GCSTACK());
}

/* You can attach the following annotations to each methods. */
#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _Static   kMethod_Static

#define _F(F)     (intptr_t)(F)

static kbool_t LibVirt_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	/* Class Definition */
	/* class Libvirt */
	KDEFINE_CLASS defLibvirt = {0};
	SETSTRUCTNAME(defLibvirt, Libvirt);
	defLibvirt.cflag     = kClass_Final;
	defLibvirt.init      = Libvirt_init;
	//defLibvirt.p         = Libvirt_p;
	//defLibvirt.reftrace  = Libvirt_reftrace;
	defLibvirt.free      = Libvirt_free;
	KonohaClass *LibVirtClass = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defLibvirt, trace);

	/* class Domain */
	KDEFINE_CLASS defDomain = {0};
	SETSTRUCTNAME(defDomain, Domain);
	defDomain.cflag     = kClass_Final;
	defDomain.init      = Domain_init;
	//defDomain.p         = Domain_p;
	//defDomain.reftrace  = Domain_reftrace;
	defDomain.free      = Domain_free;
	KonohaClass *DomainClass = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defDomain, trace);

	KonohaClass *CT_ArrayInt = CT_p0(kctx, CT_Array, TY_int);
	ktype_t TY_ArrayInt = CT_ArrayInt->typeId;

	/* You can define methods with the following procedures. */
	int TY_Libvirt = LibVirtClass->typeId;
	int TY_Domain  = DomainClass->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Libvirt_new), TY_Libvirt, TY_Libvirt, MN_("new"), 1, TY_String, FN_("uri"),
		_Public, _F(Libvirt_listDomainIDs), TY_ArrayInt, TY_Libvirt, MN_("listDomainIDs"), 0,
		_Public, _F(Libvirt_DomainLookupByID), TY_Domain, TY_Libvirt, MN_("lookUpDomain"), 1, TY_int, FN_("id"),
		_Public, _F(Libvirt_DomainLookupByName), TY_Domain, TY_Libvirt, MN_("lookUpDomain"), 1, TY_String, FN_("name"),
		_Public, _F(Domain_Reboot), TY_int, TY_Domain, MN_("reboot"), 1, TY_int, FN_("flag"),
		_Public, _F(Domain_Shutdown), TY_int, TY_Domain, MN_("shutdown"), 0,
		_Public, _F(Domain_Boot), TY_int, TY_Domain, MN_("boot"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	/* You can define constant variable with the following procedures. */
	KDEFINE_INT_CONST IntData[] = {
		{"VIR_DOMAIN_REBOOT_DEFAULT", TY_int, VIR_DOMAIN_REBOOT_DEFAULT},
		{"VIR_DOMAIN_REBOOT_ACPI_POWER_BTN", TY_int, VIR_DOMAIN_REBOOT_ACPI_POWER_BTN},
		{"VIR_DOMAIN_REBOOT_GUEST_AGENT", TY_int, VIR_DOMAIN_REBOOT_GUEST_AGENT},
		{} /* <= sentinel */
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), trace);
	return true;
}

static kbool_t LibVirt_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* libvirt_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "libvirt", "0.1");
	d.initPackage    = LibVirt_initPackage;
	d.setupPackage   = LibVirt_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
