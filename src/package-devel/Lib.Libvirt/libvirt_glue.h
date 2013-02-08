#ifndef LIBVIRT_GLUE_H
#define LIBVIRT_GLUE_H

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/import/methoddecl.h>
#include "libvirt/libvirt.h"
#include "libvirt/libvirt.h"

#include "type_cast.h"

/* TODO 
 * Declare fundamental data types as konoha object, such as void*
 * */
#define KType_voidPtr 0
#define KType_unsigned_charPtr 0
#define KType_size_t 0
#define KType_charPtrPtr 0
#define KType_unsigned_long_long_Ptr 0
#define KType_unsigned_intPtr 0
#define KType_size_tPtr 0

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
#define KType_unsigned_long         KType_Int
#define KType_unsigned_long_long    KType_Int
#define KType_unsigned_int          KType_Int

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
//#define TYPE_Array(TYPE)      (KClass_p0(kctx, KClass_Array, KType_##TYPE))->typeId
//#define KType_char            KType_Int
//#define KType_Locale          KType_String
#define RETURNint(RET)     KReturnInt(RET)
#define RETURNvirDomainPtr(RET) KReturn(KLIB new_kObject(kctx, OnStack, KClass_(KType_virDomainPtr), (uintptr_t)RET))
#define RETURNvirConnectPtr(RET) KReturn(KLIB new_kObject(kctx, OnStack, KClass_(KType_virConnectPtr), (uintptr_t)RET))
#define RETURNcharPtr(RET) KReturn(KLIB new_kObject(kctx, OnStack, KClass_(KType_charPtr), (uintptr_t)RET))

#define LIBVIRT_PACKAGE_NAME "Libvirt"

#define PACKAGE_INIT(NAME)      NAME##_Init
#define PACKAGE_INIT_PKG(NAME)  NAME##_PackupNameSpace
#define PACKAGE_SETUP_PKG(NAME) NAME##_ExportNameSpace
#define PACKAGE_TOSTRING(NAME) "" # NAME

typedef struct kvirCommonPtr {
	kObjectHeader h;
	void *p;
}kvirCommonPtr;

static void virt_common_ptr_Init(KonohaContext *kctx, kObject *o, void *conf) {
	kvirCommonPtr *ptr = (kvirCommonPtr *)o;
	ptr->p = conf;
};

static void virt_common_ptr_Free(KonohaContext *kctx, kObject *o) {
	kvirCommonPtr *ptr = (kvirCommonPtr *)o;
	free(ptr->p);
};

typedef struct kvirConnectAuthPtr {
	kObjectHeader h;
	virConnectAuthPtr p;
}kvirConnectAuthPtr;
static int KType_virConnectAuthPtr;

typedef struct kvirConnectCloseFunc {
	kObjectHeader h;
	virConnectCloseFunc p;
}kvirConnectCloseFunc;
static int KType_virConnectCloseFunc;

typedef struct kvirConnectDomainEventCallback {
	kObjectHeader h;
	virConnectDomainEventCallback p;
}kvirConnectDomainEventCallback;
static int KType_virConnectDomainEventCallback;

typedef struct kvirConnectDomainEventGenericCallback {
	kObjectHeader h;
	virConnectDomainEventGenericCallback p;
}kvirConnectDomainEventGenericCallback;
static int KType_virConnectDomainEventGenericCallback;

typedef struct kvirConnectPtr {
	kObjectHeader h;
	virConnectPtr p;
}kvirConnectPtr;
static int KType_virConnectPtr;

typedef struct kvirDomainNodeInfoPtr {
	kObjectHeader h;
	virDomainNodeInfoPtr p;
}kvirDomainNodeInfoPtr;
static int KType_virDomainNodeInfoPtr;

typedef struct kvirDomainNodeJobInfoPtr {
	kObjectHeader h;
	virDomainNodeJobInfoPtr p;
}kvirDomainNodeJobInfoPtr;
static int KType_virDomainNodeJobInfoPtr;

typedef struct kvirDomainNodeStatsPtr {
	kObjectHeader h;
	virDomainNodeStatsPtr p;
}kvirDomainNodeStatsPtr;
static int KType_virDomainNodeStatsPtr;

typedef struct kvirDomainControlInfoPtr {
	kObjectHeader h;
	virDomainControlInfoPtr p;
}kvirDomainControlInfoPtr;
static int KType_virDomainControlInfoPtr;

typedef struct kvirDomainDiskErrorPtr {
	kObjectHeader h;
	virDomainDiskErrorPtr p;
}kvirDomainDiskErrorPtr;
static int KType_virDomainDiskErrorPtr;

typedef struct kvirDomainInfoPtr {
	kObjectHeader h;
	virDomainInfoPtr p;
}kvirDomainInfoPtr;
static int KType_virDomainInfoPtr;

typedef struct kvirDomainInterfaceStatsPtr {
	kObjectHeader h;
	virDomainInterfaceStatsPtr p;
}kvirDomainInterfaceStatsPtr;
static int KType_virDomainInterfaceStatsPtr;

typedef struct kvirDomainJobInfoPtr {
	kObjectHeader h;
	virDomainJobInfoPtr p;
}kvirDomainJobInfoPtr;
static int KType_virDomainJobInfoPtr;

typedef struct kvirDomainMemoryStatPtr {
	kObjectHeader h;
	virDomainMemoryStatPtr p;
}kvirDomainMemoryStatPtr;
static int KType_virDomainMemoryStatPtr;

typedef struct kvirDomainPtr {
	kObjectHeader h;
	virDomainPtr p;
}kvirDomainPtr;
static int KType_virDomainPtr;

typedef struct virDomainPtrPtr *virDomainPtrPtrPtr;
typedef struct kvirDomainPtrPtrPtr {
	kObjectHeader h;
	virDomainPtrPtrPtr p;
}kvirDomainPtrPtrPtr;
static int KType_virDomainPtrPtrPtr;

typedef struct kvirDomainSnapshotPtr {
	kObjectHeader h;
	virDomainSnapshotPtr p;
}kvirDomainSnapshotPtr;
static int KType_virDomainSnapshotPtr;

typedef struct virDomainSnapshotPtrPtr *virDomainSnapshotPtrPtrPtr;
typedef struct kvirDomainSnapshotPtrPtrPtr {
	kObjectHeader h;
	virDomainSnapshotPtrPtrPtr p;
}kvirDomainSnapshotPtrPtrPtr;
static int KType_virDomainSnapshotPtrPtrPtr;

typedef struct kvirEventAddHandleFunc {
	kObjectHeader h;
	virEventAddHandleFunc p;
}kvirEventAddHandleFunc;
static int KType_virEventAddHandleFunc;

typedef struct kvirEventAddTimeoutFunc {
	kObjectHeader h;
	virEventAddTimeoutFunc p;
}kvirEventAddTimeoutFunc;
static int KType_virEventAddTimeoutFunc;

typedef struct kvirEventHandleCallback {
	kObjectHeader h;
	virEventHandleCallback p;
}kvirEventHandleCallback;
static int KType_virEventHandleCallback;

typedef struct kvirEventRemoveHandleFunc {
	kObjectHeader h;
	virEventRemoveHandleFunc p;
}kvirEventRemoveHandleFunc;
static int KType_virEventRemoveHandleFunc;

typedef struct kvirEventRemoveTimeoutFunc {
	kObjectHeader h;
	virEventRemoveTimeoutFunc p;
}kvirEventRemoveTimeoutFunc;
static int KType_virEventRemoveTimeoutFunc;

typedef struct kvirEventTimeoutCallback {
	kObjectHeader h;
	virEventTimeoutCallback p;
}kvirEventTimeoutCallback;
static int KType_virEventTimeoutCallback;

typedef struct kvirEventUpdateHandleFunc {
	kObjectHeader h;
	virEventUpdateHandleFunc p;
}kvirEventUpdateHandleFunc;
static int KType_virEventUpdateHandleFunc;

typedef struct kvirEventUpdateTimeoutFunc {
	kObjectHeader h;
	virEventUpdateTimeoutFunc p;
}kvirEventUpdateTimeoutFunc;
static int KType_virEventUpdateTimeoutFunc;

typedef struct kvirFreeCallback {
	kObjectHeader h;
	virFreeCallback p;
}kvirFreeCallback;
static int KType_virFreeCallback;

typedef struct kvirInterfacePtr {
	kObjectHeader h;
	virInterfacePtr p;
}kvirInterfacePtr;
static int KType_virInterfacePtr;

typedef struct virInterfacePtrPtr *virInterfacePtrPtrPtr;
typedef struct kvirInterfacePtrPtrPtr {
	kObjectHeader h;
	virInterfacePtrPtrPtr p;
}kvirInterfacePtrPtrPtr;
static int KType_virInterfacePtrPtrPtr;

typedef struct kvirNWFilterPtr {
	kObjectHeader h;
	virNWFilterPtr p;
}kvirNWFilterPtr;
static int KType_virNWFilterPtr;

typedef struct virNWFilterPtrPtr *virNWFilterPtrPtrPtr;
typedef struct kvirNWFilterPtrPtrPtr {
	kObjectHeader h;
	virNWFilterPtrPtrPtr p;
}kvirNWFilterPtrPtrPtr;
static int KType_virNWFilterPtrPtrPtr;

typedef struct kvirNetworkPtr {
	kObjectHeader h;
	virNetworkPtr p;
}kvirNetworkPtr;
static int KType_virNetworkPtr;

typedef struct virNetworkPtrPtr *virNetworkPtrPtrPtr;
typedef struct kvirNetworkPtrPtrPtr {
	kObjectHeader h;
	virNetworkPtrPtrPtr p;
}kvirNetworkPtrPtrPtr;
static int KType_virNetworkPtrPtrPtr;

typedef struct kvirNodeCPUStatsPtr {
	kObjectHeader h;
	virNodeCPUStatsPtr p;
}kvirNodeCPUStatsPtr;
static int KType_virNodeCPUStatsPtr;

typedef struct kvirNodeDevicePtr {
	kObjectHeader h;
	virNodeDevicePtr p;
}kvirNodeDevicePtr;
static int KType_virNodeDevicePtr;

typedef struct virNodeDevicePtrPtr *virNodeDevicePtrPtrPtr;
typedef struct kvirNodeDevicePtrPtrPtr {
	kObjectHeader h;
	virNodeDevicePtrPtrPtr p;
}kvirNodeDevicePtrPtrPtr;
static int KType_virNodeDevicePtrPtrPtr;

typedef struct kvirNodeInfoPtr {
	kObjectHeader h;
	virNodeInfoPtr p;
}kvirNodeInfoPtr;
static int KType_virNodeInfoPtr;

typedef struct kvirNodeMemoryStatsPtr {
	kObjectHeader h;
	virNodeMemoryStatsPtr p;
}kvirNodeMemoryStatsPtr;
static int KType_virNodeMemoryStatsPtr;

typedef struct kvirSecretPtr {
	kObjectHeader h;
	virSecretPtr p;
}kvirSecretPtr;
static int KType_virSecretPtr;

typedef struct virSecretPtrPtr *virSecretPtrPtrPtr;
typedef struct kvirSecretPtrPtrPtr {
	kObjectHeader h;
	virSecretPtrPtrPtr p;
}kvirSecretPtrPtrPtr;
static int KType_virSecretPtrPtrPtr;

typedef struct kvirSecurityLabelPtr {
	kObjectHeader h;
	virSecurityLabelPtr p;
}kvirSecurityLabelPtr;
static int KType_virSecurityLabelPtr;

typedef struct virSecurityLabelPtr *virSecurityLabelPtrPtr;
typedef struct kvirSecurityLabelPtrPtr {
	kObjectHeader h;
	virSecurityLabelPtrPtr p;
}kvirSecurityLabelPtrPtr;
static int KType_virSecurityLabelPtrPtr;

typedef struct kvirSecurityModelPtr {
	kObjectHeader h;
	virSecurityModelPtr p;
}kvirSecurityModelPtr;
static int KType_virSecurityModelPtr;

typedef struct kvirStoragePoolInfoPtr {
	kObjectHeader h;
	virStoragePoolInfoPtr p;
}kvirStoragePoolInfoPtr;
static int KType_virStoragePoolInfoPtr;

typedef struct kvirStoragePoolPtr {
	kObjectHeader h;
	virStoragePoolPtr p;
}kvirStoragePoolPtr;
static int KType_virStoragePoolPtr;

typedef struct virStoragePoolPtrPtr *virStoragePoolPtrPtrPtr;
typedef struct kvirStoragePoolPtrPtrPtr {
	kObjectHeader h;
	virStoragePoolPtrPtrPtr p;
}kvirStoragePoolPtrPtrPtr;
static int KType_virStoragePoolPtrPtrPtr;

typedef struct kvirStorageVolInfoPtr {
	kObjectHeader h;
	virStorageVolInfoPtr p;
}kvirStorageVolInfoPtr;
static int KType_virStorageVolInfoPtr;

typedef struct kvirStorageVolPtr {
	kObjectHeader h;
	virStorageVolPtr p;
}kvirStorageVolPtr;
static int KType_virStorageVolPtr;

typedef struct virStorageVolPtrPtr *virStorageVolPtrPtrPtr;
typedef struct kvirStorageVolPtrPtrPtr {
	kObjectHeader h;
	virStorageVolPtrPtrPtr p;
}kvirStorageVolPtrPtrPtr;
static int KType_virStorageVolPtrPtrPtr;

typedef struct kvirStreamEventCallback {
	kObjectHeader h;
	virStreamEventCallback p;
}kvirStreamEventCallback;
static int KType_virStreamEventCallback;

typedef struct kvirStreamPtr {
	kObjectHeader h;
	virStreamPtr p;
}kvirStreamPtr;
static int KType_virStreamPtr;

typedef struct kvirStreamSinkFunc {
	kObjectHeader h;
	virStreamSinkFunc p;
}kvirStreamSinkFunc;
static int KType_virStreamSinkFunc;

typedef struct kvirStreamSourceFunc {
	kObjectHeader h;
	virStreamSourceFunc p;
}kvirStreamSourceFunc;
static int KType_virStreamSourceFunc;

typedef struct kvirTypedParameterPtr {
	kObjectHeader h;
	virTypedParameterPtr p;
}kvirTypedParameterPtr;
static int KType_virTypedParameterPtr;

typedef struct kvirVcpuInfoPtr {
	kObjectHeader h;
	virVcpuInfoPtr p;
}kvirVcpuInfoPtr;
static int KType_virVcpuInfoPtr;


//typedef struct kvirDomainPtr {
//	kObjectHeader h;
//	virDomainPtr p;
//}kvirDomainPtr;
//static int KType_virDomainPtr;
//
//typedef struct kvirTypedParameterPtr {
//	kObjectHeader h;
//	virTypedParameterPtr p;
//}kvirTypedParameterPtr;
//static int KType_virTypedParameterPtr;
//
//typedef struct kvirConnectPtr {
//	kObjectHeader h;
//	virConnectPtr p;
//}kvirConnectPtr;
//static int KType_virConnectPtr;
//
//typedef struct kvirConnectAuthPtr {
//	kObjectHeader h;
//	virConnectAuthPtr p;
//}kvirConnectAuthPtr;
//static int KType_virConnectAuthPtr;

typedef kStringVar kCharPtr;
static int KType_charPtr = KType_String;

typedef struct kIntPtr {
	kObjectHeader h;
	intPtr p;
}kIntPtr;
static int KType_IntPtr;

typedef struct kUnsigned_longPtr {
	kObjectHeader h;
	unsigned_long_Ptr p;
}kUnsigned_longPtr;
static int KType_unsigned_long_Ptr;

typedef kUnsigned_longPtr kUnsigned_long_longPtr;

static void virt_loadStructData(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_CLASS defvirConnectAuthPtr = {
		.structname = "virConnectAuthPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirConnectAuthPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirConnectAuthPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirConnectAuthPtr, trace);
	KType_virConnectAuthPtr = ctvirConnectAuthPtr->typeId;

	KDEFINE_CLASS defvirConnectCloseFunc = {
		.structname = "virConnectCloseFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirConnectCloseFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirConnectCloseFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirConnectCloseFunc, trace);
	KType_virConnectCloseFunc = ctvirConnectCloseFunc->typeId;

	KDEFINE_CLASS defvirConnectDomainEventCallback = {
		.structname = "virConnectDomainEventCallback",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirConnectDomainEventCallback),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirConnectDomainEventCallback = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirConnectDomainEventCallback, trace);
	KType_virConnectDomainEventCallback = ctvirConnectDomainEventCallback->typeId;

	KDEFINE_CLASS defvirConnectDomainEventGenericCallback = {
		.structname = "virConnectDomainEventGenericCallback",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirConnectDomainEventGenericCallback),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirConnectDomainEventGenericCallback = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirConnectDomainEventGenericCallback, trace);
	KType_virConnectDomainEventGenericCallback = ctvirConnectDomainEventGenericCallback->typeId;

	KDEFINE_CLASS defvirConnectPtr = {
		.structname = "virConnectPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirConnectPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirConnectPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirConnectPtr, trace);
	KType_virConnectPtr = ctvirConnectPtr->typeId;

	KDEFINE_CLASS defvirDomainNodeInfoPtr = {
		.structname = "virDomainNodeInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainNodeInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainNodeInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainNodeInfoPtr, trace);
	KType_virDomainNodeInfoPtr = ctvirDomainNodeInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainNodeJobInfoPtr = {
		.structname = "virDomainNodeJobInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainNodeJobInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainNodeJobInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainNodeJobInfoPtr, trace);
	KType_virDomainNodeJobInfoPtr = ctvirDomainNodeJobInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainNodeStatsPtr = {
		.structname = "virDomainNodeStatsPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainNodeStatsPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainNodeStatsPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainNodeStatsPtr, trace);
	KType_virDomainNodeStatsPtr = ctvirDomainNodeStatsPtr->typeId;

	KDEFINE_CLASS defvirDomainControlInfoPtr = {
		.structname = "virDomainControlInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainControlInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainControlInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainControlInfoPtr, trace);
	KType_virDomainControlInfoPtr = ctvirDomainControlInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainDiskErrorPtr = {
		.structname = "virDomainDiskErrorPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainDiskErrorPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainDiskErrorPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainDiskErrorPtr, trace);
	KType_virDomainDiskErrorPtr = ctvirDomainDiskErrorPtr->typeId;

	KDEFINE_CLASS defvirDomainInfoPtr = {
		.structname = "virDomainInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainInfoPtr, trace);
	KType_virDomainInfoPtr = ctvirDomainInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainInterfaceStatsPtr = {
		.structname = "virDomainInterfaceStatsPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainInterfaceStatsPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainInterfaceStatsPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainInterfaceStatsPtr, trace);
	KType_virDomainInterfaceStatsPtr = ctvirDomainInterfaceStatsPtr->typeId;

	KDEFINE_CLASS defvirDomainJobInfoPtr = {
		.structname = "virDomainJobInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainJobInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainJobInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainJobInfoPtr, trace);
	KType_virDomainJobInfoPtr = ctvirDomainJobInfoPtr->typeId;

	KDEFINE_CLASS defvirDomainMemoryStatPtr = {
		.structname = "virDomainMemoryStatPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainMemoryStatPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainMemoryStatPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainMemoryStatPtr, trace);
	KType_virDomainMemoryStatPtr = ctvirDomainMemoryStatPtr->typeId;

	KDEFINE_CLASS defvirDomainPtr = {
		.structname = "virDomainPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainPtr, trace);
	KType_virDomainPtr = ctvirDomainPtr->typeId;

	KDEFINE_CLASS defvirDomainPtrPtrPtr = {
		.structname = "virDomainPtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainPtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainPtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainPtrPtrPtr, trace);
	KType_virDomainPtrPtrPtr = ctvirDomainPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirDomainSnapshotPtr = {
		.structname = "virDomainSnapshotPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainSnapshotPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainSnapshotPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainSnapshotPtr, trace);
	KType_virDomainSnapshotPtr = ctvirDomainSnapshotPtr->typeId;

	KDEFINE_CLASS defvirDomainSnapshotPtrPtrPtr = {
		.structname = "virDomainSnapshotPtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirDomainSnapshotPtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirDomainSnapshotPtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirDomainSnapshotPtrPtrPtr, trace);
	KType_virDomainSnapshotPtrPtrPtr = ctvirDomainSnapshotPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirEventAddHandleFunc = {
		.structname = "virEventAddHandleFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirEventAddHandleFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirEventAddHandleFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirEventAddHandleFunc, trace);
	KType_virEventAddHandleFunc = ctvirEventAddHandleFunc->typeId;

	KDEFINE_CLASS defvirEventAddTimeoutFunc = {
		.structname = "virEventAddTimeoutFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirEventAddTimeoutFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirEventAddTimeoutFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirEventAddTimeoutFunc, trace);
	KType_virEventAddTimeoutFunc = ctvirEventAddTimeoutFunc->typeId;

	KDEFINE_CLASS defvirEventHandleCallback = {
		.structname = "virEventHandleCallback",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirEventHandleCallback),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirEventHandleCallback = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirEventHandleCallback, trace);
	KType_virEventHandleCallback = ctvirEventHandleCallback->typeId;

	KDEFINE_CLASS defvirEventRemoveHandleFunc = {
		.structname = "virEventRemoveHandleFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirEventRemoveHandleFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirEventRemoveHandleFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirEventRemoveHandleFunc, trace);
	KType_virEventRemoveHandleFunc = ctvirEventRemoveHandleFunc->typeId;

	KDEFINE_CLASS defvirEventRemoveTimeoutFunc = {
		.structname = "virEventRemoveTimeoutFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirEventRemoveTimeoutFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirEventRemoveTimeoutFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirEventRemoveTimeoutFunc, trace);
	KType_virEventRemoveTimeoutFunc = ctvirEventRemoveTimeoutFunc->typeId;

	KDEFINE_CLASS defvirEventTimeoutCallback = {
		.structname = "virEventTimeoutCallback",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirEventTimeoutCallback),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirEventTimeoutCallback = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirEventTimeoutCallback, trace);
	KType_virEventTimeoutCallback = ctvirEventTimeoutCallback->typeId;

	KDEFINE_CLASS defvirEventUpdateHandleFunc = {
		.structname = "virEventUpdateHandleFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirEventUpdateHandleFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirEventUpdateHandleFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirEventUpdateHandleFunc, trace);
	KType_virEventUpdateHandleFunc = ctvirEventUpdateHandleFunc->typeId;

	KDEFINE_CLASS defvirEventUpdateTimeoutFunc = {
		.structname = "virEventUpdateTimeoutFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirEventUpdateTimeoutFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirEventUpdateTimeoutFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirEventUpdateTimeoutFunc, trace);
	KType_virEventUpdateTimeoutFunc = ctvirEventUpdateTimeoutFunc->typeId;

	KDEFINE_CLASS defvirFreeCallback = {
		.structname = "virFreeCallback",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirFreeCallback),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirFreeCallback = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirFreeCallback, trace);
	KType_virFreeCallback = ctvirFreeCallback->typeId;

	KDEFINE_CLASS defvirInterfacePtr = {
		.structname = "virInterfacePtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirInterfacePtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirInterfacePtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirInterfacePtr, trace);
	KType_virInterfacePtr = ctvirInterfacePtr->typeId;

	KDEFINE_CLASS defvirInterfacePtrPtrPtr = {
		.structname = "virInterfacePtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirInterfacePtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirInterfacePtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirInterfacePtrPtrPtr, trace);
	KType_virInterfacePtrPtrPtr = ctvirInterfacePtrPtrPtr->typeId;

	KDEFINE_CLASS defvirNWFilterPtr = {
		.structname = "virNWFilterPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNWFilterPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNWFilterPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNWFilterPtr, trace);
	KType_virNWFilterPtr = ctvirNWFilterPtr->typeId;

	KDEFINE_CLASS defvirNWFilterPtrPtrPtr = {
		.structname = "virNWFilterPtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNWFilterPtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNWFilterPtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNWFilterPtrPtrPtr, trace);
	KType_virNWFilterPtrPtrPtr = ctvirNWFilterPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirNetworkPtr = {
		.structname = "virNetworkPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNetworkPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNetworkPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNetworkPtr, trace);
	KType_virNetworkPtr = ctvirNetworkPtr->typeId;

	KDEFINE_CLASS defvirNetworkPtrPtrPtr = {
		.structname = "virNetworkPtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNetworkPtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNetworkPtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNetworkPtrPtrPtr, trace);
	KType_virNetworkPtrPtrPtr = ctvirNetworkPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirNodeCPUStatsPtr = {
		.structname = "virNodeCPUStatsPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNodeCPUStatsPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNodeCPUStatsPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNodeCPUStatsPtr, trace);
	KType_virNodeCPUStatsPtr = ctvirNodeCPUStatsPtr->typeId;

	KDEFINE_CLASS defvirNodeDevicePtr = {
		.structname = "virNodeDevicePtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNodeDevicePtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNodeDevicePtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNodeDevicePtr, trace);
	KType_virNodeDevicePtr = ctvirNodeDevicePtr->typeId;

	KDEFINE_CLASS defvirNodeDevicePtrPtrPtr = {
		.structname = "virNodeDevicePtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNodeDevicePtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNodeDevicePtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNodeDevicePtrPtrPtr, trace);
	KType_virNodeDevicePtrPtrPtr = ctvirNodeDevicePtrPtrPtr->typeId;

	KDEFINE_CLASS defvirNodeInfoPtr = {
		.structname = "virNodeInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNodeInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNodeInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNodeInfoPtr, trace);
	KType_virNodeInfoPtr = ctvirNodeInfoPtr->typeId;

	KDEFINE_CLASS defvirNodeMemoryStatsPtr = {
		.structname = "virNodeMemoryStatsPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirNodeMemoryStatsPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirNodeMemoryStatsPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirNodeMemoryStatsPtr, trace);
	KType_virNodeMemoryStatsPtr = ctvirNodeMemoryStatsPtr->typeId;

	KDEFINE_CLASS defvirSecretPtr = {
		.structname = "virSecretPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirSecretPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirSecretPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirSecretPtr, trace);
	KType_virSecretPtr = ctvirSecretPtr->typeId;

	KDEFINE_CLASS defvirSecretPtrPtrPtr = {
		.structname = "virSecretPtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirSecretPtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirSecretPtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirSecretPtrPtrPtr, trace);
	KType_virSecretPtrPtrPtr = ctvirSecretPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirSecurityLabelPtr = {
		.structname = "virSecurityLabelPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirSecurityLabelPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirSecurityLabelPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirSecurityLabelPtr, trace);
	KType_virSecurityLabelPtr = ctvirSecurityLabelPtr->typeId;

	KDEFINE_CLASS defvirSecurityLabelPtrPtr = {
		.structname = "virSecurityLabelPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirSecurityLabelPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirSecurityLabelPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirSecurityLabelPtrPtr, trace);
	KType_virSecurityLabelPtrPtr = ctvirSecurityLabelPtrPtr->typeId;

	KDEFINE_CLASS defvirSecurityModelPtr = {
		.structname = "virSecurityModelPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirSecurityModelPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirSecurityModelPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirSecurityModelPtr, trace);
	KType_virSecurityModelPtr = ctvirSecurityModelPtr->typeId;

	KDEFINE_CLASS defvirStoragePoolInfoPtr = {
		.structname = "virStoragePoolInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStoragePoolInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStoragePoolInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStoragePoolInfoPtr, trace);
	KType_virStoragePoolInfoPtr = ctvirStoragePoolInfoPtr->typeId;

	KDEFINE_CLASS defvirStoragePoolPtr = {
		.structname = "virStoragePoolPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStoragePoolPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStoragePoolPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStoragePoolPtr, trace);
	KType_virStoragePoolPtr = ctvirStoragePoolPtr->typeId;

	KDEFINE_CLASS defvirStoragePoolPtrPtrPtr = {
		.structname = "virStoragePoolPtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStoragePoolPtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStoragePoolPtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStoragePoolPtrPtrPtr, trace);
	KType_virStoragePoolPtrPtrPtr = ctvirStoragePoolPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirStorageVolInfoPtr = {
		.structname = "virStorageVolInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStorageVolInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStorageVolInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStorageVolInfoPtr, trace);
	KType_virStorageVolInfoPtr = ctvirStorageVolInfoPtr->typeId;

	KDEFINE_CLASS defvirStorageVolPtr = {
		.structname = "virStorageVolPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStorageVolPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStorageVolPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStorageVolPtr, trace);
	KType_virStorageVolPtr = ctvirStorageVolPtr->typeId;

	KDEFINE_CLASS defvirStorageVolPtrPtrPtr = {
		.structname = "virStorageVolPtrPtrPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStorageVolPtrPtrPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStorageVolPtrPtrPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStorageVolPtrPtrPtr, trace);
	KType_virStorageVolPtrPtrPtr = ctvirStorageVolPtrPtrPtr->typeId;

	KDEFINE_CLASS defvirStreamEventCallback = {
		.structname = "virStreamEventCallback",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStreamEventCallback),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStreamEventCallback = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStreamEventCallback, trace);
	KType_virStreamEventCallback = ctvirStreamEventCallback->typeId;

	KDEFINE_CLASS defvirStreamPtr = {
		.structname = "virStreamPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStreamPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStreamPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStreamPtr, trace);
	KType_virStreamPtr = ctvirStreamPtr->typeId;

	KDEFINE_CLASS defvirStreamSinkFunc = {
		.structname = "virStreamSinkFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStreamSinkFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStreamSinkFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStreamSinkFunc, trace);
	KType_virStreamSinkFunc = ctvirStreamSinkFunc->typeId;

	KDEFINE_CLASS defvirStreamSourceFunc = {
		.structname = "virStreamSourceFunc",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirStreamSourceFunc),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirStreamSourceFunc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirStreamSourceFunc, trace);
	KType_virStreamSourceFunc = ctvirStreamSourceFunc->typeId;

	KDEFINE_CLASS defvirTypedParameterPtr = {
		.structname = "virTypedParameterPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirTypedParameterPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirTypedParameterPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirTypedParameterPtr, trace);
	KType_virTypedParameterPtr = ctvirTypedParameterPtr->typeId;

	KDEFINE_CLASS defvirVcpuInfoPtr = {
		.structname = "virVcpuInfoPtr",
		.typeId = KTypeAttr_NewId,
		.cstruct_size = sizeof(kvirVcpuInfoPtr),
		.cflag = KClassFlag_Final,
		.init = virt_common_ptr_Init,
		.free = virt_common_ptr_Free,
	};
	KClass *ctvirVcpuInfoPtr = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defvirVcpuInfoPtr, trace);
	KType_virVcpuInfoPtr = ctvirVcpuInfoPtr->typeId;

}
#endif /* end of include guard */
