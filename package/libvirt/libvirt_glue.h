#ifndef LIBVIRT_GLUE_H
#define LIBVIRT_GLUE_H

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include "libvirt/libvirt.h"
#include "libvirt/libvirt.h"

#include "type_cast.h"

/* TODO 
 * Declare fundamental data types as konoha object, such as void*
 * */
#define TY_voidPtr 0
#define TY_unsigned_charPtr 0
#define TY_size_t 0
#define TY_charPtrPtr 0
#define TY_unsigned_long_long_Ptr 0
#define TY_unsigned_intPtr 0
#define TY_size_tPtr 0

typedef int  *intPtr;
typedef void *voidPtr;
typedef size_t *size_tPtr;
typedef const char *charPtr;
typedef const char **charPtrPtr;
typedef const char *unsigned_charPtr;
typedef unsigned int  *unsigned_intPtr;
typedef unsigned long *unsigned_long_Ptr;
typedef unsigned_long_Ptr unsigned_long_long_Ptr;

typedef kuint_t unsigned_long;
typedef kuint_t unsigned_long_long;
typedef kuint_t unsigned_int;
#define TY_unsigned_long         TY_int
#define TY_unsigned_long_long    TY_int
#define TY_unsigned_int          TY_int

//#define To_String(sfp)  sfp.asString
//#define To_int(sfp)     sfp.intValue
//#define To_char(sfp)    ((char)sfp.intValue)
//#define To_boolean(sfp) sfp.boolValue
//#define To_float(sfp)   sfp.floatValue
//#define To_Locale(sfp)  sfp.asString
//typedef kString *String;
//typedef kArray  *Array;
//typedef String Locale;
//typedef bool boolean;
//#define RETURNchar(RET)    KReturnInt(RET)
//#define RETURNboolean(RET) KReturnUnboxValue(RET)
//#define RETURNString(RET)  KReturn(RET)
//#define RETURNArray(RET)   KReturn(RET)
//#define TYPE_Array(TYPE)      (CT_p0(kctx, CT_Array, TY_##TYPE))->typeId
//#define TY_char            TY_int
//#define TY_Locale          TY_String
#define RETURNint(RET)     KReturnInt(RET)
#define RETURNvirDomainPtr(RET) KReturn(KLIB new_kObject(kctx, OnStack, CT_(TY_virDomainPtr), (uintptr_t)RET))
#define RETURNvirConnectPtr(RET) KReturn(KLIB new_kObject(kctx, OnStack, CT_(TY_virConnectPtr), (uintptr_t)RET))
#define RETURNcharPtr(RET) KReturn(KLIB new_kObject(kctx, OnStack, CT_(TY_charPtr), (uintptr_t)RET))

#define LIBVIRT_PACKAGE_NAME "libvirt"

#define PACKAGE_INIT(NAME)      NAME##_init
#define PACKAGE_INIT_PKG(NAME)  NAME##_initPackage
#define PACKAGE_SETUP_PKG(NAME) NAME##_setupPackage
#define PACKAGE_TOSTRING(NAME) "" # NAME

typedef struct kvirCommonPtr {
	KonohaObjectHeader h;
	void *p;
}kvirCommonPtr;

static void virt_common_ptr_init(KonohaContext *kctx, kObject *o, void *conf) {
	kvirCommonPtr *ptr = (kvirCommonPtr *)o;
	ptr->p = conf;
};

static void virt_common_ptr_free(KonohaContext *kctx, kObject *o) {
	kvirCommonPtr *ptr = (kvirCommonPtr *)o;
	free(ptr->p);
};

typedef struct kvirConnectAuthPtr {
	KonohaObjectHeader h;
	virConnectAuthPtr p;
}kvirConnectAuthPtr;
static int TY_virConnectAuthPtr;

typedef struct kvirConnectCloseFunc {
	KonohaObjectHeader h;
	virConnectCloseFunc p;
}kvirConnectCloseFunc;
static int TY_virConnectCloseFunc;

typedef struct kvirConnectDomainEventCallback {
	KonohaObjectHeader h;
	virConnectDomainEventCallback p;
}kvirConnectDomainEventCallback;
static int TY_virConnectDomainEventCallback;

typedef struct kvirConnectDomainEventGenericCallback {
	KonohaObjectHeader h;
	virConnectDomainEventGenericCallback p;
}kvirConnectDomainEventGenericCallback;
static int TY_virConnectDomainEventGenericCallback;

typedef struct kvirConnectPtr {
	KonohaObjectHeader h;
	virConnectPtr p;
}kvirConnectPtr;
static int TY_virConnectPtr;

typedef struct kvirDomainBlockInfoPtr {
	KonohaObjectHeader h;
	virDomainBlockInfoPtr p;
}kvirDomainBlockInfoPtr;
static int TY_virDomainBlockInfoPtr;

typedef struct kvirDomainBlockJobInfoPtr {
	KonohaObjectHeader h;
	virDomainBlockJobInfoPtr p;
}kvirDomainBlockJobInfoPtr;
static int TY_virDomainBlockJobInfoPtr;

typedef struct kvirDomainBlockStatsPtr {
	KonohaObjectHeader h;
	virDomainBlockStatsPtr p;
}kvirDomainBlockStatsPtr;
static int TY_virDomainBlockStatsPtr;

typedef struct kvirDomainControlInfoPtr {
	KonohaObjectHeader h;
	virDomainControlInfoPtr p;
}kvirDomainControlInfoPtr;
static int TY_virDomainControlInfoPtr;

typedef struct kvirDomainDiskErrorPtr {
	KonohaObjectHeader h;
	virDomainDiskErrorPtr p;
}kvirDomainDiskErrorPtr;
static int TY_virDomainDiskErrorPtr;

typedef struct kvirDomainInfoPtr {
	KonohaObjectHeader h;
	virDomainInfoPtr p;
}kvirDomainInfoPtr;
static int TY_virDomainInfoPtr;

typedef struct kvirDomainInterfaceStatsPtr {
	KonohaObjectHeader h;
	virDomainInterfaceStatsPtr p;
}kvirDomainInterfaceStatsPtr;
static int TY_virDomainInterfaceStatsPtr;

typedef struct kvirDomainJobInfoPtr {
	KonohaObjectHeader h;
	virDomainJobInfoPtr p;
}kvirDomainJobInfoPtr;
static int TY_virDomainJobInfoPtr;

typedef struct kvirDomainMemoryStatPtr {
	KonohaObjectHeader h;
	virDomainMemoryStatPtr p;
}kvirDomainMemoryStatPtr;
static int TY_virDomainMemoryStatPtr;

typedef struct kvirDomainPtr {
	KonohaObjectHeader h;
	virDomainPtr p;
}kvirDomainPtr;
static int TY_virDomainPtr;

typedef struct virDomainPtrPtr *virDomainPtrPtrPtr;
typedef struct kvirDomainPtrPtrPtr {
	KonohaObjectHeader h;
	virDomainPtrPtrPtr p;
}kvirDomainPtrPtrPtr;
static int TY_virDomainPtrPtrPtr;

typedef struct kvirDomainSnapshotPtr {
	KonohaObjectHeader h;
	virDomainSnapshotPtr p;
}kvirDomainSnapshotPtr;
static int TY_virDomainSnapshotPtr;

typedef struct virDomainSnapshotPtrPtr *virDomainSnapshotPtrPtrPtr;
typedef struct kvirDomainSnapshotPtrPtrPtr {
	KonohaObjectHeader h;
	virDomainSnapshotPtrPtrPtr p;
}kvirDomainSnapshotPtrPtrPtr;
static int TY_virDomainSnapshotPtrPtrPtr;

typedef struct kvirEventAddHandleFunc {
	KonohaObjectHeader h;
	virEventAddHandleFunc p;
}kvirEventAddHandleFunc;
static int TY_virEventAddHandleFunc;

typedef struct kvirEventAddTimeoutFunc {
	KonohaObjectHeader h;
	virEventAddTimeoutFunc p;
}kvirEventAddTimeoutFunc;
static int TY_virEventAddTimeoutFunc;

typedef struct kvirEventHandleCallback {
	KonohaObjectHeader h;
	virEventHandleCallback p;
}kvirEventHandleCallback;
static int TY_virEventHandleCallback;

typedef struct kvirEventRemoveHandleFunc {
	KonohaObjectHeader h;
	virEventRemoveHandleFunc p;
}kvirEventRemoveHandleFunc;
static int TY_virEventRemoveHandleFunc;

typedef struct kvirEventRemoveTimeoutFunc {
	KonohaObjectHeader h;
	virEventRemoveTimeoutFunc p;
}kvirEventRemoveTimeoutFunc;
static int TY_virEventRemoveTimeoutFunc;

typedef struct kvirEventTimeoutCallback {
	KonohaObjectHeader h;
	virEventTimeoutCallback p;
}kvirEventTimeoutCallback;
static int TY_virEventTimeoutCallback;

typedef struct kvirEventUpdateHandleFunc {
	KonohaObjectHeader h;
	virEventUpdateHandleFunc p;
}kvirEventUpdateHandleFunc;
static int TY_virEventUpdateHandleFunc;

typedef struct kvirEventUpdateTimeoutFunc {
	KonohaObjectHeader h;
	virEventUpdateTimeoutFunc p;
}kvirEventUpdateTimeoutFunc;
static int TY_virEventUpdateTimeoutFunc;

typedef struct kvirFreeCallback {
	KonohaObjectHeader h;
	virFreeCallback p;
}kvirFreeCallback;
static int TY_virFreeCallback;

typedef struct kvirInterfacePtr {
	KonohaObjectHeader h;
	virInterfacePtr p;
}kvirInterfacePtr;
static int TY_virInterfacePtr;

typedef struct virInterfacePtrPtr *virInterfacePtrPtrPtr;
typedef struct kvirInterfacePtrPtrPtr {
	KonohaObjectHeader h;
	virInterfacePtrPtrPtr p;
}kvirInterfacePtrPtrPtr;
static int TY_virInterfacePtrPtrPtr;

typedef struct kvirNWFilterPtr {
	KonohaObjectHeader h;
	virNWFilterPtr p;
}kvirNWFilterPtr;
static int TY_virNWFilterPtr;

typedef struct virNWFilterPtrPtr *virNWFilterPtrPtrPtr;
typedef struct kvirNWFilterPtrPtrPtr {
	KonohaObjectHeader h;
	virNWFilterPtrPtrPtr p;
}kvirNWFilterPtrPtrPtr;
static int TY_virNWFilterPtrPtrPtr;

typedef struct kvirNetworkPtr {
	KonohaObjectHeader h;
	virNetworkPtr p;
}kvirNetworkPtr;
static int TY_virNetworkPtr;

typedef struct virNetworkPtrPtr *virNetworkPtrPtrPtr;
typedef struct kvirNetworkPtrPtrPtr {
	KonohaObjectHeader h;
	virNetworkPtrPtrPtr p;
}kvirNetworkPtrPtrPtr;
static int TY_virNetworkPtrPtrPtr;

typedef struct kvirNodeCPUStatsPtr {
	KonohaObjectHeader h;
	virNodeCPUStatsPtr p;
}kvirNodeCPUStatsPtr;
static int TY_virNodeCPUStatsPtr;

typedef struct kvirNodeDevicePtr {
	KonohaObjectHeader h;
	virNodeDevicePtr p;
}kvirNodeDevicePtr;
static int TY_virNodeDevicePtr;

typedef struct virNodeDevicePtrPtr *virNodeDevicePtrPtrPtr;
typedef struct kvirNodeDevicePtrPtrPtr {
	KonohaObjectHeader h;
	virNodeDevicePtrPtrPtr p;
}kvirNodeDevicePtrPtrPtr;
static int TY_virNodeDevicePtrPtrPtr;

typedef struct kvirNodeInfoPtr {
	KonohaObjectHeader h;
	virNodeInfoPtr p;
}kvirNodeInfoPtr;
static int TY_virNodeInfoPtr;

typedef struct kvirNodeMemoryStatsPtr {
	KonohaObjectHeader h;
	virNodeMemoryStatsPtr p;
}kvirNodeMemoryStatsPtr;
static int TY_virNodeMemoryStatsPtr;

typedef struct kvirSecretPtr {
	KonohaObjectHeader h;
	virSecretPtr p;
}kvirSecretPtr;
static int TY_virSecretPtr;

typedef struct virSecretPtrPtr *virSecretPtrPtrPtr;
typedef struct kvirSecretPtrPtrPtr {
	KonohaObjectHeader h;
	virSecretPtrPtrPtr p;
}kvirSecretPtrPtrPtr;
static int TY_virSecretPtrPtrPtr;

typedef struct kvirSecurityLabelPtr {
	KonohaObjectHeader h;
	virSecurityLabelPtr p;
}kvirSecurityLabelPtr;
static int TY_virSecurityLabelPtr;

typedef struct virSecurityLabelPtr *virSecurityLabelPtrPtr;
typedef struct kvirSecurityLabelPtrPtr {
	KonohaObjectHeader h;
	virSecurityLabelPtrPtr p;
}kvirSecurityLabelPtrPtr;
static int TY_virSecurityLabelPtrPtr;

typedef struct kvirSecurityModelPtr {
	KonohaObjectHeader h;
	virSecurityModelPtr p;
}kvirSecurityModelPtr;
static int TY_virSecurityModelPtr;

typedef struct kvirStoragePoolInfoPtr {
	KonohaObjectHeader h;
	virStoragePoolInfoPtr p;
}kvirStoragePoolInfoPtr;
static int TY_virStoragePoolInfoPtr;

typedef struct kvirStoragePoolPtr {
	KonohaObjectHeader h;
	virStoragePoolPtr p;
}kvirStoragePoolPtr;
static int TY_virStoragePoolPtr;

typedef struct virStoragePoolPtrPtr *virStoragePoolPtrPtrPtr;
typedef struct kvirStoragePoolPtrPtrPtr {
	KonohaObjectHeader h;
	virStoragePoolPtrPtrPtr p;
}kvirStoragePoolPtrPtrPtr;
static int TY_virStoragePoolPtrPtrPtr;

typedef struct kvirStorageVolInfoPtr {
	KonohaObjectHeader h;
	virStorageVolInfoPtr p;
}kvirStorageVolInfoPtr;
static int TY_virStorageVolInfoPtr;

typedef struct kvirStorageVolPtr {
	KonohaObjectHeader h;
	virStorageVolPtr p;
}kvirStorageVolPtr;
static int TY_virStorageVolPtr;

typedef struct virStorageVolPtrPtr *virStorageVolPtrPtrPtr;
typedef struct kvirStorageVolPtrPtrPtr {
	KonohaObjectHeader h;
	virStorageVolPtrPtrPtr p;
}kvirStorageVolPtrPtrPtr;
static int TY_virStorageVolPtrPtrPtr;

typedef struct kvirStreamEventCallback {
	KonohaObjectHeader h;
	virStreamEventCallback p;
}kvirStreamEventCallback;
static int TY_virStreamEventCallback;

typedef struct kvirStreamPtr {
	KonohaObjectHeader h;
	virStreamPtr p;
}kvirStreamPtr;
static int TY_virStreamPtr;

typedef struct kvirStreamSinkFunc {
	KonohaObjectHeader h;
	virStreamSinkFunc p;
}kvirStreamSinkFunc;
static int TY_virStreamSinkFunc;

typedef struct kvirStreamSourceFunc {
	KonohaObjectHeader h;
	virStreamSourceFunc p;
}kvirStreamSourceFunc;
static int TY_virStreamSourceFunc;

typedef struct kvirTypedParameterPtr {
	KonohaObjectHeader h;
	virTypedParameterPtr p;
}kvirTypedParameterPtr;
static int TY_virTypedParameterPtr;

typedef struct kvirVcpuInfoPtr {
	KonohaObjectHeader h;
	virVcpuInfoPtr p;
}kvirVcpuInfoPtr;
static int TY_virVcpuInfoPtr;


//typedef struct kvirDomainPtr {
//	KonohaObjectHeader h;
//	virDomainPtr p;
//}kvirDomainPtr;
//static int TY_virDomainPtr;
//
//typedef struct kvirTypedParameterPtr {
//	KonohaObjectHeader h;
//	virTypedParameterPtr p;
//}kvirTypedParameterPtr;
//static int TY_virTypedParameterPtr;
//
//typedef struct kvirConnectPtr {
//	KonohaObjectHeader h;
//	virConnectPtr p;
//}kvirConnectPtr;
//static int TY_virConnectPtr;
//
//typedef struct kvirConnectAuthPtr {
//	KonohaObjectHeader h;
//	virConnectAuthPtr p;
//}kvirConnectAuthPtr;
//static int TY_virConnectAuthPtr;

typedef kStringVar kCharPtr;
static int TY_charPtr = TY_String;

typedef struct kIntPtr {
	KonohaObjectHeader h;
	intPtr p;
}kIntPtr;
static int TY_intPtr;

typedef struct kUnsigned_longPtr {
	KonohaObjectHeader h;
	unsigned_long_Ptr p;
}kUnsigned_longPtr;
static int TY_unsigned_long_Ptr;

typedef kUnsigned_longPtr kUnsigned_long_longPtr;

static void virt_loadStructData(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_CLASS defvirConnectAuthPtr = {
		.structname = "virConnectAuthPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirConnectAuthPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirConnectAuthPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirConnectAuthPtr, trace);
	TY_virConnectAuthPtr = ctvirConnectAuthPtr->typeId;

	KDEFINE_CLASS defvirConnectCloseFunc = {
		.structname = "virConnectCloseFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirConnectCloseFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirConnectCloseFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirConnectCloseFunc, trace);
	TY_virConnectCloseFunc = ctvirConnectCloseFunc->typeId;

	KDEFINE_CLASS defvirConnectDomainEventCallback = {
		.structname = "virConnectDomainEventCallback",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirConnectDomainEventCallback),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirConnectDomainEventCallback = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirConnectDomainEventCallback, trace);
	TY_virConnectDomainEventCallback = ctvirConnectDomainEventCallback->typeId;

	KDEFINE_CLASS defvirConnectDomainEventGenericCallback = {
		.structname = "virConnectDomainEventGenericCallback",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirConnectDomainEventGenericCallback),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirConnectDomainEventGenericCallback = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirConnectDomainEventGenericCallback, trace);
	TY_virConnectDomainEventGenericCallback = ctvirConnectDomainEventGenericCallback->typeId;

	KDEFINE_CLASS defvirConnectPtr = {
		.structname = "virConnectPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirConnectPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirConnectPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirConnectPtr, trace);
	TY_virConnectPtr = ctvirConnectPtr->typeId;

	KDEFINE_CLASS defvirDomainBlockInfoPtr = {
		.structname = "virDomainBlockInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainBlockInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainBlockInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainBlockInfoPtr, trace);
	TY_virDomainBlockInfoPtr = ctvirDomainBlockInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainBlockJobInfoPtr = {
		.structname = "virDomainBlockJobInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainBlockJobInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainBlockJobInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainBlockJobInfoPtr, trace);
	TY_virDomainBlockJobInfoPtr = ctvirDomainBlockJobInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainBlockStatsPtr = {
		.structname = "virDomainBlockStatsPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainBlockStatsPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainBlockStatsPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainBlockStatsPtr, trace);
	TY_virDomainBlockStatsPtr = ctvirDomainBlockStatsPtr->typeId;

	KDEFINE_CLASS defvirDomainControlInfoPtr = {
		.structname = "virDomainControlInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainControlInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainControlInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainControlInfoPtr, trace);
	TY_virDomainControlInfoPtr = ctvirDomainControlInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainDiskErrorPtr = {
		.structname = "virDomainDiskErrorPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainDiskErrorPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainDiskErrorPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainDiskErrorPtr, trace);
	TY_virDomainDiskErrorPtr = ctvirDomainDiskErrorPtr->typeId;

	KDEFINE_CLASS defvirDomainInfoPtr = {
		.structname = "virDomainInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainInfoPtr, trace);
	TY_virDomainInfoPtr = ctvirDomainInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainInterfaceStatsPtr = {
		.structname = "virDomainInterfaceStatsPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainInterfaceStatsPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainInterfaceStatsPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainInterfaceStatsPtr, trace);
	TY_virDomainInterfaceStatsPtr = ctvirDomainInterfaceStatsPtr->typeId;

	KDEFINE_CLASS defvirDomainJobInfoPtr = {
		.structname = "virDomainJobInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainJobInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainJobInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainJobInfoPtr, trace);
	TY_virDomainJobInfoPtr = ctvirDomainJobInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainMemoryStatPtr = {
		.structname = "virDomainMemoryStatPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainMemoryStatPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainMemoryStatPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainMemoryStatPtr, trace);
	TY_virDomainMemoryStatPtr = ctvirDomainMemoryStatPtr->typeId;

	KDEFINE_CLASS defvirDomainPtr = {
		.structname = "virDomainPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainPtr, trace);
	TY_virDomainPtr = ctvirDomainPtr->typeId;

	KDEFINE_CLASS defvirDomainPtrPtrPtr = {
		.structname = "virDomainPtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainPtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainPtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainPtrPtrPtr, trace);
	TY_virDomainPtrPtrPtr = ctvirDomainPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirDomainSnapshotPtr = {
		.structname = "virDomainSnapshotPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainSnapshotPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainSnapshotPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainSnapshotPtr, trace);
	TY_virDomainSnapshotPtr = ctvirDomainSnapshotPtr->typeId;

	KDEFINE_CLASS defvirDomainSnapshotPtrPtrPtr = {
		.structname = "virDomainSnapshotPtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirDomainSnapshotPtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirDomainSnapshotPtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirDomainSnapshotPtrPtrPtr, trace);
	TY_virDomainSnapshotPtrPtrPtr = ctvirDomainSnapshotPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirEventAddHandleFunc = {
		.structname = "virEventAddHandleFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirEventAddHandleFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirEventAddHandleFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirEventAddHandleFunc, trace);
	TY_virEventAddHandleFunc = ctvirEventAddHandleFunc->typeId;

	KDEFINE_CLASS defvirEventAddTimeoutFunc = {
		.structname = "virEventAddTimeoutFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirEventAddTimeoutFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirEventAddTimeoutFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirEventAddTimeoutFunc, trace);
	TY_virEventAddTimeoutFunc = ctvirEventAddTimeoutFunc->typeId;

	KDEFINE_CLASS defvirEventHandleCallback = {
		.structname = "virEventHandleCallback",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirEventHandleCallback),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirEventHandleCallback = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirEventHandleCallback, trace);
	TY_virEventHandleCallback = ctvirEventHandleCallback->typeId;

	KDEFINE_CLASS defvirEventRemoveHandleFunc = {
		.structname = "virEventRemoveHandleFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirEventRemoveHandleFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirEventRemoveHandleFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirEventRemoveHandleFunc, trace);
	TY_virEventRemoveHandleFunc = ctvirEventRemoveHandleFunc->typeId;

	KDEFINE_CLASS defvirEventRemoveTimeoutFunc = {
		.structname = "virEventRemoveTimeoutFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirEventRemoveTimeoutFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirEventRemoveTimeoutFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirEventRemoveTimeoutFunc, trace);
	TY_virEventRemoveTimeoutFunc = ctvirEventRemoveTimeoutFunc->typeId;

	KDEFINE_CLASS defvirEventTimeoutCallback = {
		.structname = "virEventTimeoutCallback",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirEventTimeoutCallback),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirEventTimeoutCallback = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirEventTimeoutCallback, trace);
	TY_virEventTimeoutCallback = ctvirEventTimeoutCallback->typeId;

	KDEFINE_CLASS defvirEventUpdateHandleFunc = {
		.structname = "virEventUpdateHandleFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirEventUpdateHandleFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirEventUpdateHandleFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirEventUpdateHandleFunc, trace);
	TY_virEventUpdateHandleFunc = ctvirEventUpdateHandleFunc->typeId;

	KDEFINE_CLASS defvirEventUpdateTimeoutFunc = {
		.structname = "virEventUpdateTimeoutFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirEventUpdateTimeoutFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirEventUpdateTimeoutFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirEventUpdateTimeoutFunc, trace);
	TY_virEventUpdateTimeoutFunc = ctvirEventUpdateTimeoutFunc->typeId;

	KDEFINE_CLASS defvirFreeCallback = {
		.structname = "virFreeCallback",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirFreeCallback),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirFreeCallback = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirFreeCallback, trace);
	TY_virFreeCallback = ctvirFreeCallback->typeId;

	KDEFINE_CLASS defvirInterfacePtr = {
		.structname = "virInterfacePtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirInterfacePtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirInterfacePtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirInterfacePtr, trace);
	TY_virInterfacePtr = ctvirInterfacePtr->typeId;

	KDEFINE_CLASS defvirInterfacePtrPtrPtr = {
		.structname = "virInterfacePtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirInterfacePtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirInterfacePtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirInterfacePtrPtrPtr, trace);
	TY_virInterfacePtrPtrPtr = ctvirInterfacePtrPtrPtr->typeId;

	KDEFINE_CLASS defvirNWFilterPtr = {
		.structname = "virNWFilterPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNWFilterPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNWFilterPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNWFilterPtr, trace);
	TY_virNWFilterPtr = ctvirNWFilterPtr->typeId;

	KDEFINE_CLASS defvirNWFilterPtrPtrPtr = {
		.structname = "virNWFilterPtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNWFilterPtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNWFilterPtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNWFilterPtrPtrPtr, trace);
	TY_virNWFilterPtrPtrPtr = ctvirNWFilterPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirNetworkPtr = {
		.structname = "virNetworkPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNetworkPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNetworkPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNetworkPtr, trace);
	TY_virNetworkPtr = ctvirNetworkPtr->typeId;

	KDEFINE_CLASS defvirNetworkPtrPtrPtr = {
		.structname = "virNetworkPtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNetworkPtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNetworkPtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNetworkPtrPtrPtr, trace);
	TY_virNetworkPtrPtrPtr = ctvirNetworkPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirNodeCPUStatsPtr = {
		.structname = "virNodeCPUStatsPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNodeCPUStatsPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNodeCPUStatsPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNodeCPUStatsPtr, trace);
	TY_virNodeCPUStatsPtr = ctvirNodeCPUStatsPtr->typeId;

	KDEFINE_CLASS defvirNodeDevicePtr = {
		.structname = "virNodeDevicePtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNodeDevicePtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNodeDevicePtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNodeDevicePtr, trace);
	TY_virNodeDevicePtr = ctvirNodeDevicePtr->typeId;

	KDEFINE_CLASS defvirNodeDevicePtrPtrPtr = {
		.structname = "virNodeDevicePtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNodeDevicePtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNodeDevicePtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNodeDevicePtrPtrPtr, trace);
	TY_virNodeDevicePtrPtrPtr = ctvirNodeDevicePtrPtrPtr->typeId;

	KDEFINE_CLASS defvirNodeInfoPtr = {
		.structname = "virNodeInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNodeInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNodeInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNodeInfoPtr, trace);
	TY_virNodeInfoPtr = ctvirNodeInfoPtr->typeId;

	KDEFINE_CLASS defvirNodeMemoryStatsPtr = {
		.structname = "virNodeMemoryStatsPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirNodeMemoryStatsPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirNodeMemoryStatsPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirNodeMemoryStatsPtr, trace);
	TY_virNodeMemoryStatsPtr = ctvirNodeMemoryStatsPtr->typeId;

	KDEFINE_CLASS defvirSecretPtr = {
		.structname = "virSecretPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirSecretPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirSecretPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirSecretPtr, trace);
	TY_virSecretPtr = ctvirSecretPtr->typeId;

	KDEFINE_CLASS defvirSecretPtrPtrPtr = {
		.structname = "virSecretPtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirSecretPtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirSecretPtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirSecretPtrPtrPtr, trace);
	TY_virSecretPtrPtrPtr = ctvirSecretPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirSecurityLabelPtr = {
		.structname = "virSecurityLabelPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirSecurityLabelPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirSecurityLabelPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirSecurityLabelPtr, trace);
	TY_virSecurityLabelPtr = ctvirSecurityLabelPtr->typeId;

	KDEFINE_CLASS defvirSecurityLabelPtrPtr = {
		.structname = "virSecurityLabelPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirSecurityLabelPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirSecurityLabelPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirSecurityLabelPtrPtr, trace);
	TY_virSecurityLabelPtrPtr = ctvirSecurityLabelPtrPtr->typeId;

	KDEFINE_CLASS defvirSecurityModelPtr = {
		.structname = "virSecurityModelPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirSecurityModelPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirSecurityModelPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirSecurityModelPtr, trace);
	TY_virSecurityModelPtr = ctvirSecurityModelPtr->typeId;

	KDEFINE_CLASS defvirStoragePoolInfoPtr = {
		.structname = "virStoragePoolInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStoragePoolInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStoragePoolInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStoragePoolInfoPtr, trace);
	TY_virStoragePoolInfoPtr = ctvirStoragePoolInfoPtr->typeId;

	KDEFINE_CLASS defvirStoragePoolPtr = {
		.structname = "virStoragePoolPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStoragePoolPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStoragePoolPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStoragePoolPtr, trace);
	TY_virStoragePoolPtr = ctvirStoragePoolPtr->typeId;

	KDEFINE_CLASS defvirStoragePoolPtrPtrPtr = {
		.structname = "virStoragePoolPtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStoragePoolPtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStoragePoolPtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStoragePoolPtrPtrPtr, trace);
	TY_virStoragePoolPtrPtrPtr = ctvirStoragePoolPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirStorageVolInfoPtr = {
		.structname = "virStorageVolInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStorageVolInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStorageVolInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStorageVolInfoPtr, trace);
	TY_virStorageVolInfoPtr = ctvirStorageVolInfoPtr->typeId;

	KDEFINE_CLASS defvirStorageVolPtr = {
		.structname = "virStorageVolPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStorageVolPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStorageVolPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStorageVolPtr, trace);
	TY_virStorageVolPtr = ctvirStorageVolPtr->typeId;

	KDEFINE_CLASS defvirStorageVolPtrPtrPtr = {
		.structname = "virStorageVolPtrPtrPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStorageVolPtrPtrPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStorageVolPtrPtrPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStorageVolPtrPtrPtr, trace);
	TY_virStorageVolPtrPtrPtr = ctvirStorageVolPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirStreamEventCallback = {
		.structname = "virStreamEventCallback",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStreamEventCallback),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStreamEventCallback = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStreamEventCallback, trace);
	TY_virStreamEventCallback = ctvirStreamEventCallback->typeId;

	KDEFINE_CLASS defvirStreamPtr = {
		.structname = "virStreamPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStreamPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStreamPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStreamPtr, trace);
	TY_virStreamPtr = ctvirStreamPtr->typeId;

	KDEFINE_CLASS defvirStreamSinkFunc = {
		.structname = "virStreamSinkFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStreamSinkFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStreamSinkFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStreamSinkFunc, trace);
	TY_virStreamSinkFunc = ctvirStreamSinkFunc->typeId;

	KDEFINE_CLASS defvirStreamSourceFunc = {
		.structname = "virStreamSourceFunc",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirStreamSourceFunc),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirStreamSourceFunc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirStreamSourceFunc, trace);
	TY_virStreamSourceFunc = ctvirStreamSourceFunc->typeId;

	KDEFINE_CLASS defvirTypedParameterPtr = {
		.structname = "virTypedParameterPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirTypedParameterPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirTypedParameterPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirTypedParameterPtr, trace);
	TY_virTypedParameterPtr = ctvirTypedParameterPtr->typeId;

	KDEFINE_CLASS defvirVcpuInfoPtr = {
		.structname = "virVcpuInfoPtr",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kvirVcpuInfoPtr),
		.cflag = kClass_Final,
		.init = virt_common_ptr_init,
		.free = virt_common_ptr_free,
	};
	KonohaClass *ctvirVcpuInfoPtr = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defvirVcpuInfoPtr, trace);
	TY_virVcpuInfoPtr = ctvirVcpuInfoPtr->typeId;

}
#endif /* end of include guard */
