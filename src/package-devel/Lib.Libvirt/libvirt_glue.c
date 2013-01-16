#include "libvirt_glue.h"

// ["virDomainGetSchedulerParameters", "int", "virDomainPtr", "virTypedParameterPtr", "int *"]
static KMETHOD KvirDomainGetSchedulerParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	intPtr arg2 = To_intPtr(sfp[2]);
	int ret = virDomainGetSchedulerParameters (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainGetSchedulerParametersFlags", "int", "virDomainPtr", "virTypedParameterPtr", "int *", "unsigned int"]
static KMETHOD KvirDomainGetSchedulerParametersFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	intPtr arg2 = To_intPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetSchedulerParametersFlags (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainSetSchedulerParameters", "int", "virDomainPtr", "virTypedParameterPtr", "int"]
static KMETHOD KvirDomainSetSchedulerParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virDomainSetSchedulerParameters (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainSetSchedulerParametersFlags", "int", "virDomainPtr", "virTypedParameterPtr", "int", "unsigned int"]
static KMETHOD KvirDomainSetSchedulerParametersFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainSetSchedulerParametersFlags (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainMigrate", "virDomainPtr", "virDomainPtr", "virConnectPtr", "unsigned long", "const char *", "const char *", "unsigned long"]
static KMETHOD KvirDomainMigrate(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virConnectPtr arg1 = To_virConnectPtr(sfp[1]);
	unsigned_long arg2 = To_unsigned_long(sfp[2]);
	charPtr arg3 = To_charPtr(sfp[3]);
	charPtr arg4 = To_charPtr(sfp[4]);
	unsigned_long arg5 = To_unsigned_long(sfp[5]);
	virDomainPtr ret = virDomainMigrate (arg0, arg1, arg2, arg3, arg4, arg5);
	RETURNvirDomainPtr(ret);
}

// ["virDomainMigrate2", "virDomainPtr", "virDomainPtr", "virConnectPtr", "const char *", "unsigned long", "const char *", "const char *", "unsigned long"]
static KMETHOD KvirDomainMigrate2(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virConnectPtr arg1 = To_virConnectPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_long arg3 = To_unsigned_long(sfp[3]);
	charPtr arg4 = To_charPtr(sfp[4]);
	charPtr arg5 = To_charPtr(sfp[5]);
	unsigned_long arg6 = To_unsigned_long(sfp[6]);
	virDomainPtr ret = virDomainMigrate2 (arg0, arg1, arg2, arg3, arg4, arg5, arg6);
	RETURNvirDomainPtr(ret);
}

// ["virDomainMigrateToURI", "int", "virDomainPtr", "const char *", "unsigned long", "const char *", "unsigned long"]
static KMETHOD KvirDomainMigrateToURI(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_long arg2 = To_unsigned_long(sfp[2]);
	charPtr arg3 = To_charPtr(sfp[3]);
	unsigned_long arg4 = To_unsigned_long(sfp[4]);
	int ret = virDomainMigrateToURI (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainMigrateToURI2", "int", "virDomainPtr", "const char *", "const char *", "const char *", "unsigned long", "const char *", "unsigned long"]
static KMETHOD KvirDomainMigrateToURI2(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	charPtr arg3 = To_charPtr(sfp[3]);
	unsigned_long arg4 = To_unsigned_long(sfp[4]);
	charPtr arg5 = To_charPtr(sfp[5]);
	unsigned_long arg6 = To_unsigned_long(sfp[6]);
	int ret = virDomainMigrateToURI2 (arg0, arg1, arg2, arg3, arg4, arg5, arg6);
	RETURNint(ret);
}

// ["virDomainMigrateSetMaxDowntime", "int", "virDomainPtr", "unsigned long long", "unsigned int"]
static KMETHOD KvirDomainMigrateSetMaxDowntime(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_long_long arg1 = To_unsigned_long_long(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainMigrateSetMaxDowntime (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainMigrateSetMaxSpeed", "int", "virDomainPtr", "unsigned long", "unsigned int"]
static KMETHOD KvirDomainMigrateSetMaxSpeed(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_long arg1 = To_unsigned_long(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainMigrateSetMaxSpeed (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainMigrateGetMaxSpeed", "int", "virDomainPtr", "unsigned long *", "unsigned int"]
static KMETHOD KvirDomainMigrateGetMaxSpeed(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_long_Ptr arg1 = To_unsigned_long_Ptr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainMigrateGetMaxSpeed (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virGetVersion", "int", "unsigned long *", "const char *", "unsigned long *"]
static KMETHOD KvirGetVersion(KonohaContext *kctx,  KonohaStack *sfp)
{
	unsigned_long_Ptr arg0 = To_unsigned_long_Ptr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_long_Ptr arg2 = To_unsigned_long_Ptr(sfp[2]);
	int ret = virGetVersion (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virInitialize", "int"]
static KMETHOD KvirInitialize(KonohaContext *kctx,  KonohaStack *sfp)
{
	int ret = virInitialize ();
	RETURNint(ret);
}

// ["virConnectOpen", "virConnectPtr", "const char *"]
static KMETHOD KvirConnectOpen(KonohaContext *kctx,  KonohaStack *sfp)
{
	charPtr arg0 = To_charPtr(sfp[0]);
	virConnectPtr ret = virConnectOpen (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virConnectOpenReadOnly", "virConnectPtr", "const char *"]
static KMETHOD KvirConnectOpenReadOnly(KonohaContext *kctx,  KonohaStack *sfp)
{
	charPtr arg0 = To_charPtr(sfp[0]);
	virConnectPtr ret = virConnectOpenReadOnly (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virConnectOpenAuth", "virConnectPtr", "const char *", "virConnectAuthPtr", "unsigned int"]
static KMETHOD KvirConnectOpenAuth(KonohaContext *kctx,  KonohaStack *sfp)
{
	charPtr arg0 = To_charPtr(sfp[0]);
	virConnectAuthPtr arg1 = To_virConnectAuthPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virConnectPtr ret = virConnectOpenAuth (arg0, arg1, arg2);
	RETURNvirConnectPtr(ret);
}

// ["virConnectRef", "int", "virConnectPtr"]
static KMETHOD KvirConnectRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectRef (arg0);
	RETURNint(ret);
}

// ["virConnectClose", "int", "virConnectPtr"]
static KMETHOD KvirConnectClose(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectClose (arg0);
	RETURNint(ret);
}

// ["virConnectGetType", "const char *", "virConnectPtr"]
static KMETHOD KvirConnectGetType(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr ret = virConnectGetType (arg0);
	RETURNcharPtr(ret);
}

// ["virConnectGetVersion", "int", "virConnectPtr", "unsigned long *"]
static KMETHOD KvirConnectGetVersion(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_long_Ptr arg1 = To_unsigned_long_Ptr(sfp[1]);
	int ret = virConnectGetVersion (arg0, arg1);
	RETURNint(ret);
}

// ["virConnectGetLibVersion", "int", "virConnectPtr", "unsigned long *"]
static KMETHOD KvirConnectGetLibVersion(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_long_Ptr arg1 = To_unsigned_long_Ptr(sfp[1]);
	int ret = virConnectGetLibVersion (arg0, arg1);
	RETURNint(ret);
}

// ["virConnectGetHostname", "char *", "virConnectPtr"]
static KMETHOD KvirConnectGetHostname(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr ret = virConnectGetHostname (arg0);
	RETURNcharPtr(ret);
}

// ["virConnectGetURI", "char *", "virConnectPtr"]
static KMETHOD KvirConnectGetURI(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr ret = virConnectGetURI (arg0);
	RETURNcharPtr(ret);
}

// ["virConnectGetSysinfo", "char *", "virConnectPtr", "unsigned int"]
static KMETHOD KvirConnectGetSysinfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virConnectGetSysinfo (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virConnectSetKeepAlive", "int", "virConnectPtr", "int", "unsigned int"]
static KMETHOD KvirConnectSetKeepAlive(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectSetKeepAlive (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectRegisterCloseCallback", "int", "virConnectPtr", "virConnectCloseFunc", "void *", "virFreeCallback"]
static KMETHOD KvirConnectRegisterCloseCallback(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virConnectCloseFunc arg1 = To_virConnectCloseFunc(sfp[1]);
	voidPtr arg2 = To_voidPtr(sfp[2]);
	virFreeCallback arg3 = To_virFreeCallback(sfp[3]);
	int ret = virConnectRegisterCloseCallback (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virConnectUnregisterCloseCallback", "int", "virConnectPtr", "virConnectCloseFunc"]
static KMETHOD KvirConnectUnregisterCloseCallback(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virConnectCloseFunc arg1 = To_virConnectCloseFunc(sfp[1]);
	int ret = virConnectUnregisterCloseCallback (arg0, arg1);
	RETURNint(ret);
}

// ["virConnectGetMaxVcpus", "int", "virConnectPtr", "const char *"]
static KMETHOD KvirConnectGetMaxVcpus(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virConnectGetMaxVcpus (arg0, arg1);
	RETURNint(ret);
}

// ["virNodeGetInfo", "int", "virConnectPtr", "virNodeInfoPtr"]
static KMETHOD KvirNodeGetInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virNodeInfoPtr arg1 = To_virNodeInfoPtr(sfp[1]);
	int ret = virNodeGetInfo (arg0, arg1);
	RETURNint(ret);
}

// ["virConnectGetCapabilities", "char *", "virConnectPtr"]
static KMETHOD KvirConnectGetCapabilities(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr ret = virConnectGetCapabilities (arg0);
	RETURNcharPtr(ret);
}

// ["virNodeGetCPUStats", "int", "virConnectPtr", "int", "virNodeCPUStatsPtr", "int *", "unsigned int"]
static KMETHOD KvirNodeGetCPUStats(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	virNodeCPUStatsPtr arg2 = To_virNodeCPUStatsPtr(sfp[2]);
	intPtr arg3 = To_intPtr(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virNodeGetCPUStats (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virNodeGetMemoryStats", "int", "virConnectPtr", "int", "virNodeMemoryStatsPtr", "int *", "unsigned int"]
static KMETHOD KvirNodeGetMemoryStats(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	virNodeMemoryStatsPtr arg2 = To_virNodeMemoryStatsPtr(sfp[2]);
	intPtr arg3 = To_intPtr(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virNodeGetMemoryStats (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virNodeGetFreeMemory", "unsigned long long", "virConnectPtr"]
static KMETHOD KvirNodeGetFreeMemory(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_long_long ret = virNodeGetFreeMemory (arg0);
	RETURNunsigned_long_long(ret);
}

// ["virNodeGetSecurityModel", "int", "virConnectPtr", "virSecurityModelPtr"]
static KMETHOD KvirNodeGetSecurityModel(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virSecurityModelPtr arg1 = To_virSecurityModelPtr(sfp[1]);
	int ret = virNodeGetSecurityModel (arg0, arg1);
	RETURNint(ret);
}

// ["virNodeSuspendForDuration", "int", "virConnectPtr", "unsigned int", "unsigned long long", "unsigned int"]
static KMETHOD KvirNodeSuspendForDuration(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	unsigned_long_long arg2 = To_unsigned_long_long(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virNodeSuspendForDuration (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virConnectListDomains", "int", "virConnectPtr", "int *", "int"]
static KMETHOD KvirConnectListDomains(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	intPtr arg1 = To_intPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListDomains (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectNumOfDomains", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfDomains(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfDomains (arg0);
	RETURNint(ret);
}

// ["virDomainGetConnect", "virConnectPtr", "virDomainPtr"]
static KMETHOD KvirDomainGetConnect(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virConnectPtr ret = virDomainGetConnect (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virDomainCreateXML", "virDomainPtr", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainCreateXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virDomainPtr ret = virDomainCreateXML (arg0, arg1, arg2);
	RETURNvirDomainPtr(ret);
}

// ["virDomainLookupByName", "virDomainPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirDomainLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virDomainPtr ret = virDomainLookupByName (arg0, arg1);
	RETURNvirDomainPtr(ret);
}

// ["virDomainLookupByID", "virDomainPtr", "virConnectPtr", "int"]
static KMETHOD KvirDomainLookupByID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	virDomainPtr ret = virDomainLookupByID (arg0, arg1);
	RETURNvirDomainPtr(ret);
}

// ["virDomainLookupByUUID", "virDomainPtr", "virConnectPtr", "const unsigned char *"]
static KMETHOD KvirDomainLookupByUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	virDomainPtr ret = virDomainLookupByUUID (arg0, arg1);
	RETURNvirDomainPtr(ret);
}

// ["virDomainLookupByUUIDString", "virDomainPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirDomainLookupByUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virDomainPtr ret = virDomainLookupByUUIDString (arg0, arg1);
	RETURNvirDomainPtr(ret);
}

// ["virDomainShutdown", "int", "virDomainPtr"]
static KMETHOD KvirDomainShutdown(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainShutdown (arg0);
	RETURNint(ret);
}

// ["virDomainShutdownFlags", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainShutdownFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainShutdownFlags (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainReboot", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainReboot(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainReboot (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainReset", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainReset(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainReset (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainDestroy", "int", "virDomainPtr"]
static KMETHOD KvirDomainDestroy(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainDestroy (arg0);
	RETURNint(ret);
}

// ["virDomainDestroyFlags", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainDestroyFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainDestroyFlags (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainRef", "int", "virDomainPtr"]
static KMETHOD KvirDomainRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainRef (arg0);
	RETURNint(ret);
}

// ["virDomainFree", "int", "virDomainPtr"]
static KMETHOD KvirDomainFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainFree (arg0);
	RETURNint(ret);
}

// ["virDomainSuspend", "int", "virDomainPtr"]
static KMETHOD KvirDomainSuspend(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainSuspend (arg0);
	RETURNint(ret);
}

// ["virDomainResume", "int", "virDomainPtr"]
static KMETHOD KvirDomainResume(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainResume (arg0);
	RETURNint(ret);
}

// ["virDomainPMSuspendForDuration", "int", "virDomainPtr", "unsigned int", "unsigned long long", "unsigned int"]
static KMETHOD KvirDomainPMSuspendForDuration(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	unsigned_long_long arg2 = To_unsigned_long_long(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainPMSuspendForDuration (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainPMWakeup", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainPMWakeup(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainPMWakeup (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSave", "int", "virDomainPtr", "const char *"]
static KMETHOD KvirDomainSave(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virDomainSave (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSaveFlags", "int", "virDomainPtr", "const char *", "const char *", "unsigned int"]
static KMETHOD KvirDomainSaveFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainSaveFlags (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainRestore", "int", "virConnectPtr", "const char *"]
static KMETHOD KvirDomainRestore(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virDomainRestore (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainRestoreFlags", "int", "virConnectPtr", "const char *", "const char *", "unsigned int"]
static KMETHOD KvirDomainRestoreFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainRestoreFlags (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainSaveImageGetXMLDesc", "char *", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainSaveImageGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	charPtr ret = virDomainSaveImageGetXMLDesc (arg0, arg1, arg2);
	RETURNcharPtr(ret);
}

// ["virDomainSaveImageDefineXML", "int", "virConnectPtr", "const char *", "const char *", "unsigned int"]
static KMETHOD KvirDomainSaveImageDefineXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainSaveImageDefineXML (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainManagedSave", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainManagedSave(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainManagedSave (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainHasManagedSaveImage", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainHasManagedSaveImage(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainHasManagedSaveImage (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainManagedSaveRemove", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainManagedSaveRemove(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainManagedSaveRemove (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainCoreDump", "int", "virDomainPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainCoreDump(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainCoreDump (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainScreenshot", "char *", "virDomainPtr", "virStreamPtr", "unsigned int", "unsigned int"]
static KMETHOD KvirDomainScreenshot(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virStreamPtr arg1 = To_virStreamPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	charPtr ret = virDomainScreenshot (arg0, arg1, arg2, arg3);
	RETURNcharPtr(ret);
}

// ["virDomainGetInfo", "int", "virDomainPtr", "virDomainInfoPtr"]
static KMETHOD KvirDomainGetInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virDomainInfoPtr arg1 = To_virDomainInfoPtr(sfp[1]);
	int ret = virDomainGetInfo (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainGetState", "int", "virDomainPtr", "int *", "int *", "unsigned int"]
static KMETHOD KvirDomainGetState(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	intPtr arg1 = To_intPtr(sfp[1]);
	intPtr arg2 = To_intPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetState (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainGetCPUStats", "int", "virDomainPtr", "virTypedParameterPtr", "unsigned int", "int", "unsigned int", "unsigned int"]
static KMETHOD KvirDomainGetCPUStats(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int arg3 = To_int(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	unsigned_int arg5 = To_unsigned_int(sfp[5]);
	int ret = virDomainGetCPUStats (arg0, arg1, arg2, arg3, arg4, arg5);
	RETURNint(ret);
}

// ["virDomainGetControlInfo", "int", "virDomainPtr", "virDomainControlInfoPtr", "unsigned int"]
static KMETHOD KvirDomainGetControlInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virDomainControlInfoPtr arg1 = To_virDomainControlInfoPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainGetControlInfo (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainGetSchedulerType", "char *", "virDomainPtr", "int *"]
static KMETHOD KvirDomainGetSchedulerType(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	intPtr arg1 = To_intPtr(sfp[1]);
	charPtr ret = virDomainGetSchedulerType (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virDomainSetBlkioParameters", "int", "virDomainPtr", "virTypedParameterPtr", "int", "unsigned int"]
static KMETHOD KvirDomainSetBlkioParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainSetBlkioParameters (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainGetBlkioParameters", "int", "virDomainPtr", "virTypedParameterPtr", "int *", "unsigned int"]
static KMETHOD KvirDomainGetBlkioParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	intPtr arg2 = To_intPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetBlkioParameters (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainSetMemoryParameters", "int", "virDomainPtr", "virTypedParameterPtr", "int", "unsigned int"]
static KMETHOD KvirDomainSetMemoryParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainSetMemoryParameters (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainGetMemoryParameters", "int", "virDomainPtr", "virTypedParameterPtr", "int *", "unsigned int"]
static KMETHOD KvirDomainGetMemoryParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	intPtr arg2 = To_intPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetMemoryParameters (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainSetNumaParameters", "int", "virDomainPtr", "virTypedParameterPtr", "int", "unsigned int"]
static KMETHOD KvirDomainSetNumaParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainSetNumaParameters (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainGetNumaParameters", "int", "virDomainPtr", "virTypedParameterPtr", "int *", "unsigned int"]
static KMETHOD KvirDomainGetNumaParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	intPtr arg2 = To_intPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetNumaParameters (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainGetName", "const char *", "virDomainPtr"]
static KMETHOD KvirDomainGetName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr ret = virDomainGetName (arg0);
	RETURNcharPtr(ret);
}

// ["virDomainGetID", "unsigned int", "virDomainPtr"]
static KMETHOD KvirDomainGetID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int ret = virDomainGetID (arg0);
	RETURNunsigned_int(ret);
}

// ["virDomainGetUUID", "int", "virDomainPtr", "unsigned char *"]
static KMETHOD KvirDomainGetUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	int ret = virDomainGetUUID (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainGetUUIDString", "int", "virDomainPtr", "char *"]
static KMETHOD KvirDomainGetUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virDomainGetUUIDString (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainGetOSType", "char *", "virDomainPtr"]
static KMETHOD KvirDomainGetOSType(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr ret = virDomainGetOSType (arg0);
	RETURNcharPtr(ret);
}

// ["virDomainGetMaxMemory", "unsigned long", "virDomainPtr"]
static KMETHOD KvirDomainGetMaxMemory(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_long ret = virDomainGetMaxMemory (arg0);
	RETURNunsigned_long(ret);
}

// ["virDomainSetMaxMemory", "int", "virDomainPtr", "unsigned long"]
static KMETHOD KvirDomainSetMaxMemory(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_long arg1 = To_unsigned_long(sfp[1]);
	int ret = virDomainSetMaxMemory (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSetMemory", "int", "virDomainPtr", "unsigned long"]
static KMETHOD KvirDomainSetMemory(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_long arg1 = To_unsigned_long(sfp[1]);
	int ret = virDomainSetMemory (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSetMemoryFlags", "int", "virDomainPtr", "unsigned long", "unsigned int"]
static KMETHOD KvirDomainSetMemoryFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_long arg1 = To_unsigned_long(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainSetMemoryFlags (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainGetMaxVcpus", "int", "virDomainPtr"]
static KMETHOD KvirDomainGetMaxVcpus(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainGetMaxVcpus (arg0);
	RETURNint(ret);
}

// ["virDomainGetSecurityLabel", "int", "virDomainPtr", "virSecurityLabelPtr"]
static KMETHOD KvirDomainGetSecurityLabel(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virSecurityLabelPtr arg1 = To_virSecurityLabelPtr(sfp[1]);
	int ret = virDomainGetSecurityLabel (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainGetHostname", "char *", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainGetHostname(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virDomainGetHostname (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virDomainGetSecurityLabelList", "int", "virDomainPtr", "virSecurityLabelPtr *"]
static KMETHOD KvirDomainGetSecurityLabelList(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virSecurityLabelPtrPtr arg1 = To_virSecurityLabelPtrPtr(sfp[1]);
	int ret = virDomainGetSecurityLabelList (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSetMetadata", "int", "virDomainPtr", "int", "const char *", "const char *", "const char *", "unsigned int"]
static KMETHOD KvirDomainSetMetadata(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	charPtr arg3 = To_charPtr(sfp[3]);
	charPtr arg4 = To_charPtr(sfp[4]);
	unsigned_int arg5 = To_unsigned_int(sfp[5]);
	int ret = virDomainSetMetadata (arg0, arg1, arg2, arg3, arg4, arg5);
	RETURNint(ret);
}

// ["virDomainGetMetadata", "char *", "virDomainPtr", "int", "const char *", "unsigned int"]
static KMETHOD KvirDomainGetMetadata(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	charPtr ret = virDomainGetMetadata (arg0, arg1, arg2, arg3);
	RETURNcharPtr(ret);
}

// ["virDomainGetXMLDesc", "char *", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virDomainGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virConnectDomainXMLFromNative", "char *", "virConnectPtr", "const char *", "const char *", "unsigned int"]
static KMETHOD KvirConnectDomainXMLFromNative(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	charPtr ret = virConnectDomainXMLFromNative (arg0, arg1, arg2, arg3);
	RETURNcharPtr(ret);
}

// ["virConnectDomainXMLToNative", "char *", "virConnectPtr", "const char *", "const char *", "unsigned int"]
static KMETHOD KvirConnectDomainXMLToNative(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	charPtr ret = virConnectDomainXMLToNative (arg0, arg1, arg2, arg3);
	RETURNcharPtr(ret);
}

// ["virDomainNodeStats", "int", "virDomainPtr", "const char *", "virDomainNodeStatsPtr", "size_t"]
static KMETHOD KvirDomainNodeStats(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virDomainNodeStatsPtr arg2 = To_virDomainNodeStatsPtr(sfp[2]);
	size_t arg3 = To_size_t(sfp[3]);
	int ret = virDomainNodeStats (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainNodeStatsFlags", "int", "virDomainPtr", "const char *", "virTypedParameterPtr", "int *", "unsigned int"]
static KMETHOD KvirDomainNodeStatsFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virTypedParameterPtr arg2 = To_virTypedParameterPtr(sfp[2]);
	intPtr arg3 = To_intPtr(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainNodeStatsFlags (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainInterfaceStats", "int", "virDomainPtr", "const char *", "virDomainInterfaceStatsPtr", "size_t"]
static KMETHOD KvirDomainInterfaceStats(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virDomainInterfaceStatsPtr arg2 = To_virDomainInterfaceStatsPtr(sfp[2]);
	size_t arg3 = To_size_t(sfp[3]);
	int ret = virDomainInterfaceStats (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainSetInterfaceParameters", "int", "virDomainPtr", "const char *", "virTypedParameterPtr", "int", "unsigned int"]
static KMETHOD KvirDomainSetInterfaceParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virTypedParameterPtr arg2 = To_virTypedParameterPtr(sfp[2]);
	int arg3 = To_int(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainSetInterfaceParameters (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainGetInterfaceParameters", "int", "virDomainPtr", "const char *", "virTypedParameterPtr", "int *", "unsigned int"]
static KMETHOD KvirDomainGetInterfaceParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virTypedParameterPtr arg2 = To_virTypedParameterPtr(sfp[2]);
	intPtr arg3 = To_intPtr(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainGetInterfaceParameters (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainNodePeek", "int", "virDomainPtr", "const char *", "unsigned long long", "size_t", "void *", "unsigned int"]
static KMETHOD KvirDomainNodePeek(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_long_long arg2 = To_unsigned_long_long(sfp[2]);
	size_t arg3 = To_size_t(sfp[3]);
	voidPtr arg4 = To_voidPtr(sfp[4]);
	unsigned_int arg5 = To_unsigned_int(sfp[5]);
	int ret = virDomainNodePeek (arg0, arg1, arg2, arg3, arg4, arg5);
	RETURNint(ret);
}

// ["virDomainNodeResize", "int", "virDomainPtr", "const char *", "unsigned long long", "unsigned int"]
static KMETHOD KvirDomainNodeResize(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_long_long arg2 = To_unsigned_long_long(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainNodeResize (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainGetNodeInfo", "int", "virDomainPtr", "const char *", "virDomainNodeInfoPtr", "unsigned int"]
static KMETHOD KvirDomainGetNodeInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virDomainNodeInfoPtr arg2 = To_virDomainNodeInfoPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetNodeInfo (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainMemoryStats", "int", "virDomainPtr", "virDomainMemoryStatPtr", "unsigned int", "unsigned int"]
static KMETHOD KvirDomainMemoryStats(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virDomainMemoryStatPtr arg1 = To_virDomainMemoryStatPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainMemoryStats (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainMemoryPeek", "int", "virDomainPtr", "unsigned long long", "size_t", "void *", "unsigned int"]
static KMETHOD KvirDomainMemoryPeek(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_long_long arg1 = To_unsigned_long_long(sfp[1]);
	size_t arg2 = To_size_t(sfp[2]);
	voidPtr arg3 = To_voidPtr(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainMemoryPeek (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainDefineXML", "virDomainPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirDomainDefineXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virDomainPtr ret = virDomainDefineXML (arg0, arg1);
	RETURNvirDomainPtr(ret);
}

// ["virDomainUndefine", "int", "virDomainPtr"]
static KMETHOD KvirDomainUndefine(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainUndefine (arg0);
	RETURNint(ret);
}

// ["virDomainUndefineFlags", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainUndefineFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainUndefineFlags (arg0, arg1);
	RETURNint(ret);
}

// ["virConnectNumOfDefinedDomains", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfDefinedDomains(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfDefinedDomains (arg0);
	RETURNint(ret);
}

// ["virConnectListDefinedDomains", "int", "virConnectPtr", "char **const", "int"]
static KMETHOD KvirConnectListDefinedDomains(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListDefinedDomains (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectListAllDomains", "int", "virConnectPtr", "virDomainPtr **", "unsigned int"]
static KMETHOD KvirConnectListAllDomains(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virDomainPtrPtrPtr arg1 = To_virDomainPtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectListAllDomains (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainCreate", "int", "virDomainPtr"]
static KMETHOD KvirDomainCreate(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainCreate (arg0);
	RETURNint(ret);
}

// ["virDomainCreateWithFlags", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainCreateWithFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainCreateWithFlags (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainGetAutostart", "int", "virDomainPtr", "int *"]
static KMETHOD KvirDomainGetAutostart(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	intPtr arg1 = To_intPtr(sfp[1]);
	int ret = virDomainGetAutostart (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSetAutostart", "int", "virDomainPtr", "int"]
static KMETHOD KvirDomainSetAutostart(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	int ret = virDomainSetAutostart (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSetVcpus", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainSetVcpus(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainSetVcpus (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSetVcpusFlags", "int", "virDomainPtr", "unsigned int", "unsigned int"]
static KMETHOD KvirDomainSetVcpusFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainSetVcpusFlags (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainGetVcpusFlags", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainGetVcpusFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainGetVcpusFlags (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainPinVcpu", "int", "virDomainPtr", "unsigned int", "unsigned char *", "int"]
static KMETHOD KvirDomainPinVcpu(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	unsigned_charPtr arg2 = To_unsigned_charPtr(sfp[2]);
	int arg3 = To_int(sfp[3]);
	int ret = virDomainPinVcpu (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainPinVcpuFlags", "int", "virDomainPtr", "unsigned int", "unsigned char *", "int", "unsigned int"]
static KMETHOD KvirDomainPinVcpuFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	unsigned_charPtr arg2 = To_unsigned_charPtr(sfp[2]);
	int arg3 = To_int(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainPinVcpuFlags (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainGetVcpuPinInfo", "int", "virDomainPtr", "int", "unsigned char *", "int", "unsigned int"]
static KMETHOD KvirDomainGetVcpuPinInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	unsigned_charPtr arg2 = To_unsigned_charPtr(sfp[2]);
	int arg3 = To_int(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainGetVcpuPinInfo (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainPinEmulator", "int", "virDomainPtr", "unsigned char *", "int", "unsigned int"]
static KMETHOD KvirDomainPinEmulator(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainPinEmulator (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainGetEmulatorPinInfo", "int", "virDomainPtr", "unsigned char *", "int", "unsigned int"]
static KMETHOD KvirDomainGetEmulatorPinInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetEmulatorPinInfo (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainGetVcpus", "int", "virDomainPtr", "virVcpuInfoPtr", "int", "unsigned char *", "int"]
static KMETHOD KvirDomainGetVcpus(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virVcpuInfoPtr arg1 = To_virVcpuInfoPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_charPtr arg3 = To_unsigned_charPtr(sfp[3]);
	int arg4 = To_int(sfp[4]);
	int ret = virDomainGetVcpus (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainAttachDevice", "int", "virDomainPtr", "const char *"]
static KMETHOD KvirDomainAttachDevice(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virDomainAttachDevice (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainDetachDevice", "int", "virDomainPtr", "const char *"]
static KMETHOD KvirDomainDetachDevice(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virDomainDetachDevice (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainAttachDeviceFlags", "int", "virDomainPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainAttachDeviceFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainAttachDeviceFlags (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainDetachDeviceFlags", "int", "virDomainPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainDetachDeviceFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainDetachDeviceFlags (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainUpdateDeviceFlags", "int", "virDomainPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainUpdateDeviceFlags(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainUpdateDeviceFlags (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainNodeJobAbort", "int", "virDomainPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainNodeJobAbort(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainNodeJobAbort (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainGetNodeJobInfo", "int", "virDomainPtr", "const char *", "virDomainNodeJobInfoPtr", "unsigned int"]
static KMETHOD KvirDomainGetNodeJobInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virDomainNodeJobInfoPtr arg2 = To_virDomainNodeJobInfoPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetNodeJobInfo (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainNodeJobSetSpeed", "int", "virDomainPtr", "const char *", "unsigned long", "unsigned int"]
static KMETHOD KvirDomainNodeJobSetSpeed(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_long arg2 = To_unsigned_long(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainNodeJobSetSpeed (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainNodePull", "int", "virDomainPtr", "const char *", "unsigned long", "unsigned int"]
static KMETHOD KvirDomainNodePull(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_long arg2 = To_unsigned_long(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainNodePull (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainNodeRebase", "int", "virDomainPtr", "const char *", "const char *", "unsigned long", "unsigned int"]
static KMETHOD KvirDomainNodeRebase(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_long arg3 = To_unsigned_long(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainNodeRebase (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainNodeCommit", "int", "virDomainPtr", "const char *", "const char *", "const char *", "unsigned long", "unsigned int"]
static KMETHOD KvirDomainNodeCommit(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	charPtr arg3 = To_charPtr(sfp[3]);
	unsigned_long arg4 = To_unsigned_long(sfp[4]);
	unsigned_int arg5 = To_unsigned_int(sfp[5]);
	int ret = virDomainNodeCommit (arg0, arg1, arg2, arg3, arg4, arg5);
	RETURNint(ret);
}

// ["virDomainSetNodeIoTune", "int", "virDomainPtr", "const char *", "virTypedParameterPtr", "int", "unsigned int"]
static KMETHOD KvirDomainSetNodeIoTune(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virTypedParameterPtr arg2 = To_virTypedParameterPtr(sfp[2]);
	int arg3 = To_int(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainSetNodeIoTune (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainGetNodeIoTune", "int", "virDomainPtr", "const char *", "virTypedParameterPtr", "int *", "unsigned int"]
static KMETHOD KvirDomainGetNodeIoTune(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virTypedParameterPtr arg2 = To_virTypedParameterPtr(sfp[2]);
	intPtr arg3 = To_intPtr(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virDomainGetNodeIoTune (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virDomainGetDiskErrors", "int", "virDomainPtr", "virDomainDiskErrorPtr", "unsigned int", "unsigned int"]
static KMETHOD KvirDomainGetDiskErrors(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virDomainDiskErrorPtr arg1 = To_virDomainDiskErrorPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainGetDiskErrors (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virNodeGetCellsFreeMemory", "int", "virConnectPtr", "unsigned long long *", "int", "int"]
static KMETHOD KvirNodeGetCellsFreeMemory(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_long_long_Ptr arg1 = To_unsigned_long_long_Ptr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int arg3 = To_int(sfp[3]);
	int ret = virNodeGetCellsFreeMemory (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virNetworkGetConnect", "virConnectPtr", "virNetworkPtr"]
static KMETHOD KvirNetworkGetConnect(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	virConnectPtr ret = virNetworkGetConnect (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virConnectNumOfNetworks", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfNetworks(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfNetworks (arg0);
	RETURNint(ret);
}

// ["virConnectListNetworks", "int", "virConnectPtr", "char **const", "int"]
static KMETHOD KvirConnectListNetworks(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListNetworks (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectNumOfDefinedNetworks", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfDefinedNetworks(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfDefinedNetworks (arg0);
	RETURNint(ret);
}

// ["virConnectListDefinedNetworks", "int", "virConnectPtr", "char **const", "int"]
static KMETHOD KvirConnectListDefinedNetworks(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListDefinedNetworks (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectListAllNetworks", "int", "virConnectPtr", "virNetworkPtr **", "unsigned int"]
static KMETHOD KvirConnectListAllNetworks(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virNetworkPtrPtrPtr arg1 = To_virNetworkPtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectListAllNetworks (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virNetworkLookupByName", "virNetworkPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirNetworkLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virNetworkPtr ret = virNetworkLookupByName (arg0, arg1);
	RETURNvirNetworkPtr(ret);
}

// ["virNetworkLookupByUUID", "virNetworkPtr", "virConnectPtr", "const unsigned char *"]
static KMETHOD KvirNetworkLookupByUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	virNetworkPtr ret = virNetworkLookupByUUID (arg0, arg1);
	RETURNvirNetworkPtr(ret);
}

// ["virNetworkLookupByUUIDString", "virNetworkPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirNetworkLookupByUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virNetworkPtr ret = virNetworkLookupByUUIDString (arg0, arg1);
	RETURNvirNetworkPtr(ret);
}

// ["virNetworkCreateXML", "virNetworkPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirNetworkCreateXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virNetworkPtr ret = virNetworkCreateXML (arg0, arg1);
	RETURNvirNetworkPtr(ret);
}

// ["virNetworkDefineXML", "virNetworkPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirNetworkDefineXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virNetworkPtr ret = virNetworkDefineXML (arg0, arg1);
	RETURNvirNetworkPtr(ret);
}

// ["virNetworkUndefine", "int", "virNetworkPtr"]
static KMETHOD KvirNetworkUndefine(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	int ret = virNetworkUndefine (arg0);
	RETURNint(ret);
}

// ["virNetworkUpdate", "int", "virNetworkPtr", "unsigned int", "unsigned int", "int", "const char *", "unsigned int"]
static KMETHOD KvirNetworkUpdate(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int arg3 = To_int(sfp[3]);
	charPtr arg4 = To_charPtr(sfp[4]);
	unsigned_int arg5 = To_unsigned_int(sfp[5]);
	int ret = virNetworkUpdate (arg0, arg1, arg2, arg3, arg4, arg5);
	RETURNint(ret);
}

// ["virNetworkCreate", "int", "virNetworkPtr"]
static KMETHOD KvirNetworkCreate(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	int ret = virNetworkCreate (arg0);
	RETURNint(ret);
}

// ["virNetworkDestroy", "int", "virNetworkPtr"]
static KMETHOD KvirNetworkDestroy(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	int ret = virNetworkDestroy (arg0);
	RETURNint(ret);
}

// ["virNetworkRef", "int", "virNetworkPtr"]
static KMETHOD KvirNetworkRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	int ret = virNetworkRef (arg0);
	RETURNint(ret);
}

// ["virNetworkFree", "int", "virNetworkPtr"]
static KMETHOD KvirNetworkFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	int ret = virNetworkFree (arg0);
	RETURNint(ret);
}

// ["virNetworkGetName", "const char *", "virNetworkPtr"]
static KMETHOD KvirNetworkGetName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	charPtr ret = virNetworkGetName (arg0);
	RETURNcharPtr(ret);
}

// ["virNetworkGetUUID", "int", "virNetworkPtr", "unsigned char *"]
static KMETHOD KvirNetworkGetUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	int ret = virNetworkGetUUID (arg0, arg1);
	RETURNint(ret);
}

// ["virNetworkGetUUIDString", "int", "virNetworkPtr", "char *"]
static KMETHOD KvirNetworkGetUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virNetworkGetUUIDString (arg0, arg1);
	RETURNint(ret);
}

// ["virNetworkGetXMLDesc", "char *", "virNetworkPtr", "unsigned int"]
static KMETHOD KvirNetworkGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virNetworkGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virNetworkGetBridgeName", "char *", "virNetworkPtr"]
static KMETHOD KvirNetworkGetBridgeName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	charPtr ret = virNetworkGetBridgeName (arg0);
	RETURNcharPtr(ret);
}

// ["virNetworkGetAutostart", "int", "virNetworkPtr", "int *"]
static KMETHOD KvirNetworkGetAutostart(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	intPtr arg1 = To_intPtr(sfp[1]);
	int ret = virNetworkGetAutostart (arg0, arg1);
	RETURNint(ret);
}

// ["virNetworkSetAutostart", "int", "virNetworkPtr", "int"]
static KMETHOD KvirNetworkSetAutostart(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	int ret = virNetworkSetAutostart (arg0, arg1);
	RETURNint(ret);
}

// ["virInterfaceGetConnect", "virConnectPtr", "virInterfacePtr"]
static KMETHOD KvirInterfaceGetConnect(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	virConnectPtr ret = virInterfaceGetConnect (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virConnectNumOfInterfaces", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfInterfaces(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfInterfaces (arg0);
	RETURNint(ret);
}

// ["virConnectListInterfaces", "int", "virConnectPtr", "char **const", "int"]
static KMETHOD KvirConnectListInterfaces(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListInterfaces (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectNumOfDefinedInterfaces", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfDefinedInterfaces(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfDefinedInterfaces (arg0);
	RETURNint(ret);
}

// ["virConnectListDefinedInterfaces", "int", "virConnectPtr", "char **const", "int"]
static KMETHOD KvirConnectListDefinedInterfaces(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListDefinedInterfaces (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectListAllInterfaces", "int", "virConnectPtr", "virInterfacePtr **", "unsigned int"]
static KMETHOD KvirConnectListAllInterfaces(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virInterfacePtrPtrPtr arg1 = To_virInterfacePtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectListAllInterfaces (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virInterfaceLookupByName", "virInterfacePtr", "virConnectPtr", "const char *"]
static KMETHOD KvirInterfaceLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virInterfacePtr ret = virInterfaceLookupByName (arg0, arg1);
	RETURNvirInterfacePtr(ret);
}

// ["virInterfaceLookupByMACString", "virInterfacePtr", "virConnectPtr", "const char *"]
static KMETHOD KvirInterfaceLookupByMACString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virInterfacePtr ret = virInterfaceLookupByMACString (arg0, arg1);
	RETURNvirInterfacePtr(ret);
}

// ["virInterfaceGetName", "const char *", "virInterfacePtr"]
static KMETHOD KvirInterfaceGetName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	charPtr ret = virInterfaceGetName (arg0);
	RETURNcharPtr(ret);
}

// ["virInterfaceGetMACString", "const char *", "virInterfacePtr"]
static KMETHOD KvirInterfaceGetMACString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	charPtr ret = virInterfaceGetMACString (arg0);
	RETURNcharPtr(ret);
}

// ["virInterfaceGetXMLDesc", "char *", "virInterfacePtr", "unsigned int"]
static KMETHOD KvirInterfaceGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virInterfaceGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virInterfaceDefineXML", "virInterfacePtr", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirInterfaceDefineXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virInterfacePtr ret = virInterfaceDefineXML (arg0, arg1, arg2);
	RETURNvirInterfacePtr(ret);
}

// ["virInterfaceUndefine", "int", "virInterfacePtr"]
static KMETHOD KvirInterfaceUndefine(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	int ret = virInterfaceUndefine (arg0);
	RETURNint(ret);
}

// ["virInterfaceCreate", "int", "virInterfacePtr", "unsigned int"]
static KMETHOD KvirInterfaceCreate(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virInterfaceCreate (arg0, arg1);
	RETURNint(ret);
}

// ["virInterfaceDestroy", "int", "virInterfacePtr", "unsigned int"]
static KMETHOD KvirInterfaceDestroy(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virInterfaceDestroy (arg0, arg1);
	RETURNint(ret);
}

// ["virInterfaceRef", "int", "virInterfacePtr"]
static KMETHOD KvirInterfaceRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	int ret = virInterfaceRef (arg0);
	RETURNint(ret);
}

// ["virInterfaceFree", "int", "virInterfacePtr"]
static KMETHOD KvirInterfaceFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	int ret = virInterfaceFree (arg0);
	RETURNint(ret);
}

// ["virInterfaceChangeBegin", "int", "virConnectPtr", "unsigned int"]
static KMETHOD KvirInterfaceChangeBegin(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virInterfaceChangeBegin (arg0, arg1);
	RETURNint(ret);
}

// ["virInterfaceChangeCommit", "int", "virConnectPtr", "unsigned int"]
static KMETHOD KvirInterfaceChangeCommit(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virInterfaceChangeCommit (arg0, arg1);
	RETURNint(ret);
}

// ["virInterfaceChangeRollback", "int", "virConnectPtr", "unsigned int"]
static KMETHOD KvirInterfaceChangeRollback(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virInterfaceChangeRollback (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolGetConnect", "virConnectPtr", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolGetConnect(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	virConnectPtr ret = virStoragePoolGetConnect (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virConnectNumOfStoragePools", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfStoragePools(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfStoragePools (arg0);
	RETURNint(ret);
}

// ["virConnectListStoragePools", "int", "virConnectPtr", "char **const", "int"]
static KMETHOD KvirConnectListStoragePools(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListStoragePools (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectNumOfDefinedStoragePools", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfDefinedStoragePools(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfDefinedStoragePools (arg0);
	RETURNint(ret);
}

// ["virConnectListDefinedStoragePools", "int", "virConnectPtr", "char **const", "int"]
static KMETHOD KvirConnectListDefinedStoragePools(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListDefinedStoragePools (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectListAllStoragePools", "int", "virConnectPtr", "virStoragePoolPtr **", "unsigned int"]
static KMETHOD KvirConnectListAllStoragePools(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virStoragePoolPtrPtrPtr arg1 = To_virStoragePoolPtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectListAllStoragePools (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectFindStoragePoolSources", "char *", "virConnectPtr", "const char *", "const char *", "unsigned int"]
static KMETHOD KvirConnectFindStoragePoolSources(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	charPtr ret = virConnectFindStoragePoolSources (arg0, arg1, arg2, arg3);
	RETURNcharPtr(ret);
}

// ["virStoragePoolLookupByName", "virStoragePoolPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirStoragePoolLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virStoragePoolPtr ret = virStoragePoolLookupByName (arg0, arg1);
	RETURNvirStoragePoolPtr(ret);
}

// ["virStoragePoolLookupByUUID", "virStoragePoolPtr", "virConnectPtr", "const unsigned char *"]
static KMETHOD KvirStoragePoolLookupByUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	virStoragePoolPtr ret = virStoragePoolLookupByUUID (arg0, arg1);
	RETURNvirStoragePoolPtr(ret);
}

// ["virStoragePoolLookupByUUIDString", "virStoragePoolPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirStoragePoolLookupByUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virStoragePoolPtr ret = virStoragePoolLookupByUUIDString (arg0, arg1);
	RETURNvirStoragePoolPtr(ret);
}

// ["virStoragePoolLookupByVolume", "virStoragePoolPtr", "virStorageVolPtr"]
static KMETHOD KvirStoragePoolLookupByVolume(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	virStoragePoolPtr ret = virStoragePoolLookupByVolume (arg0);
	RETURNvirStoragePoolPtr(ret);
}

// ["virStoragePoolCreateXML", "virStoragePoolPtr", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirStoragePoolCreateXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virStoragePoolPtr ret = virStoragePoolCreateXML (arg0, arg1, arg2);
	RETURNvirStoragePoolPtr(ret);
}

// ["virStoragePoolDefineXML", "virStoragePoolPtr", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirStoragePoolDefineXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virStoragePoolPtr ret = virStoragePoolDefineXML (arg0, arg1, arg2);
	RETURNvirStoragePoolPtr(ret);
}

// ["virStoragePoolBuild", "int", "virStoragePoolPtr", "unsigned int"]
static KMETHOD KvirStoragePoolBuild(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virStoragePoolBuild (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolUndefine", "int", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolUndefine(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	int ret = virStoragePoolUndefine (arg0);
	RETURNint(ret);
}

// ["virStoragePoolCreate", "int", "virStoragePoolPtr", "unsigned int"]
static KMETHOD KvirStoragePoolCreate(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virStoragePoolCreate (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolDestroy", "int", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolDestroy(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	int ret = virStoragePoolDestroy (arg0);
	RETURNint(ret);
}

// ["virStoragePoolDelete", "int", "virStoragePoolPtr", "unsigned int"]
static KMETHOD KvirStoragePoolDelete(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virStoragePoolDelete (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolRef", "int", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	int ret = virStoragePoolRef (arg0);
	RETURNint(ret);
}

// ["virStoragePoolFree", "int", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	int ret = virStoragePoolFree (arg0);
	RETURNint(ret);
}

// ["virStoragePoolRefresh", "int", "virStoragePoolPtr", "unsigned int"]
static KMETHOD KvirStoragePoolRefresh(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virStoragePoolRefresh (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolGetName", "const char *", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolGetName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	charPtr ret = virStoragePoolGetName (arg0);
	RETURNcharPtr(ret);
}

// ["virStoragePoolGetUUID", "int", "virStoragePoolPtr", "unsigned char *"]
static KMETHOD KvirStoragePoolGetUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	int ret = virStoragePoolGetUUID (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolGetUUIDString", "int", "virStoragePoolPtr", "char *"]
static KMETHOD KvirStoragePoolGetUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virStoragePoolGetUUIDString (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolGetInfo", "int", "virStoragePoolPtr", "virStoragePoolInfoPtr"]
static KMETHOD KvirStoragePoolGetInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	virStoragePoolInfoPtr arg1 = To_virStoragePoolInfoPtr(sfp[1]);
	int ret = virStoragePoolGetInfo (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolGetXMLDesc", "char *", "virStoragePoolPtr", "unsigned int"]
static KMETHOD KvirStoragePoolGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virStoragePoolGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virStoragePoolGetAutostart", "int", "virStoragePoolPtr", "int *"]
static KMETHOD KvirStoragePoolGetAutostart(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	intPtr arg1 = To_intPtr(sfp[1]);
	int ret = virStoragePoolGetAutostart (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolSetAutostart", "int", "virStoragePoolPtr", "int"]
static KMETHOD KvirStoragePoolSetAutostart(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	int ret = virStoragePoolSetAutostart (arg0, arg1);
	RETURNint(ret);
}

// ["virStoragePoolNumOfVolumes", "int", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolNumOfVolumes(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	int ret = virStoragePoolNumOfVolumes (arg0);
	RETURNint(ret);
}

// ["virStoragePoolListVolumes", "int", "virStoragePoolPtr", "char **const", "int"]
static KMETHOD KvirStoragePoolListVolumes(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virStoragePoolListVolumes (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virStoragePoolListAllVolumes", "int", "virStoragePoolPtr", "virStorageVolPtr **", "unsigned int"]
static KMETHOD KvirStoragePoolListAllVolumes(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	virStorageVolPtrPtrPtr arg1 = To_virStorageVolPtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virStoragePoolListAllVolumes (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virStorageVolGetConnect", "virConnectPtr", "virStorageVolPtr"]
static KMETHOD KvirStorageVolGetConnect(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	virConnectPtr ret = virStorageVolGetConnect (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virStorageVolLookupByName", "virStorageVolPtr", "virStoragePoolPtr", "const char *"]
static KMETHOD KvirStorageVolLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virStorageVolPtr ret = virStorageVolLookupByName (arg0, arg1);
	RETURNvirStorageVolPtr(ret);
}

// ["virStorageVolLookupByKey", "virStorageVolPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirStorageVolLookupByKey(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virStorageVolPtr ret = virStorageVolLookupByKey (arg0, arg1);
	RETURNvirStorageVolPtr(ret);
}

// ["virStorageVolLookupByPath", "virStorageVolPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirStorageVolLookupByPath(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virStorageVolPtr ret = virStorageVolLookupByPath (arg0, arg1);
	RETURNvirStorageVolPtr(ret);
}

// ["virStorageVolGetName", "const char *", "virStorageVolPtr"]
static KMETHOD KvirStorageVolGetName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	charPtr ret = virStorageVolGetName (arg0);
	RETURNcharPtr(ret);
}

// ["virStorageVolGetKey", "const char *", "virStorageVolPtr"]
static KMETHOD KvirStorageVolGetKey(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	charPtr ret = virStorageVolGetKey (arg0);
	RETURNcharPtr(ret);
}

// ["virStorageVolCreateXML", "virStorageVolPtr", "virStoragePoolPtr", "const char *", "unsigned int"]
static KMETHOD KvirStorageVolCreateXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virStorageVolPtr ret = virStorageVolCreateXML (arg0, arg1, arg2);
	RETURNvirStorageVolPtr(ret);
}

// ["virStorageVolCreateXMLFrom", "virStorageVolPtr", "virStoragePoolPtr", "const char *", "virStorageVolPtr", "unsigned int"]
static KMETHOD KvirStorageVolCreateXMLFrom(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virStorageVolPtr arg2 = To_virStorageVolPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	virStorageVolPtr ret = virStorageVolCreateXMLFrom (arg0, arg1, arg2, arg3);
	RETURNvirStorageVolPtr(ret);
}

// ["virStorageVolDownload", "int", "virStorageVolPtr", "virStreamPtr", "unsigned long long", "unsigned long long", "unsigned int"]
static KMETHOD KvirStorageVolDownload(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	virStreamPtr arg1 = To_virStreamPtr(sfp[1]);
	unsigned_long_long arg2 = To_unsigned_long_long(sfp[2]);
	unsigned_long_long arg3 = To_unsigned_long_long(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virStorageVolDownload (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virStorageVolUpload", "int", "virStorageVolPtr", "virStreamPtr", "unsigned long long", "unsigned long long", "unsigned int"]
static KMETHOD KvirStorageVolUpload(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	virStreamPtr arg1 = To_virStreamPtr(sfp[1]);
	unsigned_long_long arg2 = To_unsigned_long_long(sfp[2]);
	unsigned_long_long arg3 = To_unsigned_long_long(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virStorageVolUpload (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virStorageVolDelete", "int", "virStorageVolPtr", "unsigned int"]
static KMETHOD KvirStorageVolDelete(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virStorageVolDelete (arg0, arg1);
	RETURNint(ret);
}

// ["virStorageVolWipe", "int", "virStorageVolPtr", "unsigned int"]
static KMETHOD KvirStorageVolWipe(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virStorageVolWipe (arg0, arg1);
	RETURNint(ret);
}

// ["virStorageVolWipePattern", "int", "virStorageVolPtr", "unsigned int", "unsigned int"]
static KMETHOD KvirStorageVolWipePattern(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virStorageVolWipePattern (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virStorageVolRef", "int", "virStorageVolPtr"]
static KMETHOD KvirStorageVolRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	int ret = virStorageVolRef (arg0);
	RETURNint(ret);
}

// ["virStorageVolFree", "int", "virStorageVolPtr"]
static KMETHOD KvirStorageVolFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	int ret = virStorageVolFree (arg0);
	RETURNint(ret);
}

// ["virStorageVolGetInfo", "int", "virStorageVolPtr", "virStorageVolInfoPtr"]
static KMETHOD KvirStorageVolGetInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	virStorageVolInfoPtr arg1 = To_virStorageVolInfoPtr(sfp[1]);
	int ret = virStorageVolGetInfo (arg0, arg1);
	RETURNint(ret);
}

// ["virStorageVolGetXMLDesc", "char *", "virStorageVolPtr", "unsigned int"]
static KMETHOD KvirStorageVolGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virStorageVolGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virStorageVolGetPath", "char *", "virStorageVolPtr"]
static KMETHOD KvirStorageVolGetPath(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	charPtr ret = virStorageVolGetPath (arg0);
	RETURNcharPtr(ret);
}

// ["virStorageVolResize", "int", "virStorageVolPtr", "unsigned long long", "unsigned int"]
static KMETHOD KvirStorageVolResize(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStorageVolPtr arg0 = To_virStorageVolPtr(sfp[0]);
	unsigned_long_long arg1 = To_unsigned_long_long(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virStorageVolResize (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainSendKey", "int", "virDomainPtr", "unsigned int", "unsigned int", "unsigned int *", "int", "unsigned int"]
static KMETHOD KvirDomainSendKey(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	unsigned_intPtr arg3 = To_unsigned_intPtr(sfp[3]);
	int arg4 = To_int(sfp[4]);
	unsigned_int arg5 = To_unsigned_int(sfp[5]);
	int ret = virDomainSendKey (arg0, arg1, arg2, arg3, arg4, arg5);
	RETURNint(ret);
}

// ["virDomainCreateLinux", "virDomainPtr", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainCreateLinux(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virDomainPtr ret = virDomainCreateLinux (arg0, arg1, arg2);
	RETURNvirDomainPtr(ret);
}

// ["virNodeNumOfDevices", "int", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirNodeNumOfDevices(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virNodeNumOfDevices (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virNodeListDevices", "int", "virConnectPtr", "const char *", "char **const", "int", "unsigned int"]
static KMETHOD KvirNodeListDevices(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	charPtrPtr arg2 = To_charPtrPtr(sfp[2]);
	int arg3 = To_int(sfp[3]);
	unsigned_int arg4 = To_unsigned_int(sfp[4]);
	int ret = virNodeListDevices (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virConnectListAllNodeDevices", "int", "virConnectPtr", "virNodeDevicePtr **", "unsigned int"]
static KMETHOD KvirConnectListAllNodeDevices(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virNodeDevicePtrPtrPtr arg1 = To_virNodeDevicePtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectListAllNodeDevices (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virNodeDeviceLookupByName", "virNodeDevicePtr", "virConnectPtr", "const char *"]
static KMETHOD KvirNodeDeviceLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virNodeDevicePtr ret = virNodeDeviceLookupByName (arg0, arg1);
	RETURNvirNodeDevicePtr(ret);
}

// ["virNodeDeviceGetName", "const char *", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceGetName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	charPtr ret = virNodeDeviceGetName (arg0);
	RETURNcharPtr(ret);
}

// ["virNodeDeviceGetParent", "const char *", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceGetParent(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	charPtr ret = virNodeDeviceGetParent (arg0);
	RETURNcharPtr(ret);
}

// ["virNodeDeviceNumOfCaps", "int", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceNumOfCaps(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	int ret = virNodeDeviceNumOfCaps (arg0);
	RETURNint(ret);
}

// ["virNodeDeviceListCaps", "int", "virNodeDevicePtr", "char **const", "int"]
static KMETHOD KvirNodeDeviceListCaps(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virNodeDeviceListCaps (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virNodeDeviceGetXMLDesc", "char *", "virNodeDevicePtr", "unsigned int"]
static KMETHOD KvirNodeDeviceGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virNodeDeviceGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virNodeDeviceRef", "int", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	int ret = virNodeDeviceRef (arg0);
	RETURNint(ret);
}

// ["virNodeDeviceFree", "int", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	int ret = virNodeDeviceFree (arg0);
	RETURNint(ret);
}

// ["virNodeDeviceDettach", "int", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceDettach(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	int ret = virNodeDeviceDettach (arg0);
	RETURNint(ret);
}

// ["virNodeDeviceReAttach", "int", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceReAttach(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	int ret = virNodeDeviceReAttach (arg0);
	RETURNint(ret);
}

// ["virNodeDeviceReset", "int", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceReset(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	int ret = virNodeDeviceReset (arg0);
	RETURNint(ret);
}

// ["virNodeDeviceCreateXML", "virNodeDevicePtr", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirNodeDeviceCreateXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virNodeDevicePtr ret = virNodeDeviceCreateXML (arg0, arg1, arg2);
	RETURNvirNodeDevicePtr(ret);
}

// ["virNodeDeviceDestroy", "int", "virNodeDevicePtr"]
static KMETHOD KvirNodeDeviceDestroy(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNodeDevicePtr arg0 = To_virNodeDevicePtr(sfp[0]);
	int ret = virNodeDeviceDestroy (arg0);
	RETURNint(ret);
}

// ["virConnectDomainEventRegister", "int", "virConnectPtr", "virConnectDomainEventCallback", "void *", "virFreeCallback"]
static KMETHOD KvirConnectDomainEventRegister(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virConnectDomainEventCallback arg1 = To_virConnectDomainEventCallback(sfp[1]);
	voidPtr arg2 = To_voidPtr(sfp[2]);
	virFreeCallback arg3 = To_virFreeCallback(sfp[3]);
	int ret = virConnectDomainEventRegister (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virConnectDomainEventDeregister", "int", "virConnectPtr", "virConnectDomainEventCallback"]
static KMETHOD KvirConnectDomainEventDeregister(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virConnectDomainEventCallback arg1 = To_virConnectDomainEventCallback(sfp[1]);
	int ret = virConnectDomainEventDeregister (arg0, arg1);
	RETURNint(ret);
}

// ["virEventRegisterImpl", "void", "virEventAddHandleFunc", "virEventUpdateHandleFunc", "virEventRemoveHandleFunc", "virEventAddTimeoutFunc", "virEventUpdateTimeoutFunc", "virEventRemoveTimeoutFunc"]
static KMETHOD KvirEventRegisterImpl(KonohaContext *kctx,  KonohaStack *sfp)
{
	virEventAddHandleFunc arg0 = To_virEventAddHandleFunc(sfp[0]);
	virEventUpdateHandleFunc arg1 = To_virEventUpdateHandleFunc(sfp[1]);
	virEventRemoveHandleFunc arg2 = To_virEventRemoveHandleFunc(sfp[2]);
	virEventAddTimeoutFunc arg3 = To_virEventAddTimeoutFunc(sfp[3]);
	virEventUpdateTimeoutFunc arg4 = To_virEventUpdateTimeoutFunc(sfp[4]);
	virEventRemoveTimeoutFunc arg5 = To_virEventRemoveTimeoutFunc(sfp[5]);
	virEventRegisterImpl (arg0, arg1, arg2, arg3, arg4, arg5);
	returnvoid(virEventRegisterImpl);
}

// ["virEventRegisterDefaultImpl", "int"]
static KMETHOD KvirEventRegisterDefaultImpl(KonohaContext *kctx,  KonohaStack *sfp)
{
	int ret = virEventRegisterDefaultImpl ();
	RETURNint(ret);
}

// ["virEventRunDefaultImpl", "int"]
static KMETHOD KvirEventRunDefaultImpl(KonohaContext *kctx,  KonohaStack *sfp)
{
	int ret = virEventRunDefaultImpl ();
	RETURNint(ret);
}

// ["virEventAddHandle", "int", "int", "int", "virEventHandleCallback", "void *", "virFreeCallback"]
static KMETHOD KvirEventAddHandle(KonohaContext *kctx,  KonohaStack *sfp)
{
	int arg0 = To_int(sfp[0]);
	int arg1 = To_int(sfp[1]);
	virEventHandleCallback arg2 = To_virEventHandleCallback(sfp[2]);
	voidPtr arg3 = To_voidPtr(sfp[3]);
	virFreeCallback arg4 = To_virFreeCallback(sfp[4]);
	int ret = virEventAddHandle (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virEventUpdateHandle", "void", "int", "int"]
static KMETHOD KvirEventUpdateHandle(KonohaContext *kctx,  KonohaStack *sfp)
{
	int arg0 = To_int(sfp[0]);
	int arg1 = To_int(sfp[1]);
	virEventUpdateHandle (arg0, arg1);
	returnvoid(virEventUpdateHandle);
}

// ["virEventRemoveHandle", "int", "int"]
static KMETHOD KvirEventRemoveHandle(KonohaContext *kctx,  KonohaStack *sfp)
{
	int arg0 = To_int(sfp[0]);
	int ret = virEventRemoveHandle (arg0);
	RETURNint(ret);
}

// ["virEventAddTimeout", "int", "int", "virEventTimeoutCallback", "void *", "virFreeCallback"]
static KMETHOD KvirEventAddTimeout(KonohaContext *kctx,  KonohaStack *sfp)
{
	int arg0 = To_int(sfp[0]);
	virEventTimeoutCallback arg1 = To_virEventTimeoutCallback(sfp[1]);
	voidPtr arg2 = To_voidPtr(sfp[2]);
	virFreeCallback arg3 = To_virFreeCallback(sfp[3]);
	int ret = virEventAddTimeout (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virEventUpdateTimeout", "void", "int", "int"]
static KMETHOD KvirEventUpdateTimeout(KonohaContext *kctx,  KonohaStack *sfp)
{
	int arg0 = To_int(sfp[0]);
	int arg1 = To_int(sfp[1]);
	virEventUpdateTimeout (arg0, arg1);
	returnvoid(virEventUpdateTimeout);
}

// ["virEventRemoveTimeout", "int", "int"]
static KMETHOD KvirEventRemoveTimeout(KonohaContext *kctx,  KonohaStack *sfp)
{
	int arg0 = To_int(sfp[0]);
	int ret = virEventRemoveTimeout (arg0);
	RETURNint(ret);
}

// ["virSecretGetConnect", "virConnectPtr", "virSecretPtr"]
static KMETHOD KvirSecretGetConnect(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	virConnectPtr ret = virSecretGetConnect (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virConnectNumOfSecrets", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfSecrets(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfSecrets (arg0);
	RETURNint(ret);
}

// ["virConnectListSecrets", "int", "virConnectPtr", "char **", "int"]
static KMETHOD KvirConnectListSecrets(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListSecrets (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectListAllSecrets", "int", "virConnectPtr", "virSecretPtr **", "unsigned int"]
static KMETHOD KvirConnectListAllSecrets(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virSecretPtrPtrPtr arg1 = To_virSecretPtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectListAllSecrets (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virSecretLookupByUUID", "virSecretPtr", "virConnectPtr", "const unsigned char *"]
static KMETHOD KvirSecretLookupByUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	virSecretPtr ret = virSecretLookupByUUID (arg0, arg1);
	RETURNvirSecretPtr(ret);
}

// ["virSecretLookupByUUIDString", "virSecretPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirSecretLookupByUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virSecretPtr ret = virSecretLookupByUUIDString (arg0, arg1);
	RETURNvirSecretPtr(ret);
}

// ["virSecretLookupByUsage", "virSecretPtr", "virConnectPtr", "int", "const char *"]
static KMETHOD KvirSecretLookupByUsage(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	charPtr arg2 = To_charPtr(sfp[2]);
	virSecretPtr ret = virSecretLookupByUsage (arg0, arg1, arg2);
	RETURNvirSecretPtr(ret);
}

// ["virSecretDefineXML", "virSecretPtr", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirSecretDefineXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virSecretPtr ret = virSecretDefineXML (arg0, arg1, arg2);
	RETURNvirSecretPtr(ret);
}

// ["virSecretGetUUID", "int", "virSecretPtr", "unsigned char *"]
static KMETHOD KvirSecretGetUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	int ret = virSecretGetUUID (arg0, arg1);
	RETURNint(ret);
}

// ["virSecretGetUUIDString", "int", "virSecretPtr", "char *"]
static KMETHOD KvirSecretGetUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virSecretGetUUIDString (arg0, arg1);
	RETURNint(ret);
}

// ["virSecretGetUsageType", "int", "virSecretPtr"]
static KMETHOD KvirSecretGetUsageType(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	int ret = virSecretGetUsageType (arg0);
	RETURNint(ret);
}

// ["virSecretGetUsageID", "const char *", "virSecretPtr"]
static KMETHOD KvirSecretGetUsageID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	charPtr ret = virSecretGetUsageID (arg0);
	RETURNcharPtr(ret);
}

// ["virSecretGetXMLDesc", "char *", "virSecretPtr", "unsigned int"]
static KMETHOD KvirSecretGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virSecretGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virSecretSetValue", "int", "virSecretPtr", "const unsigned char *", "size_t", "unsigned int"]
static KMETHOD KvirSecretSetValue(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	size_t arg2 = To_size_t(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virSecretSetValue (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virSecretGetValue", "unsigned char *", "virSecretPtr", "size_t *", "unsigned int"]
static KMETHOD KvirSecretGetValue(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	size_tPtr arg1 = To_size_tPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	unsigned_charPtr ret = virSecretGetValue (arg0, arg1, arg2);
	RETURNunsigned_charPtr(ret);
}

// ["virSecretUndefine", "int", "virSecretPtr"]
static KMETHOD KvirSecretUndefine(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	int ret = virSecretUndefine (arg0);
	RETURNint(ret);
}

// ["virSecretRef", "int", "virSecretPtr"]
static KMETHOD KvirSecretRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	int ret = virSecretRef (arg0);
	RETURNint(ret);
}

// ["virSecretFree", "int", "virSecretPtr"]
static KMETHOD KvirSecretFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virSecretPtr arg0 = To_virSecretPtr(sfp[0]);
	int ret = virSecretFree (arg0);
	RETURNint(ret);
}

// ["virStreamNew", "virStreamPtr", "virConnectPtr", "unsigned int"]
static KMETHOD KvirStreamNew(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	virStreamPtr ret = virStreamNew (arg0, arg1);
	RETURNvirStreamPtr(ret);
}

// ["virStreamRef", "int", "virStreamPtr"]
static KMETHOD KvirStreamRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	int ret = virStreamRef (arg0);
	RETURNint(ret);
}

// ["virStreamSend", "int", "virStreamPtr", "const char *", "size_t"]
static KMETHOD KvirStreamSend(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	size_t arg2 = To_size_t(sfp[2]);
	int ret = virStreamSend (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virStreamRecv", "int", "virStreamPtr", "char *", "size_t"]
static KMETHOD KvirStreamRecv(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	size_t arg2 = To_size_t(sfp[2]);
	int ret = virStreamRecv (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virStreamSendAll", "int", "virStreamPtr", "virStreamSourceFunc", "void *"]
static KMETHOD KvirStreamSendAll(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	virStreamSourceFunc arg1 = To_virStreamSourceFunc(sfp[1]);
	voidPtr arg2 = To_voidPtr(sfp[2]);
	int ret = virStreamSendAll (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virStreamRecvAll", "int", "virStreamPtr", "virStreamSinkFunc", "void *"]
static KMETHOD KvirStreamRecvAll(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	virStreamSinkFunc arg1 = To_virStreamSinkFunc(sfp[1]);
	voidPtr arg2 = To_voidPtr(sfp[2]);
	int ret = virStreamRecvAll (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virStreamEventAddCallback", "int", "virStreamPtr", "int", "virStreamEventCallback", "void *", "virFreeCallback"]
static KMETHOD KvirStreamEventAddCallback(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	virStreamEventCallback arg2 = To_virStreamEventCallback(sfp[2]);
	voidPtr arg3 = To_voidPtr(sfp[3]);
	virFreeCallback arg4 = To_virFreeCallback(sfp[4]);
	int ret = virStreamEventAddCallback (arg0, arg1, arg2, arg3, arg4);
	RETURNint(ret);
}

// ["virStreamEventUpdateCallback", "int", "virStreamPtr", "int"]
static KMETHOD KvirStreamEventUpdateCallback(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	int ret = virStreamEventUpdateCallback (arg0, arg1);
	RETURNint(ret);
}

// ["virStreamEventRemoveCallback", "int", "virStreamPtr"]
static KMETHOD KvirStreamEventRemoveCallback(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	int ret = virStreamEventRemoveCallback (arg0);
	RETURNint(ret);
}

// ["virStreamFinish", "int", "virStreamPtr"]
static KMETHOD KvirStreamFinish(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	int ret = virStreamFinish (arg0);
	RETURNint(ret);
}

// ["virStreamAbort", "int", "virStreamPtr"]
static KMETHOD KvirStreamAbort(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	int ret = virStreamAbort (arg0);
	RETURNint(ret);
}

// ["virStreamFree", "int", "virStreamPtr"]
static KMETHOD KvirStreamFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStreamPtr arg0 = To_virStreamPtr(sfp[0]);
	int ret = virStreamFree (arg0);
	RETURNint(ret);
}

// ["virDomainIsActive", "int", "virDomainPtr"]
static KMETHOD KvirDomainIsActive(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainIsActive (arg0);
	RETURNint(ret);
}

// ["virDomainIsPersistent", "int", "virDomainPtr"]
static KMETHOD KvirDomainIsPersistent(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainIsPersistent (arg0);
	RETURNint(ret);
}

// ["virDomainIsUpdated", "int", "virDomainPtr"]
static KMETHOD KvirDomainIsUpdated(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainIsUpdated (arg0);
	RETURNint(ret);
}

// ["virNetworkIsActive", "int", "virNetworkPtr"]
static KMETHOD KvirNetworkIsActive(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	int ret = virNetworkIsActive (arg0);
	RETURNint(ret);
}

// ["virNetworkIsPersistent", "int", "virNetworkPtr"]
static KMETHOD KvirNetworkIsPersistent(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNetworkPtr arg0 = To_virNetworkPtr(sfp[0]);
	int ret = virNetworkIsPersistent (arg0);
	RETURNint(ret);
}

// ["virStoragePoolIsActive", "int", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolIsActive(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	int ret = virStoragePoolIsActive (arg0);
	RETURNint(ret);
}

// ["virStoragePoolIsPersistent", "int", "virStoragePoolPtr"]
static KMETHOD KvirStoragePoolIsPersistent(KonohaContext *kctx,  KonohaStack *sfp)
{
	virStoragePoolPtr arg0 = To_virStoragePoolPtr(sfp[0]);
	int ret = virStoragePoolIsPersistent (arg0);
	RETURNint(ret);
}

// ["virInterfaceIsActive", "int", "virInterfacePtr"]
static KMETHOD KvirInterfaceIsActive(KonohaContext *kctx,  KonohaStack *sfp)
{
	virInterfacePtr arg0 = To_virInterfacePtr(sfp[0]);
	int ret = virInterfaceIsActive (arg0);
	RETURNint(ret);
}

// ["virConnectIsEncrypted", "int", "virConnectPtr"]
static KMETHOD KvirConnectIsEncrypted(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectIsEncrypted (arg0);
	RETURNint(ret);
}

// ["virConnectIsSecure", "int", "virConnectPtr"]
static KMETHOD KvirConnectIsSecure(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectIsSecure (arg0);
	RETURNint(ret);
}

// ["virConnectIsAlive", "int", "virConnectPtr"]
static KMETHOD KvirConnectIsAlive(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectIsAlive (arg0);
	RETURNint(ret);
}

// ["virConnectCompareCPU", "int", "virConnectPtr", "const char *", "unsigned int"]
static KMETHOD KvirConnectCompareCPU(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectCompareCPU (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectBaselineCPU", "char *", "virConnectPtr", "const char **", "unsigned int", "unsigned int"]
static KMETHOD KvirConnectBaselineCPU(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	charPtr ret = virConnectBaselineCPU (arg0, arg1, arg2, arg3);
	RETURNcharPtr(ret);
}

// ["virDomainGetJobInfo", "int", "virDomainPtr", "virDomainJobInfoPtr"]
static KMETHOD KvirDomainGetJobInfo(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virDomainJobInfoPtr arg1 = To_virDomainJobInfoPtr(sfp[1]);
	int ret = virDomainGetJobInfo (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainAbortJob", "int", "virDomainPtr"]
static KMETHOD KvirDomainAbortJob(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	int ret = virDomainAbortJob (arg0);
	RETURNint(ret);
}

// ["virDomainSnapshotGetName", "const char *", "virDomainSnapshotPtr"]
static KMETHOD KvirDomainSnapshotGetName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	charPtr ret = virDomainSnapshotGetName (arg0);
	RETURNcharPtr(ret);
}

// ["virDomainSnapshotGetDomain", "virDomainPtr", "virDomainSnapshotPtr"]
static KMETHOD KvirDomainSnapshotGetDomain(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	virDomainPtr ret = virDomainSnapshotGetDomain (arg0);
	RETURNvirDomainPtr(ret);
}

// ["virDomainSnapshotGetConnect", "virConnectPtr", "virDomainSnapshotPtr"]
static KMETHOD KvirDomainSnapshotGetConnect(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	virConnectPtr ret = virDomainSnapshotGetConnect (arg0);
	RETURNvirConnectPtr(ret);
}

// ["virDomainSnapshotCreateXML", "virDomainSnapshotPtr", "virDomainPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainSnapshotCreateXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virDomainSnapshotPtr ret = virDomainSnapshotCreateXML (arg0, arg1, arg2);
	RETURNvirDomainSnapshotPtr(ret);
}

// ["virDomainSnapshotGetXMLDesc", "char *", "virDomainSnapshotPtr", "unsigned int"]
static KMETHOD KvirDomainSnapshotGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virDomainSnapshotGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virDomainSnapshotNum", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainSnapshotNum(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainSnapshotNum (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSnapshotListNames", "int", "virDomainPtr", "char **", "int", "unsigned int"]
static KMETHOD KvirDomainSnapshotListNames(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainSnapshotListNames (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainListAllSnapshots", "int", "virDomainPtr", "virDomainSnapshotPtr **", "unsigned int"]
static KMETHOD KvirDomainListAllSnapshots(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	virDomainSnapshotPtrPtrPtr arg1 = To_virDomainSnapshotPtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainListAllSnapshots (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainSnapshotNumChildren", "int", "virDomainSnapshotPtr", "unsigned int"]
static KMETHOD KvirDomainSnapshotNumChildren(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainSnapshotNumChildren (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSnapshotListChildrenNames", "int", "virDomainSnapshotPtr", "char **", "int", "unsigned int"]
static KMETHOD KvirDomainSnapshotListChildrenNames(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainSnapshotListChildrenNames (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainSnapshotListAllChildren", "int", "virDomainSnapshotPtr", "virDomainSnapshotPtr **", "unsigned int"]
static KMETHOD KvirDomainSnapshotListAllChildren(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	virDomainSnapshotPtrPtrPtr arg1 = To_virDomainSnapshotPtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virDomainSnapshotListAllChildren (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virDomainSnapshotLookupByName", "virDomainSnapshotPtr", "virDomainPtr", "const char *", "unsigned int"]
static KMETHOD KvirDomainSnapshotLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	virDomainSnapshotPtr ret = virDomainSnapshotLookupByName (arg0, arg1, arg2);
	RETURNvirDomainSnapshotPtr(ret);
}

// ["virDomainHasCurrentSnapshot", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainHasCurrentSnapshot(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainHasCurrentSnapshot (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSnapshotCurrent", "virDomainSnapshotPtr", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainSnapshotCurrent(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	virDomainSnapshotPtr ret = virDomainSnapshotCurrent (arg0, arg1);
	RETURNvirDomainSnapshotPtr(ret);
}

// ["virDomainSnapshotGetParent", "virDomainSnapshotPtr", "virDomainSnapshotPtr", "unsigned int"]
static KMETHOD KvirDomainSnapshotGetParent(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	virDomainSnapshotPtr ret = virDomainSnapshotGetParent (arg0, arg1);
	RETURNvirDomainSnapshotPtr(ret);
}

// ["virDomainSnapshotIsCurrent", "int", "virDomainSnapshotPtr", "unsigned int"]
static KMETHOD KvirDomainSnapshotIsCurrent(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainSnapshotIsCurrent (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSnapshotHasMetadata", "int", "virDomainSnapshotPtr", "unsigned int"]
static KMETHOD KvirDomainSnapshotHasMetadata(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainSnapshotHasMetadata (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainRevertToSnapshot", "int", "virDomainSnapshotPtr", "unsigned int"]
static KMETHOD KvirDomainRevertToSnapshot(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainRevertToSnapshot (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSnapshotDelete", "int", "virDomainSnapshotPtr", "unsigned int"]
static KMETHOD KvirDomainSnapshotDelete(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainSnapshotDelete (arg0, arg1);
	RETURNint(ret);
}

// ["virDomainSnapshotRef", "int", "virDomainSnapshotPtr"]
static KMETHOD KvirDomainSnapshotRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	int ret = virDomainSnapshotRef (arg0);
	RETURNint(ret);
}

// ["virDomainSnapshotFree", "int", "virDomainSnapshotPtr"]
static KMETHOD KvirDomainSnapshotFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainSnapshotPtr arg0 = To_virDomainSnapshotPtr(sfp[0]);
	int ret = virDomainSnapshotFree (arg0);
	RETURNint(ret);
}

// ["virConnectDomainEventRegisterAny", "int", "virConnectPtr", "virDomainPtr", "int", "virConnectDomainEventGenericCallback", "void *", "virFreeCallback"]
static KMETHOD KvirConnectDomainEventRegisterAny(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virDomainPtr arg1 = To_virDomainPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	virConnectDomainEventGenericCallback arg3 = To_virConnectDomainEventGenericCallback(sfp[3]);
	voidPtr arg4 = To_voidPtr(sfp[4]);
	virFreeCallback arg5 = To_virFreeCallback(sfp[5]);
	int ret = virConnectDomainEventRegisterAny (arg0, arg1, arg2, arg3, arg4, arg5);
	RETURNint(ret);
}

// ["virConnectDomainEventDeregisterAny", "int", "virConnectPtr", "int"]
static KMETHOD KvirConnectDomainEventDeregisterAny(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int arg1 = To_int(sfp[1]);
	int ret = virConnectDomainEventDeregisterAny (arg0, arg1);
	RETURNint(ret);
}

// ["virConnectNumOfNWFilters", "int", "virConnectPtr"]
static KMETHOD KvirConnectNumOfNWFilters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	int ret = virConnectNumOfNWFilters (arg0);
	RETURNint(ret);
}

// ["virConnectListNWFilters", "int", "virConnectPtr", "char **const", "int"]
static KMETHOD KvirConnectListNWFilters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtrPtr arg1 = To_charPtrPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	int ret = virConnectListNWFilters (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virConnectListAllNWFilters", "int", "virConnectPtr", "virNWFilterPtr **", "unsigned int"]
static KMETHOD KvirConnectListAllNWFilters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virNWFilterPtrPtrPtr arg1 = To_virNWFilterPtrPtrPtr(sfp[1]);
	unsigned_int arg2 = To_unsigned_int(sfp[2]);
	int ret = virConnectListAllNWFilters (arg0, arg1, arg2);
	RETURNint(ret);
}

// ["virNWFilterLookupByName", "virNWFilterPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirNWFilterLookupByName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virNWFilterPtr ret = virNWFilterLookupByName (arg0, arg1);
	RETURNvirNWFilterPtr(ret);
}

// ["virNWFilterLookupByUUID", "virNWFilterPtr", "virConnectPtr", "const unsigned char *"]
static KMETHOD KvirNWFilterLookupByUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	virNWFilterPtr ret = virNWFilterLookupByUUID (arg0, arg1);
	RETURNvirNWFilterPtr(ret);
}

// ["virNWFilterLookupByUUIDString", "virNWFilterPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirNWFilterLookupByUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virNWFilterPtr ret = virNWFilterLookupByUUIDString (arg0, arg1);
	RETURNvirNWFilterPtr(ret);
}

// ["virNWFilterDefineXML", "virNWFilterPtr", "virConnectPtr", "const char *"]
static KMETHOD KvirNWFilterDefineXML(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virNWFilterPtr ret = virNWFilterDefineXML (arg0, arg1);
	RETURNvirNWFilterPtr(ret);
}

// ["virNWFilterUndefine", "int", "virNWFilterPtr"]
static KMETHOD KvirNWFilterUndefine(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNWFilterPtr arg0 = To_virNWFilterPtr(sfp[0]);
	int ret = virNWFilterUndefine (arg0);
	RETURNint(ret);
}

// ["virNWFilterRef", "int", "virNWFilterPtr"]
static KMETHOD KvirNWFilterRef(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNWFilterPtr arg0 = To_virNWFilterPtr(sfp[0]);
	int ret = virNWFilterRef (arg0);
	RETURNint(ret);
}

// ["virNWFilterFree", "int", "virNWFilterPtr"]
static KMETHOD KvirNWFilterFree(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNWFilterPtr arg0 = To_virNWFilterPtr(sfp[0]);
	int ret = virNWFilterFree (arg0);
	RETURNint(ret);
}

// ["virNWFilterGetName", "const char *", "virNWFilterPtr"]
static KMETHOD KvirNWFilterGetName(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNWFilterPtr arg0 = To_virNWFilterPtr(sfp[0]);
	charPtr ret = virNWFilterGetName (arg0);
	RETURNcharPtr(ret);
}

// ["virNWFilterGetUUID", "int", "virNWFilterPtr", "unsigned char *"]
static KMETHOD KvirNWFilterGetUUID(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNWFilterPtr arg0 = To_virNWFilterPtr(sfp[0]);
	unsigned_charPtr arg1 = To_unsigned_charPtr(sfp[1]);
	int ret = virNWFilterGetUUID (arg0, arg1);
	RETURNint(ret);
}

// ["virNWFilterGetUUIDString", "int", "virNWFilterPtr", "char *"]
static KMETHOD KvirNWFilterGetUUIDString(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNWFilterPtr arg0 = To_virNWFilterPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	int ret = virNWFilterGetUUIDString (arg0, arg1);
	RETURNint(ret);
}

// ["virNWFilterGetXMLDesc", "char *", "virNWFilterPtr", "unsigned int"]
static KMETHOD KvirNWFilterGetXMLDesc(KonohaContext *kctx,  KonohaStack *sfp)
{
	virNWFilterPtr arg0 = To_virNWFilterPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	charPtr ret = virNWFilterGetXMLDesc (arg0, arg1);
	RETURNcharPtr(ret);
}

// ["virDomainOpenConsole", "int", "virDomainPtr", "const char *", "virStreamPtr", "unsigned int"]
static KMETHOD KvirDomainOpenConsole(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	charPtr arg1 = To_charPtr(sfp[1]);
	virStreamPtr arg2 = To_virStreamPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainOpenConsole (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainOpenGraphics", "int", "virDomainPtr", "unsigned int", "int", "unsigned int"]
static KMETHOD KvirDomainOpenGraphics(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virDomainOpenGraphics (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virDomainInjectNMI", "int", "virDomainPtr", "unsigned int"]
static KMETHOD KvirDomainInjectNMI(KonohaContext *kctx,  KonohaStack *sfp)
{
	virDomainPtr arg0 = To_virDomainPtr(sfp[0]);
	unsigned_int arg1 = To_unsigned_int(sfp[1]);
	int ret = virDomainInjectNMI (arg0, arg1);
	RETURNint(ret);
}

// ["virNodeGetMemoryParameters", "int", "virConnectPtr", "virTypedParameterPtr", "int *", "unsigned int"]
static KMETHOD KvirNodeGetMemoryParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	intPtr arg2 = To_intPtr(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virNodeGetMemoryParameters (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

// ["virNodeSetMemoryParameters", "int", "virConnectPtr", "virTypedParameterPtr", "int", "unsigned int"]
static KMETHOD KvirNodeSetMemoryParameters(KonohaContext *kctx,  KonohaStack *sfp)
{
	virConnectPtr arg0 = To_virConnectPtr(sfp[0]);
	virTypedParameterPtr arg1 = To_virTypedParameterPtr(sfp[1]);
	int arg2 = To_int(sfp[2]);
	unsigned_int arg3 = To_unsigned_int(sfp[3]);
	int ret = virNodeSetMemoryParameters (arg0, arg1, arg2, arg3);
	RETURNint(ret);
}

static kbool_t PACKAGE_INIT_PKG(LIBVIRT_PACKAGE_NAME)(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	virt_loadStructData(kctx, ns, trace);
	int FN_0 = KFieldName_("arg0");
	int FN_1 = KFieldName_("arg1");
	int FN_2 = KFieldName_("arg2");
	int FN_3 = KFieldName_("arg3");
	int FN_4 = KFieldName_("arg4");
	int FN_5 = KFieldName_("arg5");
	int FN_6 = KFieldName_("arg6");
	int FN_7 = KFieldName_("arg7");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(KvirDomainGetSchedulerParameters), KType_Int, 3, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_IntPtr, FN_2,
		_Public, _F(KvirDomainGetSchedulerParametersFlags), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_IntPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainSetSchedulerParameters), KType_Int, 3, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirDomainSetSchedulerParametersFlags), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainMigrate), KType_virDomainPtr, 6, KType_virDomainPtr, FN_0, KType_virConnectPtr, FN_1,KType_unsigned_long, FN_2,KType_charPtr, FN_3,KType_charPtr, FN_4,KType_unsigned_long, FN_5,
		_Public, _F(KvirDomainMigrate2), KType_virDomainPtr, 7, KType_virDomainPtr, FN_0, KType_virConnectPtr, FN_1,KType_charPtr, FN_2,KType_unsigned_long, FN_3,KType_charPtr, FN_4,KType_charPtr, FN_5,KType_unsigned_long, FN_6,
		_Public, _F(KvirDomainMigrateToURI), KType_Int, 5, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_long, FN_2,KType_charPtr, FN_3,KType_unsigned_long, FN_4,
		_Public, _F(KvirDomainMigrateToURI2), KType_Int, 7, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_charPtr, FN_3,KType_unsigned_long, FN_4,KType_charPtr, FN_5,KType_unsigned_long, FN_6,
		_Public, _F(KvirDomainMigrateSetMaxDowntime), KType_Int, 3, KType_virDomainPtr, FN_0, KType_unsigned_long_long, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainMigrateSetMaxSpeed), KType_Int, 3, KType_virDomainPtr, FN_0, KType_unsigned_long, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainMigrateGetMaxSpeed), KType_Int, 3, KType_virDomainPtr, FN_0, KType_unsigned_long_Ptr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirGetVersion), KType_Int, 3, KType_unsigned_long_Ptr, FN_0, KType_charPtr, FN_1,KType_unsigned_long_Ptr, FN_2,
		_Public, _F(KvirInitialize), KType_Int, 0, 
		_Public, _F(KvirConnectOpen), KType_virConnectPtr, 1, KType_charPtr, FN_0, 
		_Public, _F(KvirConnectOpenReadOnly), KType_virConnectPtr, 1, KType_charPtr, FN_0, 
		_Public, _F(KvirConnectOpenAuth), KType_virConnectPtr, 3, KType_charPtr, FN_0, KType_virConnectAuthPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirConnectRef), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectClose), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectGetType), KType_charPtr, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectGetVersion), KType_Int, 2, KType_virConnectPtr, FN_0, KType_unsigned_long_Ptr, FN_1,
		_Public, _F(KvirConnectGetLibVersion), KType_Int, 2, KType_virConnectPtr, FN_0, KType_unsigned_long_Ptr, FN_1,
		_Public, _F(KvirConnectGetHostname), KType_charPtr, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectGetURI), KType_charPtr, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectGetSysinfo), KType_charPtr, 2, KType_virConnectPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirConnectSetKeepAlive), KType_Int, 3, KType_virConnectPtr, FN_0, KType_Int, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirConnectRegisterCloseCallback), KType_Int, 4, KType_virConnectPtr, FN_0, KType_virConnectCloseFunc, FN_1,KType_voidPtr, FN_2,KType_virFreeCallback, FN_3,
		_Public, _F(KvirConnectUnregisterCloseCallback), KType_Int, 2, KType_virConnectPtr, FN_0, KType_virConnectCloseFunc, FN_1,
		_Public, _F(KvirConnectGetMaxVcpus), KType_Int, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNodeGetInfo), KType_Int, 2, KType_virConnectPtr, FN_0, KType_virNodeInfoPtr, FN_1,
		_Public, _F(KvirConnectGetCapabilities), KType_charPtr, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirNodeGetCPUStats), KType_Int, 5, KType_virConnectPtr, FN_0, KType_Int, FN_1,KType_virNodeCPUStatsPtr, FN_2,KType_IntPtr, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirNodeGetMemoryStats), KType_Int, 5, KType_virConnectPtr, FN_0, KType_Int, FN_1,KType_virNodeMemoryStatsPtr, FN_2,KType_IntPtr, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirNodeGetFreeMemory), KType_unsigned_long_long, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirNodeGetSecurityModel), KType_Int, 2, KType_virConnectPtr, FN_0, KType_virSecurityModelPtr, FN_1,
		_Public, _F(KvirNodeSuspendForDuration), KType_Int, 4, KType_virConnectPtr, FN_0, KType_unsigned_int, FN_1,KType_unsigned_long_long, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirConnectListDomains), KType_Int, 3, KType_virConnectPtr, FN_0, KType_IntPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectNumOfDomains), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirDomainGetConnect), KType_virConnectPtr, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainCreateXML), KType_virDomainPtr, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainLookupByName), KType_virDomainPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirDomainLookupByID), KType_virDomainPtr, 2, KType_virConnectPtr, FN_0, KType_Int, FN_1,
		_Public, _F(KvirDomainLookupByUUID), KType_virDomainPtr, 2, KType_virConnectPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirDomainLookupByUUIDString), KType_virDomainPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirDomainShutdown), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainShutdownFlags), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainReboot), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainReset), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainDestroy), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainDestroyFlags), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainRef), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainFree), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainSuspend), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainResume), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainPMSuspendForDuration), KType_Int, 4, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,KType_unsigned_long_long, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainPMWakeup), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSave), KType_Int, 2, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirDomainSaveFlags), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainRestore), KType_Int, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirDomainRestoreFlags), KType_Int, 4, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainSaveImageGetXMLDesc), KType_charPtr, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainSaveImageDefineXML), KType_Int, 4, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainManagedSave), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainHasManagedSaveImage), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainManagedSaveRemove), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainCoreDump), KType_Int, 3, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainScreenshot), KType_charPtr, 4, KType_virDomainPtr, FN_0, KType_virStreamPtr, FN_1,KType_unsigned_int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetInfo), KType_Int, 2, KType_virDomainPtr, FN_0, KType_virDomainInfoPtr, FN_1,
		_Public, _F(KvirDomainGetState), KType_Int, 4, KType_virDomainPtr, FN_0, KType_IntPtr, FN_1,KType_IntPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetCPUStats), KType_Int, 6, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_unsigned_int, FN_2,KType_Int, FN_3,KType_unsigned_int, FN_4,KType_unsigned_int, FN_5,
		_Public, _F(KvirDomainGetControlInfo), KType_Int, 3, KType_virDomainPtr, FN_0, KType_virDomainControlInfoPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainGetSchedulerType), KType_charPtr, 2, KType_virDomainPtr, FN_0, KType_IntPtr, FN_1,
		_Public, _F(KvirDomainSetBlkioParameters), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetBlkioParameters), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_IntPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainSetMemoryParameters), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetMemoryParameters), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_IntPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainSetNumaParameters), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetNumaParameters), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_IntPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetName), KType_charPtr, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainGetID), KType_unsigned_int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainGetUUID), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirDomainGetUUIDString), KType_Int, 2, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirDomainGetOSType), KType_charPtr, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainGetMaxMemory), KType_unsigned_long, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainSetMaxMemory), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_long, FN_1,
		_Public, _F(KvirDomainSetMemory), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_long, FN_1,
		_Public, _F(KvirDomainSetMemoryFlags), KType_Int, 3, KType_virDomainPtr, FN_0, KType_unsigned_long, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainGetMaxVcpus), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainGetSecurityLabel), KType_Int, 2, KType_virDomainPtr, FN_0, KType_virSecurityLabelPtr, FN_1,
		_Public, _F(KvirDomainGetHostname), KType_charPtr, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainGetSecurityLabelList), KType_Int, 2, KType_virDomainPtr, FN_0, KType_virSecurityLabelPtrPtr, FN_1,
		_Public, _F(KvirDomainSetMetadata), KType_Int, 6, KType_virDomainPtr, FN_0, KType_Int, FN_1,KType_charPtr, FN_2,KType_charPtr, FN_3,KType_charPtr, FN_4,KType_unsigned_int, FN_5,
		_Public, _F(KvirDomainGetMetadata), KType_charPtr, 4, KType_virDomainPtr, FN_0, KType_Int, FN_1,KType_charPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetXMLDesc), KType_charPtr, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirConnectDomainXMLFromNative), KType_charPtr, 4, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirConnectDomainXMLToNative), KType_charPtr, 4, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainNodeStats), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virDomainNodeStatsPtr, FN_2,KType_size_t, FN_3,
		_Public, _F(KvirDomainNodeStatsFlags), KType_Int, 5, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virTypedParameterPtr, FN_2,KType_IntPtr, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainInterfaceStats), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virDomainInterfaceStatsPtr, FN_2,KType_size_t, FN_3,
		_Public, _F(KvirDomainSetInterfaceParameters), KType_Int, 5, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virTypedParameterPtr, FN_2,KType_Int, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainGetInterfaceParameters), KType_Int, 5, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virTypedParameterPtr, FN_2,KType_IntPtr, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainNodePeek), KType_Int, 6, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_long_long, FN_2,KType_size_t, FN_3,KType_voidPtr, FN_4,KType_unsigned_int, FN_5,
		_Public, _F(KvirDomainNodeResize), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_long_long, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetNodeInfo), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virDomainNodeInfoPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainMemoryStats), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virDomainMemoryStatPtr, FN_1,KType_unsigned_int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainMemoryPeek), KType_Int, 5, KType_virDomainPtr, FN_0, KType_unsigned_long_long, FN_1,KType_size_t, FN_2,KType_voidPtr, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainDefineXML), KType_virDomainPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirDomainUndefine), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainUndefineFlags), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirConnectNumOfDefinedDomains), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListDefinedDomains), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectListAllDomains), KType_Int, 3, KType_virConnectPtr, FN_0, KType_virDomainPtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainCreate), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainCreateWithFlags), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainGetAutostart), KType_Int, 2, KType_virDomainPtr, FN_0, KType_IntPtr, FN_1,
		_Public, _F(KvirDomainSetAutostart), KType_Int, 2, KType_virDomainPtr, FN_0, KType_Int, FN_1,
		_Public, _F(KvirDomainSetVcpus), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSetVcpusFlags), KType_Int, 3, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainGetVcpusFlags), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainPinVcpu), KType_Int, 4, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,KType_unsigned_charPtr, FN_2,KType_Int, FN_3,
		_Public, _F(KvirDomainPinVcpuFlags), KType_Int, 5, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,KType_unsigned_charPtr, FN_2,KType_Int, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainGetVcpuPinInfo), KType_Int, 5, KType_virDomainPtr, FN_0, KType_Int, FN_1,KType_unsigned_charPtr, FN_2,KType_Int, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainPinEmulator), KType_Int, 4, KType_virDomainPtr, FN_0, KType_unsigned_charPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetEmulatorPinInfo), KType_Int, 4, KType_virDomainPtr, FN_0, KType_unsigned_charPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetVcpus), KType_Int, 5, KType_virDomainPtr, FN_0, KType_virVcpuInfoPtr, FN_1,KType_Int, FN_2,KType_unsigned_charPtr, FN_3,KType_Int, FN_4,
		_Public, _F(KvirDomainAttachDevice), KType_Int, 2, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirDomainDetachDevice), KType_Int, 2, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirDomainAttachDeviceFlags), KType_Int, 3, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainDetachDeviceFlags), KType_Int, 3, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainUpdateDeviceFlags), KType_Int, 3, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainNodeJobAbort), KType_Int, 3, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainGetNodeJobInfo), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virDomainNodeJobInfoPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainNodeJobSetSpeed), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_long, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainNodePull), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_long, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainNodeRebase), KType_Int, 5, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_unsigned_long, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainNodeCommit), KType_Int, 6, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_charPtr, FN_3,KType_unsigned_long, FN_4,KType_unsigned_int, FN_5,
		_Public, _F(KvirDomainSetNodeIoTune), KType_Int, 5, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virTypedParameterPtr, FN_2,KType_Int, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainGetNodeIoTune), KType_Int, 5, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virTypedParameterPtr, FN_2,KType_IntPtr, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirDomainGetDiskErrors), KType_Int, 4, KType_virDomainPtr, FN_0, KType_virDomainDiskErrorPtr, FN_1,KType_unsigned_int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirNodeGetCellsFreeMemory), KType_Int, 4, KType_virConnectPtr, FN_0, KType_unsigned_long_long_Ptr, FN_1,KType_Int, FN_2,KType_Int, FN_3,
		_Public, _F(KvirNetworkGetConnect), KType_virConnectPtr, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirConnectNumOfNetworks), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListNetworks), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectNumOfDefinedNetworks), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListDefinedNetworks), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectListAllNetworks), KType_Int, 3, KType_virConnectPtr, FN_0, KType_virNetworkPtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirNetworkLookupByName), KType_virNetworkPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNetworkLookupByUUID), KType_virNetworkPtr, 2, KType_virConnectPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirNetworkLookupByUUIDString), KType_virNetworkPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNetworkCreateXML), KType_virNetworkPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNetworkDefineXML), KType_virNetworkPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNetworkUndefine), KType_Int, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirNetworkUpdate), KType_Int, 6, KType_virNetworkPtr, FN_0, KType_unsigned_int, FN_1,KType_unsigned_int, FN_2,KType_Int, FN_3,KType_charPtr, FN_4,KType_unsigned_int, FN_5,
		_Public, _F(KvirNetworkCreate), KType_Int, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirNetworkDestroy), KType_Int, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirNetworkRef), KType_Int, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirNetworkFree), KType_Int, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirNetworkGetName), KType_charPtr, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirNetworkGetUUID), KType_Int, 2, KType_virNetworkPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirNetworkGetUUIDString), KType_Int, 2, KType_virNetworkPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNetworkGetXMLDesc), KType_charPtr, 2, KType_virNetworkPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirNetworkGetBridgeName), KType_charPtr, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirNetworkGetAutostart), KType_Int, 2, KType_virNetworkPtr, FN_0, KType_IntPtr, FN_1,
		_Public, _F(KvirNetworkSetAutostart), KType_Int, 2, KType_virNetworkPtr, FN_0, KType_Int, FN_1,
		_Public, _F(KvirInterfaceGetConnect), KType_virConnectPtr, 1, KType_virInterfacePtr, FN_0, 
		_Public, _F(KvirConnectNumOfInterfaces), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListInterfaces), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectNumOfDefinedInterfaces), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListDefinedInterfaces), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectListAllInterfaces), KType_Int, 3, KType_virConnectPtr, FN_0, KType_virInterfacePtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirInterfaceLookupByName), KType_virInterfacePtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirInterfaceLookupByMACString), KType_virInterfacePtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirInterfaceGetName), KType_charPtr, 1, KType_virInterfacePtr, FN_0, 
		_Public, _F(KvirInterfaceGetMACString), KType_charPtr, 1, KType_virInterfacePtr, FN_0, 
		_Public, _F(KvirInterfaceGetXMLDesc), KType_charPtr, 2, KType_virInterfacePtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirInterfaceDefineXML), KType_virInterfacePtr, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirInterfaceUndefine), KType_Int, 1, KType_virInterfacePtr, FN_0, 
		_Public, _F(KvirInterfaceCreate), KType_Int, 2, KType_virInterfacePtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirInterfaceDestroy), KType_Int, 2, KType_virInterfacePtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirInterfaceRef), KType_Int, 1, KType_virInterfacePtr, FN_0, 
		_Public, _F(KvirInterfaceFree), KType_Int, 1, KType_virInterfacePtr, FN_0, 
		_Public, _F(KvirInterfaceChangeBegin), KType_Int, 2, KType_virConnectPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirInterfaceChangeCommit), KType_Int, 2, KType_virConnectPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirInterfaceChangeRollback), KType_Int, 2, KType_virConnectPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStoragePoolGetConnect), KType_virConnectPtr, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirConnectNumOfStoragePools), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListStoragePools), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectNumOfDefinedStoragePools), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListDefinedStoragePools), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectListAllStoragePools), KType_Int, 3, KType_virConnectPtr, FN_0, KType_virStoragePoolPtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirConnectFindStoragePoolSources), KType_charPtr, 4, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_charPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirStoragePoolLookupByName), KType_virStoragePoolPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirStoragePoolLookupByUUID), KType_virStoragePoolPtr, 2, KType_virConnectPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirStoragePoolLookupByUUIDString), KType_virStoragePoolPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirStoragePoolLookupByVolume), KType_virStoragePoolPtr, 1, KType_virStorageVolPtr, FN_0, 
		_Public, _F(KvirStoragePoolCreateXML), KType_virStoragePoolPtr, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirStoragePoolDefineXML), KType_virStoragePoolPtr, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirStoragePoolBuild), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStoragePoolUndefine), KType_Int, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirStoragePoolCreate), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStoragePoolDestroy), KType_Int, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirStoragePoolDelete), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStoragePoolRef), KType_Int, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirStoragePoolFree), KType_Int, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirStoragePoolRefresh), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStoragePoolGetName), KType_charPtr, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirStoragePoolGetUUID), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirStoragePoolGetUUIDString), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirStoragePoolGetInfo), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_virStoragePoolInfoPtr, FN_1,
		_Public, _F(KvirStoragePoolGetXMLDesc), KType_charPtr, 2, KType_virStoragePoolPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStoragePoolGetAutostart), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_IntPtr, FN_1,
		_Public, _F(KvirStoragePoolSetAutostart), KType_Int, 2, KType_virStoragePoolPtr, FN_0, KType_Int, FN_1,
		_Public, _F(KvirStoragePoolNumOfVolumes), KType_Int, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirStoragePoolListVolumes), KType_Int, 3, KType_virStoragePoolPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirStoragePoolListAllVolumes), KType_Int, 3, KType_virStoragePoolPtr, FN_0, KType_virStorageVolPtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirStorageVolGetConnect), KType_virConnectPtr, 1, KType_virStorageVolPtr, FN_0, 
		_Public, _F(KvirStorageVolLookupByName), KType_virStorageVolPtr, 2, KType_virStoragePoolPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirStorageVolLookupByKey), KType_virStorageVolPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirStorageVolLookupByPath), KType_virStorageVolPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirStorageVolGetName), KType_charPtr, 1, KType_virStorageVolPtr, FN_0, 
		_Public, _F(KvirStorageVolGetKey), KType_charPtr, 1, KType_virStorageVolPtr, FN_0, 
		_Public, _F(KvirStorageVolCreateXML), KType_virStorageVolPtr, 3, KType_virStoragePoolPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirStorageVolCreateXMLFrom), KType_virStorageVolPtr, 4, KType_virStoragePoolPtr, FN_0, KType_charPtr, FN_1,KType_virStorageVolPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirStorageVolDownload), KType_Int, 5, KType_virStorageVolPtr, FN_0, KType_virStreamPtr, FN_1,KType_unsigned_long_long, FN_2,KType_unsigned_long_long, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirStorageVolUpload), KType_Int, 5, KType_virStorageVolPtr, FN_0, KType_virStreamPtr, FN_1,KType_unsigned_long_long, FN_2,KType_unsigned_long_long, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirStorageVolDelete), KType_Int, 2, KType_virStorageVolPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStorageVolWipe), KType_Int, 2, KType_virStorageVolPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStorageVolWipePattern), KType_Int, 3, KType_virStorageVolPtr, FN_0, KType_unsigned_int, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirStorageVolRef), KType_Int, 1, KType_virStorageVolPtr, FN_0, 
		_Public, _F(KvirStorageVolFree), KType_Int, 1, KType_virStorageVolPtr, FN_0, 
		_Public, _F(KvirStorageVolGetInfo), KType_Int, 2, KType_virStorageVolPtr, FN_0, KType_virStorageVolInfoPtr, FN_1,
		_Public, _F(KvirStorageVolGetXMLDesc), KType_charPtr, 2, KType_virStorageVolPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStorageVolGetPath), KType_charPtr, 1, KType_virStorageVolPtr, FN_0, 
		_Public, _F(KvirStorageVolResize), KType_Int, 3, KType_virStorageVolPtr, FN_0, KType_unsigned_long_long, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainSendKey), KType_Int, 6, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,KType_unsigned_int, FN_2,KType_unsigned_intPtr, FN_3,KType_Int, FN_4,KType_unsigned_int, FN_5,
		_Public, _F(KvirDomainCreateLinux), KType_virDomainPtr, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirNodeNumOfDevices), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirNodeListDevices), KType_Int, 5, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_charPtrPtr, FN_2,KType_Int, FN_3,KType_unsigned_int, FN_4,
		_Public, _F(KvirConnectListAllNodeDevices), KType_Int, 3, KType_virConnectPtr, FN_0, KType_virNodeDevicePtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirNodeDeviceLookupByName), KType_virNodeDevicePtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNodeDeviceGetName), KType_charPtr, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirNodeDeviceGetParent), KType_charPtr, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirNodeDeviceNumOfCaps), KType_Int, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirNodeDeviceListCaps), KType_Int, 3, KType_virNodeDevicePtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirNodeDeviceGetXMLDesc), KType_charPtr, 2, KType_virNodeDevicePtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirNodeDeviceRef), KType_Int, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirNodeDeviceFree), KType_Int, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirNodeDeviceDettach), KType_Int, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirNodeDeviceReAttach), KType_Int, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirNodeDeviceReset), KType_Int, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirNodeDeviceCreateXML), KType_virNodeDevicePtr, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirNodeDeviceDestroy), KType_Int, 1, KType_virNodeDevicePtr, FN_0, 
		_Public, _F(KvirConnectDomainEventRegister), KType_Int, 4, KType_virConnectPtr, FN_0, KType_virConnectDomainEventCallback, FN_1,KType_voidPtr, FN_2,KType_virFreeCallback, FN_3,
		_Public, _F(KvirConnectDomainEventDeregister), KType_Int, 2, KType_virConnectPtr, FN_0, KType_virConnectDomainEventCallback, FN_1,
		_Public, _F(KvirEventRegisterImpl), KType_void, 6, KType_virEventAddHandleFunc, FN_0, KType_virEventUpdateHandleFunc, FN_1,KType_virEventRemoveHandleFunc, FN_2,KType_virEventAddTimeoutFunc, FN_3,KType_virEventUpdateTimeoutFunc, FN_4,KType_virEventRemoveTimeoutFunc, FN_5,
		_Public, _F(KvirEventRegisterDefaultImpl), KType_Int, 0, 
		_Public, _F(KvirEventRunDefaultImpl), KType_Int, 0, 
		_Public, _F(KvirEventAddHandle), KType_Int, 5, KType_Int, FN_0, KType_Int, FN_1,KType_virEventHandleCallback, FN_2,KType_voidPtr, FN_3,KType_virFreeCallback, FN_4,
		_Public, _F(KvirEventUpdateHandle), KType_void, 2, KType_Int, FN_0, KType_Int, FN_1,
		_Public, _F(KvirEventRemoveHandle), KType_Int, 1, KType_Int, FN_0, 
		_Public, _F(KvirEventAddTimeout), KType_Int, 4, KType_Int, FN_0, KType_virEventTimeoutCallback, FN_1,KType_voidPtr, FN_2,KType_virFreeCallback, FN_3,
		_Public, _F(KvirEventUpdateTimeout), KType_void, 2, KType_Int, FN_0, KType_Int, FN_1,
		_Public, _F(KvirEventRemoveTimeout), KType_Int, 1, KType_Int, FN_0, 
		_Public, _F(KvirSecretGetConnect), KType_virConnectPtr, 1, KType_virSecretPtr, FN_0, 
		_Public, _F(KvirConnectNumOfSecrets), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListSecrets), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectListAllSecrets), KType_Int, 3, KType_virConnectPtr, FN_0, KType_virSecretPtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirSecretLookupByUUID), KType_virSecretPtr, 2, KType_virConnectPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirSecretLookupByUUIDString), KType_virSecretPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirSecretLookupByUsage), KType_virSecretPtr, 3, KType_virConnectPtr, FN_0, KType_Int, FN_1,KType_charPtr, FN_2,
		_Public, _F(KvirSecretDefineXML), KType_virSecretPtr, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirSecretGetUUID), KType_Int, 2, KType_virSecretPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirSecretGetUUIDString), KType_Int, 2, KType_virSecretPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirSecretGetUsageType), KType_Int, 1, KType_virSecretPtr, FN_0, 
		_Public, _F(KvirSecretGetUsageID), KType_charPtr, 1, KType_virSecretPtr, FN_0, 
		_Public, _F(KvirSecretGetXMLDesc), KType_charPtr, 2, KType_virSecretPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirSecretSetValue), KType_Int, 4, KType_virSecretPtr, FN_0, KType_unsigned_charPtr, FN_1,KType_size_t, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirSecretGetValue), KType_unsigned_charPtr, 3, KType_virSecretPtr, FN_0, KType_size_tPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirSecretUndefine), KType_Int, 1, KType_virSecretPtr, FN_0, 
		_Public, _F(KvirSecretRef), KType_Int, 1, KType_virSecretPtr, FN_0, 
		_Public, _F(KvirSecretFree), KType_Int, 1, KType_virSecretPtr, FN_0, 
		_Public, _F(KvirStreamNew), KType_virStreamPtr, 2, KType_virConnectPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirStreamRef), KType_Int, 1, KType_virStreamPtr, FN_0, 
		_Public, _F(KvirStreamSend), KType_Int, 3, KType_virStreamPtr, FN_0, KType_charPtr, FN_1,KType_size_t, FN_2,
		_Public, _F(KvirStreamRecv), KType_Int, 3, KType_virStreamPtr, FN_0, KType_charPtr, FN_1,KType_size_t, FN_2,
		_Public, _F(KvirStreamSendAll), KType_Int, 3, KType_virStreamPtr, FN_0, KType_virStreamSourceFunc, FN_1,KType_voidPtr, FN_2,
		_Public, _F(KvirStreamRecvAll), KType_Int, 3, KType_virStreamPtr, FN_0, KType_virStreamSinkFunc, FN_1,KType_voidPtr, FN_2,
		_Public, _F(KvirStreamEventAddCallback), KType_Int, 5, KType_virStreamPtr, FN_0, KType_Int, FN_1,KType_virStreamEventCallback, FN_2,KType_voidPtr, FN_3,KType_virFreeCallback, FN_4,
		_Public, _F(KvirStreamEventUpdateCallback), KType_Int, 2, KType_virStreamPtr, FN_0, KType_Int, FN_1,
		_Public, _F(KvirStreamEventRemoveCallback), KType_Int, 1, KType_virStreamPtr, FN_0, 
		_Public, _F(KvirStreamFinish), KType_Int, 1, KType_virStreamPtr, FN_0, 
		_Public, _F(KvirStreamAbort), KType_Int, 1, KType_virStreamPtr, FN_0, 
		_Public, _F(KvirStreamFree), KType_Int, 1, KType_virStreamPtr, FN_0, 
		_Public, _F(KvirDomainIsActive), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainIsPersistent), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainIsUpdated), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirNetworkIsActive), KType_Int, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirNetworkIsPersistent), KType_Int, 1, KType_virNetworkPtr, FN_0, 
		_Public, _F(KvirStoragePoolIsActive), KType_Int, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirStoragePoolIsPersistent), KType_Int, 1, KType_virStoragePoolPtr, FN_0, 
		_Public, _F(KvirInterfaceIsActive), KType_Int, 1, KType_virInterfacePtr, FN_0, 
		_Public, _F(KvirConnectIsEncrypted), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectIsSecure), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectIsAlive), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectCompareCPU), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirConnectBaselineCPU), KType_charPtr, 4, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_unsigned_int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainGetJobInfo), KType_Int, 2, KType_virDomainPtr, FN_0, KType_virDomainJobInfoPtr, FN_1,
		_Public, _F(KvirDomainAbortJob), KType_Int, 1, KType_virDomainPtr, FN_0, 
		_Public, _F(KvirDomainSnapshotGetName), KType_charPtr, 1, KType_virDomainSnapshotPtr, FN_0, 
		_Public, _F(KvirDomainSnapshotGetDomain), KType_virDomainPtr, 1, KType_virDomainSnapshotPtr, FN_0, 
		_Public, _F(KvirDomainSnapshotGetConnect), KType_virConnectPtr, 1, KType_virDomainSnapshotPtr, FN_0, 
		_Public, _F(KvirDomainSnapshotCreateXML), KType_virDomainSnapshotPtr, 3, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainSnapshotGetXMLDesc), KType_charPtr, 2, KType_virDomainSnapshotPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotNum), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotListNames), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainListAllSnapshots), KType_Int, 3, KType_virDomainPtr, FN_0, KType_virDomainSnapshotPtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainSnapshotNumChildren), KType_Int, 2, KType_virDomainSnapshotPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotListChildrenNames), KType_Int, 4, KType_virDomainSnapshotPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainSnapshotListAllChildren), KType_Int, 3, KType_virDomainSnapshotPtr, FN_0, KType_virDomainSnapshotPtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainSnapshotLookupByName), KType_virDomainSnapshotPtr, 3, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirDomainHasCurrentSnapshot), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotCurrent), KType_virDomainSnapshotPtr, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotGetParent), KType_virDomainSnapshotPtr, 2, KType_virDomainSnapshotPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotIsCurrent), KType_Int, 2, KType_virDomainSnapshotPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotHasMetadata), KType_Int, 2, KType_virDomainSnapshotPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainRevertToSnapshot), KType_Int, 2, KType_virDomainSnapshotPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotDelete), KType_Int, 2, KType_virDomainSnapshotPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainSnapshotRef), KType_Int, 1, KType_virDomainSnapshotPtr, FN_0, 
		_Public, _F(KvirDomainSnapshotFree), KType_Int, 1, KType_virDomainSnapshotPtr, FN_0, 
		_Public, _F(KvirConnectDomainEventRegisterAny), KType_Int, 6, KType_virConnectPtr, FN_0, KType_virDomainPtr, FN_1,KType_Int, FN_2,KType_virConnectDomainEventGenericCallback, FN_3,KType_voidPtr, FN_4,KType_virFreeCallback, FN_5,
		_Public, _F(KvirConnectDomainEventDeregisterAny), KType_Int, 2, KType_virConnectPtr, FN_0, KType_Int, FN_1,
		_Public, _F(KvirConnectNumOfNWFilters), KType_Int, 1, KType_virConnectPtr, FN_0, 
		_Public, _F(KvirConnectListNWFilters), KType_Int, 3, KType_virConnectPtr, FN_0, KType_charPtrPtr, FN_1,KType_Int, FN_2,
		_Public, _F(KvirConnectListAllNWFilters), KType_Int, 3, KType_virConnectPtr, FN_0, KType_virNWFilterPtrPtrPtr, FN_1,KType_unsigned_int, FN_2,
		_Public, _F(KvirNWFilterLookupByName), KType_virNWFilterPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNWFilterLookupByUUID), KType_virNWFilterPtr, 2, KType_virConnectPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirNWFilterLookupByUUIDString), KType_virNWFilterPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNWFilterDefineXML), KType_virNWFilterPtr, 2, KType_virConnectPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNWFilterUndefine), KType_Int, 1, KType_virNWFilterPtr, FN_0, 
		_Public, _F(KvirNWFilterRef), KType_Int, 1, KType_virNWFilterPtr, FN_0, 
		_Public, _F(KvirNWFilterFree), KType_Int, 1, KType_virNWFilterPtr, FN_0, 
		_Public, _F(KvirNWFilterGetName), KType_charPtr, 1, KType_virNWFilterPtr, FN_0, 
		_Public, _F(KvirNWFilterGetUUID), KType_Int, 2, KType_virNWFilterPtr, FN_0, KType_unsigned_charPtr, FN_1,
		_Public, _F(KvirNWFilterGetUUIDString), KType_Int, 2, KType_virNWFilterPtr, FN_0, KType_charPtr, FN_1,
		_Public, _F(KvirNWFilterGetXMLDesc), KType_charPtr, 2, KType_virNWFilterPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirDomainOpenConsole), KType_Int, 4, KType_virDomainPtr, FN_0, KType_charPtr, FN_1,KType_virStreamPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainOpenGraphics), KType_Int, 4, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirDomainInjectNMI), KType_Int, 2, KType_virDomainPtr, FN_0, KType_unsigned_int, FN_1,
		_Public, _F(KvirNodeGetMemoryParameters), KType_Int, 4, KType_virConnectPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_IntPtr, FN_2,KType_unsigned_int, FN_3,
		_Public, _F(KvirNodeSetMemoryParameters), KType_Int, 4, KType_virConnectPtr, FN_0, KType_virTypedParameterPtr, FN_1,KType_Int, FN_2,KType_unsigned_int, FN_3,
		DEND,
	};
	/* You can define constant variable with the following procedures. */
	KDEFINE_INT_CONST IntData[] = {
		{"VIR_DOMAIN_REBOOT_DEFAULT", KType_Int, VIR_DOMAIN_REBOOT_DEFAULT},
		{"VIR_DOMAIN_REBOOT_ACPI_POWER_BTN", KType_Int, VIR_DOMAIN_REBOOT_ACPI_POWER_BTN},
		{"VIR_DOMAIN_REBOOT_GUEST_AGENT", KType_Int, VIR_DOMAIN_REBOOT_GUEST_AGENT},
		{} /* <= sentinel */
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KConst_(IntData), trace);

	return true;
}
static kbool_t PACKAGE_SETUP_PKG(LIBVIRT_PACKAGE_NAME)(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *PACKAGE_INIT(LIBVIRT_PACKAGE_NAME)(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, LIBVIRT_PACKAGE_NAME, "1.0");
	d.PackupNameSpace  = PACKAGE_INIT_PKG(LIBVIRT_PACKAGE_NAME);
	d.ExportNameSpace = PACKAGE_SETUP_PKG(LIBVIRT_PACKAGE_NAME);
	return &d;
}
