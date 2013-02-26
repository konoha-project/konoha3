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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#define TRACE(POLICY, ...) OLDTRACE_SWITCH_TO_KTrace(POLICY, __VA_ARGS__);

#ifdef __cplusplus
extern "C" {
#endif

#include "mpi_glue.h"

/* ------------------------------------------------------------------------ */
kMPIComm *g_comm_world;

typedef struct {
	kObjectHeader h;
	void *rawptr;
} kRawPtr;

static inline kObject *new_ReturnCppObject(KonohaContext *kctx,KonohaStack *sfp, void *ptr) {
	kObject *defobj = sfp[(-(K_CALLDELTA))].asObject;
	kObject *ret = KLIB new_kObject(kctx, OnStack, kObject_class(defobj), (uintptr_t)ptr);
	((kRawPtr *)ret)->rawptr = ptr;
	return ret;
}

#define WRAP(a) a
#define toRawPtr(type, o) ((type)((kRawPtr *)o)->rawptr)

static void *getbuf(kMPIData *p) {
	switch(p->typeId) {
		case KMPI_BYTES:  return p->b->buf + p->offset;
		case KMPI_FARRAY: return p->fa + p->offset;
		case KMPI_IARRAY: return p->ia + p->offset;
		default: abort();
	}
}

static void MPIData_extend(KonohaContext *kctx, kMPIData *p, int size) {
	size_t newSize = p->offset + size;
	if(p->size < newSize) {
		switch(p->typeId) {
		case KMPI_BYTES: {
			kBytes *b = (kBytes *)KLIB new_kObject(kctx, OnField, KClass_Bytes, (uintptr_t)newSize);
			memcpy(b->buf, p->b->buf, p->size);
			p->b = b;
			p->size = newSize;
			break;
		}
		case KMPI_FARRAY: {
			kfloat_t *fa = KCalloc_UNTRACE(sizeof(kfloat_t), newSize);
			memcpy(fa, p->fa, p->size * sizeof(kfloat_t));
			KFree(p->fa, p->size * sizeof(kfloat_t));
			p->fa = fa;
			p->size = newSize;
			break;
		}
		case KMPI_IARRAY: {
			kint_t *ia = KCalloc_UNTRACE(sizeof(kint_t), newSize);
			memcpy(ia, p->ia, p->size * sizeof(kint_t));
			KFree(p->ia, p->size * sizeof(kint_t));
			p->ia = ia;
			p->size = newSize;
			break;
		}
		default:
			abort();
		}
	}
}

/* ------------------------------------------------------------------------ */
static kMPIRequest *newMPIRequest(KonohaContext *kctx) {
	kMPIRequest* p = (kMPIRequest *)KMalloc_UNTRACE(sizeof(kMPIRequest));
	return p;
}

static kMPIData *newMPIData(KonohaContext *kctx) {
	kMPIData* p = (kMPIData *)KMalloc_UNTRACE(sizeof(kMPIData));
	return p;
}

static void MPIRequest_ptr_Free(KonohaContext *kctx , kObject *po)
{
	kMPIRequest *p = toRawPtr(kMPIRequest *, po);
	KFree(p, sizeof(kMPIRequest));
}

static void MPIData_ptr_Free(KonohaContext *kctx , kObject *po)
{
	kMPIData *p = toRawPtr(kMPIData *, po);
	if (p != NULL) {
		switch(p->typeId) {
			case KMPI_BYTES: break;
			case KMPI_FARRAY: KFree(p->fa, p->size * sizeof(kfloat_t)); break;
			case KMPI_IARRAY: KFree(p->ia, p->size * sizeof(kint_t)); break;
			default: abort();
		}
		KFree(p, sizeof(kMPIData));
	}
}

/* ------------------------------------------------------------------------ */
//## float MPI.getWtime();
static KMETHOD MPI_getWtime(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(MPI_Wtime());
}

/* ------------------------------------------------------------------------ */
//## MPIComm MPIComm.getWorld();
static KMETHOD MPIComm_getWorld(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(new_ReturnCppObject(kctx, sfp, WRAP(g_comm_world)));
}

//## int MPIComm.getRank();
static KMETHOD MPIComm_getRank(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	int ret;
	MPI_Comm_rank(comm->comm, &ret);
	KReturnUnboxValue(ret);
}

//## int MPIComm.getSize();
static KMETHOD MPIComm_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	int ret;
	MPI_Comm_size(comm->comm, &ret);
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.barrier();
static KMETHOD MPIComm_barrier(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG, LogText("at", "MPI_Barrier"), LogText("#", "begin"), LOG_END);
	bool ret = MPI_Barrier(comm->comm);
	OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG, LogText("at", "MPI_Barrier"), LogText("#", "finish"), LOG_END);
	KReturnUnboxValue(ret);
}

/* ------------------------------------------------------------------------ */
//## boolean MPIComm.send(MPIData data, int count, int dest, int tag);
static KMETHOD MPIComm_send(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *data = toRawPtr(kMPIData *, sfp[1].asObject);
	int count = sfp[2].intValue;
	int dest = sfp[3].intValue;
	int tag = sfp[4].intValue;
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Send"), LogText("#", "begin"), LogUint("dest", dest), LogUint("count", count), LOG_END);
	bool ret = (MPI_Send(getbuf(data), count, data->type, dest, tag, comm->comm));
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Send"), LogText("#", "finish"), LOG_END);
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.recv(MPIData data, int count, int src, int tag);
static KMETHOD MPIComm_recv(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *data = toRawPtr(kMPIData *, sfp[1].asObject);
	int count = sfp[2].intValue;
	int src = sfp[3].intValue;
	int tag = sfp[4].intValue;
	MPIData_extend(kctx, data, count);
	MPI_Status stat;
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Recv"), LogText("#", "begin"), LogUint("src", src), LogUint("count", count), LOG_END);
	bool ret = (MPI_Recv(getbuf(data), count, data->type, src, tag, comm->comm, &stat));
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Recv"), LogText("#", "finish"), LOG_END);
	KReturnUnboxValue(ret);
}

//## MPIRequest MPIComm.iSend(Bytes buf, int count, int dest, int tag);
static KMETHOD MPIComm_iSend(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *data = toRawPtr(kMPIData *, sfp[1].asObject);
	int count = sfp[2].intValue;
	int dest = sfp[3].intValue;
	int tag = sfp[4].intValue;
	kMPIRequest *ret = newMPIRequest(kctx);
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Isend"), LogText("#", "begin"), LogUint("dest", dest), LogUint("count", count), LOG_END);
	MPI_Isend(getbuf(data), count, data->type, dest, tag, comm->comm, &ret->req);
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Isend"), LogText("#", "finish"), LOG_END);
	KReturn(new_ReturnCppObject(kctx, sfp, WRAP(ret)));
}

//## MPIRequest MPIComm.iRecv(Bytes buf, int count, int src, int tag);
static KMETHOD MPIComm_iRecv(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *data = toRawPtr(kMPIData *, sfp[1].asObject);
	int count = sfp[2].intValue;
	int src = sfp[3].intValue;
	int tag = sfp[4].intValue;
	kMPIRequest *ret = newMPIRequest(kctx);
	MPIData_extend(kctx, data, count);
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Irecv"), LogText("#", "begin"), LogUint("src", src), LogUint("count", count), LOG_END);
	MPI_Irecv(getbuf(data), count, data->type, src, tag, comm->comm, &ret->req);
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Irecv"), LogText("#", "finish"), LOG_END);
	KReturn(new_ReturnCppObject(kctx, sfp, WRAP(ret)));
}

/* ------------------------------------------------------------------------ */
//## boolean MPIComm.bcast(MPIData sdata, int count, int root_rank);
static KMETHOD MPIComm_bcast(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int count = sfp[2].intValue;
	int root = sfp[3].intValue;
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Bcast"), LogText("#", "begin"), LogUint("root", root), LogUint("count", count), LOG_END);
	bool ret = (MPI_Bcast(getbuf(sdata), count, sdata->type, root, comm->comm));
	TRACE(LOGPOL_DEBUG, LogText("at", "MPI_Bcast"), LogText("#", "finish"), LOG_END);
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.scatter(MPIData sdata, int scount, MPIData rdata, int rcount, int root_rank);
static KMETHOD MPIComm_scatter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int scount = sfp[2].intValue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].asObject);
	int rcount = sfp[4].intValue;
	int root = sfp[5].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Scatter(getbuf(sdata), scount, sdata->type, 
			getbuf(rdata), rcount, rdata->type, root, comm->comm));
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.gather(MPIData sdata, int scount, MPIData rdata, int rcount, int root_rank);
static KMETHOD MPIComm_gather(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int scount = sfp[2].intValue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].asObject);
	int rcount = sfp[4].intValue;
	int root = sfp[5].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Gather(getbuf(sdata), scount, sdata->type, 
			getbuf(rdata), rcount, rdata->type, root, comm->comm));
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.allGather(MPIData sdata, int scount, MPIData rdata, int rcount);
static KMETHOD MPIComm_allGather(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int scount = sfp[2].intValue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].asObject);
	int rcount = sfp[4].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Allgather(getbuf(sdata), scount, sdata->type, getbuf(rdata),
			rcount, rdata->type, comm->comm));
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.allToAll(MPIData sdata, int scount, MPIData rdata, int rcount);
static KMETHOD MPIComm_allToAll(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int scount = sfp[2].intValue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].asObject);
	int rcount = sfp[4].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Alltoall(getbuf(sdata), scount, sdata->type, getbuf(rdata), 
			rcount, rdata->type, comm->comm));
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.reduce(MPIData sdata, MPIData rdata, int rcount, MPIOp op, int root_rank);
static KMETHOD MPIComm_reduce(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].asObject);
	int rcount = sfp[3].intValue;
	MPI_Op op = (MPI_Op)sfp[4].intValue;
	int root = sfp[5].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Reduce(getbuf(sdata), getbuf(rdata), rcount, rdata->type, 
			op, root, comm->comm));
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.allReduce(MPIData sdata, MPIData rdata, int rcount, MPIOp op);
static KMETHOD MPIComm_allReduce(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].asObject);
	int rcount = sfp[3].intValue;
	MPI_Op op = (MPI_Op)sfp[4].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Allreduce(getbuf(sdata), getbuf(rdata), rcount, rdata->type, 
			op, comm->comm));
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.scan(MPIData sdata, MPIData rdata, int rcount, MPIOp op);
static KMETHOD MPIComm_scan(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].asObject);
	int rcount = sfp[3].intValue;
	MPI_Op op = (MPI_Op)sfp[4].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Scan(getbuf(sdata), getbuf(rdata), rcount, rdata->type, op, comm->comm));
	KReturnUnboxValue(ret);
}

//## boolean MPIComm.reduceScatter(MPIData sdata, MPIData rdata, int[] rcounts, MPIOp op);
//static KMETHOD MPIComm_reduceScatter(KonohaContext *kctx, KonohaStack *sfp)
//{
//	//TODO
//}

/* ------------------------------------------------------------------------ */
//## boolean MPIRequest.wait()
static KMETHOD MPIRequest_wait(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIRequest *req = toRawPtr(kMPIRequest *, sfp[0].asObject);
	MPI_Status stat;
	bool ret = (MPI_Wait(&req->req, &stat));
	KReturnUnboxValue(ret);
}

//## boolean MPIRequest.test()
static KMETHOD MPIRequest_test(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIRequest *req = toRawPtr(kMPIRequest *, sfp[0].asObject);
	MPI_Status stat;
	int flag;
	MPI_Test(&req->req, &flag, &stat);
	KReturnUnboxValue(flag != 0);
}

//## boolean MPIRequest.cancel()
static KMETHOD MPIRequest_cancel(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIRequest *req = toRawPtr(kMPIRequest *, sfp[0].asObject);
	bool ret = (MPI_Cancel(&req->req));
	KReturnUnboxValue(ret);
}

/* ------------------------------------------------------------------------ */
//## MPIData MPIData.fromBytes(Bytes b)
static KMETHOD MPIData_fromBytes(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *d = newMPIData(kctx);
	d->type = MPI_CHAR;
	d->b = sfp[1].asBytes;
	d->size = sfp[1].asBytes->bytesize;
	d->offset = 0;
	d->typeId = KMPI_BYTES;
	KReturn(new_ReturnCppObject(kctx, sfp, WRAP(d)));
}

////## MPIData MPIData.fromIntArray(Array[int] a)
//static KMETHOD MPIData_fromIntArray(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kMPIData *d = newMPIData(kctx);
//	d->type = MPI_LONG;
//	d->size = 0;
//	d->offset = 0;
//	d->a = sfp[1].asArray;
//	d->typeId = KMPI_IARRAY;
//	KReturn(new_ReturnCppObject(kctx, sfp, WRAP(d)));
//}
//
////## MPIData MPIData.fromFloatArray(Array[float] a)
//static KMETHOD MPIData_fromFloatArray(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kMPIData *d = newMPIData(kctx);
//	d->type = MPI_DOUBLE;
//	d->size = 0;
//	d->offset = 0;
//	d->a = sfp[1].asArray;
//	d->typeId = KMPI_FARRAY;
//	KReturn(new_ReturnCppObject(kctx, sfp, WRAP(d)));
//}

//## Bytes MPIData.toBytes()
static KMETHOD MPIData_toBytes(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(toRawPtr(kMPIData *, sfp[0].asObject)->b);
}

//## void MPIData.setOffset(int offset)
static KMETHOD MPIData_SetOffset(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].asObject);
	d->offset = sfp[1].intValue;
	KReturnVoid();
}

//## int MPIData.getOffset()
static KMETHOD MPIData_getOffset(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].asObject);
	KReturnUnboxValue(d->offset);
}

//## int MPIData.getSize()
static KMETHOD MPIData_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].asObject);
	KReturnUnboxValue(d->size - d->offset);
}

/* ------------------------------------------------------------------------ */
//## MPIData MPIData.newFloatArray(int size)
static KMETHOD MPIData_newFloatArray(KonohaContext *kctx, KonohaStack *sfp)
{
	int size = sfp[1].intValue;
	kMPIData *d = newMPIData(kctx);
	d->type = MPI_DOUBLE;
	d->size = size;
	d->offset = 0;
	d->fa = KCalloc_UNTRACE(sizeof(kfloat_t), size);
	d->typeId = KMPI_FARRAY;
	KReturn(new_ReturnCppObject(kctx, sfp, WRAP(d)));
}

//## MPIData MPIData.newIntArray(int size)
static KMETHOD MPIData_newIntArray(KonohaContext *kctx, KonohaStack *sfp)
{
	int size = sfp[1].intValue;
	kMPIData *d = newMPIData(kctx);
	d->type = MPI_LONG;
	d->size = size;
	d->offset = 0;
	d->fa = KCalloc_UNTRACE(sizeof(kint_t), size);
	d->typeId = KMPI_IARRAY;
	KReturn(new_ReturnCppObject(kctx, sfp, WRAP(d)));
}

//## float MPIData.getf(int n)
static KMETHOD MPIData_getf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].asObject);
	int n = sfp[1].intValue + sdata->offset;
	KReturnFloatValue(sdata->fa[n]);
}

//## void MPIData.setf(int n, float v)
static KMETHOD MPIData_Setf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].asObject);
	int n = sfp[1].intValue + sdata->offset;
	double v = sfp[2].floatValue;
	sdata->fa[n] = v;
	KReturnVoid();
}

//## int MPIData.geti(int n)
static KMETHOD MPIData_geti(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].asObject);
	int n = sfp[1].intValue + sdata->offset;
	KReturnUnboxValue(sdata->ia[n]);
}

//## void MPIData.seti(int n, int v)
static KMETHOD MPIData_Seti(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].asObject);
	int n = sfp[1].intValue + sdata->offset;
	kint_t v = sfp[2].intValue;
	sdata->ia[n] = v;
	KReturnVoid();
}

/* ------------------------------------------------------------------------ */

typedef struct {
	KRuntimeModule h;
	KClass *cValue;
} KModuleMpi;

static void MpiModule_Setup(KonohaContext *kctx, struct KRuntimeModule *def, int newctx)
{
	(void)kctx;(void)def;(void)newctx;
}

static void MpiModule_Free(KonohaContext *kctx, struct KRuntimeModule *baseh)
{
	MPI_Finalize();
}

static kbool_t mpi_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("Type.Bytes", trace);
	KRequirePackage("Type.Float", trace);
	KModuleMpi *mod = (KModuleMpi *)KCalloc_UNTRACE(sizeof(KModuleMpi), 1);
	mod->h.name     = "mpi";
	mod->h.setupModuleContext    = MpiModule_Setup;
	mod->h.freeModule     = MpiModule_Free;
	KLIB KRuntime_SetModule(kctx, 19/*MOD_mpi*/, (KRuntimeModule *)mod, trace);

	int argc = 0;
	char *args[1] = {};
	MPI_Init(&argc, (char ***)&args);
	g_comm_world = (kMPIComm *)KMalloc_UNTRACE(sizeof(kMPIComm));
	g_comm_world->comm = MPI_COMM_WORLD;
	static KDEFINE_CLASS MPIDef = {
		.structname = "MPI",
		.typeId = KTypeAttr_NewId
	};
	static KDEFINE_CLASS MPICommDef = {
		.structname = "MPIComm",
		.typeId = KTypeAttr_NewId
	};
	static KDEFINE_CLASS MPIRequestDef = {
		.structname = "MPIRequest",
		.typeId = KTypeAttr_NewId,
		.free = MPIRequest_ptr_Free
	};
	static KDEFINE_CLASS MPIDataDef = {
		.structname = "MPIData",
		.typeId = KTypeAttr_NewId,
		.free = MPIData_ptr_Free
	};
	//static KDEFINE_CLASS MPIOpDef = {
	//	.structname = "MPIOp",
	//	.typeId = KTypeAttr_NewId
	//};
	KClass *KClass_MPI = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MPIDef, trace);
	KClass *KClass_MPIComm = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MPICommDef, trace);
	KClass *KClass_MPIRequest = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MPIRequestDef, trace);
	KClass *KClass_MPIData = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MPIDataDef, trace);
	//KClass *KClass_MPIOp = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MPIOpDef, trace);
#define KType_MPI         (KClass_MPI->typeId)
#define KType_MPIComm     (KClass_MPIComm->typeId)
#define KType_MPIRequest  (KClass_MPIRequest->typeId)
#define KType_MPIData     (KClass_MPIData->typeId)
#define KType_MPIOp       KType_Int //(KClass_MPIOp->typeId) // TODO
	KDEFINE_METHOD MethodData[] = {
		/* class MPI */
		_Public|_Static, _F(MPI_getWtime), KType_float, KType_MPI, KMethodName_("getWtime"), 0,

		/* class MPIComm */
		_Public|_Static, _F(MPIComm_getWorld), KType_MPIComm, KType_MPIComm, KMethodName_("getWorld"), 0,
		_Public, _F(MPIComm_getRank), KType_Int, KType_MPIComm, KMethodName_("getRank"), 0,
		_Public, _F(MPIComm_getSize), KType_Int, KType_MPIComm, KMethodName_("getSize"), 0,
		_Public, _F(MPIComm_barrier), KType_Int, KType_MPIComm, KMethodName_("barrier"), 0,
		_Public, _F(MPIComm_send), KType_Boolean, KType_MPIComm, KMethodName_("send"), 4,
			KType_MPIData, KFieldName_("buf"), KType_Int, KFieldName_("count"), KType_Int, KFieldName_("dest"), KType_Int, KFieldName_("tag"),
		_Public, _F(MPIComm_recv), KType_Boolean, KType_MPIComm, KMethodName_("recv"), 4,
			KType_MPIData, KFieldName_("buf"), KType_Int, KFieldName_("count"), KType_Int, KFieldName_("src"), KType_Int, KFieldName_("tag"),
		_Public, _F(MPIComm_iSend), KType_MPIRequest, KType_MPIComm, KMethodName_("iSend"), 4,
			KType_MPIData, KFieldName_("buf"), KType_Int, KFieldName_("count"), KType_Int, KFieldName_("dest"), KType_Int, KFieldName_("tag"),
		_Public, _F(MPIComm_iRecv), KType_MPIRequest, KType_MPIComm, KMethodName_("iRecv"), 4,
			KType_MPIData, KFieldName_("buf"), KType_Int, KFieldName_("count"), KType_Int, KFieldName_("src"), KType_Int, KFieldName_("tag"),
		_Public, _F(MPIComm_bcast), KType_Boolean, KType_MPIComm, KMethodName_("bcast"), 3,
			KType_MPIData, KFieldName_("sdata"), KType_Int, KFieldName_("count"), KType_Int, KFieldName_("root"),
		_Public, _F(MPIComm_scatter), KType_Boolean, KType_MPIComm, KMethodName_("scatter"), 5,
			KType_MPIData, KFieldName_("sdata"), KType_Int, KFieldName_("scount"), KType_MPIData, KFieldName_("rdata"), KType_Int, KFieldName_("rcount"), KType_Int, KFieldName_("root"),
		_Public, _F(MPIComm_gather), KType_Boolean, KType_MPIComm, KMethodName_("gather"), 5,
			KType_MPIData, KFieldName_("sdata"), KType_Int, KFieldName_("scount"), KType_MPIData, KFieldName_("rdata"), KType_Int, KFieldName_("rcount"), KType_Int, KFieldName_("root"),
		_Public, _F(MPIComm_allGather), KType_Boolean, KType_MPIComm, KMethodName_("allGather"), 4,
			KType_MPIData, KFieldName_("sdata"), KType_Int, KFieldName_("scount"), KType_MPIData, KFieldName_("rdata"), KType_Int, KFieldName_("rcount"),
		_Public, _F(MPIComm_allToAll), KType_Boolean, KType_MPIComm, KMethodName_("allToAll"), 4,
			KType_MPIData, KFieldName_("sdata"), KType_Int, KFieldName_("scount"), KType_MPIData, KFieldName_("rdata"), KType_Int, KFieldName_("rcount"),
		_Public, _F(MPIComm_reduce), KType_Boolean, KType_MPIComm, KMethodName_("reduce"), 5,
			KType_MPIData, KFieldName_("sdata"), KType_MPIData, KFieldName_("rdata"), KType_Int, KFieldName_("rcount"), KType_MPIOp, KFieldName_("op"), KType_Int, KFieldName_("root"),
		_Public, _F(MPIComm_allReduce), KType_Boolean, KType_MPIComm, KMethodName_("allReduce"), 4,
			KType_MPIData, KFieldName_("sdata"), KType_MPIData, KFieldName_("rdata"), KType_Int, KFieldName_("rcount"), KType_MPIOp, KFieldName_("op"),
		_Public, _F(MPIComm_scan), KType_Boolean, KType_MPIComm, KMethodName_("scan"), 4,
			KType_MPIData, KFieldName_("sdata"), KType_MPIData, KFieldName_("rdata"), KType_Int, KFieldName_("rcount"), KType_MPIOp, KFieldName_("op"),

		/* class MPIRequest */
		_Public, _F(MPIRequest_wait), KType_Boolean, KType_MPIRequest, KMethodName_("wait"), 0,
		_Public, _F(MPIRequest_test), KType_Boolean, KType_MPIRequest, KMethodName_("test"), 0,
		_Public, _F(MPIRequest_cancel), KType_Boolean, KType_MPIRequest, KMethodName_("cancel"), 0,

		/* class MPIData */
		_Public|_Static, _F(MPIData_fromBytes), KType_MPIData, KType_MPIData, KMethodName_("fromBytes"), 1, KType_Bytes, KFieldName_("b"),
		//_Public|_Static, _F(MPIData_fromIntArray), KType_MPIData, KType_MPIData, KMethodName_("fromIntArray"), 1, KType_Array, KFieldName_("b"),
		//_Public|_Static, _F(MPIData_fromFloatArray), KType_MPIData, KType_MPIData, KMethodName_("fromFloatArray"), 1, KType_Array, KFieldName_("b"),
		_Public|_Static, _F(MPIData_newFloatArray), KType_MPIData, KType_MPIData, KMethodName_("newFloatArray"), 1, KType_Int, KFieldName_("n"),
		_Public|_Static, _F(MPIData_newIntArray), KType_MPIData, KType_MPIData, KMethodName_("newIntArray"), 1, KType_Int, KFieldName_("n"),
		_Public, _F(MPIData_toBytes), KType_Bytes, KType_MPIData, KMethodName_("toBytes"), 0,
		//_Public, _F(MPIData_toIntArray), KType_Array, KType_MPIData, KMethodName_("toIntArray"), 0,
		//_Public, _F(MPIData_toFloatArray), KType_Array, KType_MPIData, KMethodName_("toFloatArray"), 0,
		_Public, _F(MPIData_getf), KType_float, KType_MPIData, KMethodName_("getf"), 1, KType_Int, KFieldName_("n"),
		_Public, _F(MPIData_Setf), KType_void, KType_MPIData, KMethodName_("setf"), 2, KType_Int, KFieldName_("n"), KType_float, KFieldName_("v"),
		_Public, _F(MPIData_geti), KType_Int, KType_MPIData, KMethodName_("geti"), 1, KType_Int, KFieldName_("n"),
		_Public, _F(MPIData_Seti), KType_void, KType_MPIData, KMethodName_("seti"), 2, KType_Int, KFieldName_("n"), KType_Int, KFieldName_("v"),
		_Public, _F(MPIData_SetOffset), KType_void, KType_MPIData, KMethodName_("setOffset"), 1, KType_Int, KFieldName_("offset"),
		_Public, _F(MPIData_getOffset), KType_Int, KType_MPIData, KMethodName_("getOffset"), 0,
		_Public, _F(MPIData_getSize), KType_Int, KType_MPIData, KMethodName_("getSize"), 0, 
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	KDEFINE_INT_CONST OpData[] = {
			{"MAX",  KType_MPIOp, (kint_t)MPI_MAX},
			{"MIN",  KType_MPIOp, (kint_t)MPI_MIN},
			{"SUM",  KType_MPIOp, (kint_t)MPI_SUM},
			{"PROD", KType_MPIOp, (kint_t)MPI_PROD},
			{"LAND", KType_MPIOp, (kint_t)MPI_LAND},
			{"BAND", KType_MPIOp, (kint_t)MPI_BAND},
			{"LOD",  KType_MPIOp, (kint_t)MPI_LOR},
			{"BOR",  KType_MPIOp, (kint_t)MPI_BOR},
			{"LXOR", KType_MPIOp, (kint_t)MPI_LXOR},
			{"BXOR", KType_MPIOp, (kint_t)MPI_BXOR},
			{}
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(OpData), trace);
	return true;
}

static kbool_t mpi_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Mpi_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("mpi", "1.0"),
		.PackupNameSpace    = mpi_PackupNameSpace,
		.ExportNameSpace   = mpi_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
