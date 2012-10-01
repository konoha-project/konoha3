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
#include <minikonoha/float.h>
#include <minikonoha/bytes.h>
#include <stdio.h>
#define TRACE(POLICY, ...) OLDTRACE_SWITCH_TO_KTrace(POLICY, __VA_ARGS__);

#ifdef __cplusplus
extern "C" {
#endif

#include "mpi_glue.h"

/* ------------------------------------------------------------------------ */
kMPIComm *g_comm_world;

typedef struct {
	KonohaObjectHeader h;
	void *rawptr;
} kRawPtr;

static inline kObject *new_ReturnCppObject(KonohaContext *kctx,KonohaStack *sfp, void *ptr) {
	kObject *defobj = sfp[(-(K_CALLDELTA))].o;
	kObject *ret = KLIB new_kObject(kctx, O_ct(defobj), (uintptr_t)ptr);
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
			kBytes *b = (kBytes *)KLIB new_kObject(kctx, CT_Bytes, (uintptr_t)newSize);
			memcpy(b->buf, p->b->buf, p->size);
			p->b = b;
			p->size = newSize;
			break;
		}
		case KMPI_FARRAY: {
			kfloat_t *fa = KCALLOC(sizeof(kfloat_t), newSize);
			memcpy(fa, p->fa, p->size * sizeof(kfloat_t));
			KFREE(p->fa, p->size * sizeof(kfloat_t));
			p->fa = fa;
			p->size = newSize;
			break;
		}
		case KMPI_IARRAY: {
			kint_t *ia = KCALLOC(sizeof(kint_t), newSize);
			memcpy(ia, p->ia, p->size * sizeof(kint_t));
			KFREE(p->ia, p->size * sizeof(kint_t));
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
	kMPIRequest* p = (kMPIRequest*)KMALLOC(sizeof(kMPIRequest));
	return p;
}

static kMPIData *newMPIData(KonohaContext *kctx) {
	kMPIData* p = (kMPIData*)KMALLOC(sizeof(kMPIData));
	return p;
}

static void MPIRequest_ptr_free(KonohaContext *kctx , kObject *po)
{
	kMPIRequest *p = toRawPtr(kMPIRequest *, po);
	KFREE(p, sizeof(kMPIRequest));
}

static void MPIData_ptr_free(KonohaContext *kctx , kObject *po)
{
	kMPIData *p = toRawPtr(kMPIData *, po);
	switch(p->typeId) {
		case KMPI_BYTES: break;
		case KMPI_FARRAY: KFREE(p->fa, p->size * sizeof(kfloat_t)); break;
		case KMPI_IARRAY: KFREE(p->ia, p->size * sizeof(kint_t)); break;
		default: abort();
	}
	KFREE(p, sizeof(kMPIData));
}

/* ------------------------------------------------------------------------ */
//## float MPI.getWtime();
static KMETHOD MPI_getWtime(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(MPI_Wtime());
}

/* ------------------------------------------------------------------------ */
//## MPIComm MPIComm.getWorld();
static KMETHOD MPIComm_getWorld(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(new_ReturnCppObject(kctx, sfp, WRAP(g_comm_world)));
}

//## int MPIComm.getRank();
static KMETHOD MPIComm_getRank(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	int ret;
	MPI_Comm_rank(comm->comm, &ret);
	RETURNi_(ret);
}

//## int MPIComm.getSize();
static KMETHOD MPIComm_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	int ret;
	MPI_Comm_size(comm->comm, &ret);
	RETURNi_(ret);
}

//## boolean MPIComm.barrier();
static KMETHOD MPIComm_barrier(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG, LogText("at", "MPI_Barrier"), LogText("#", "begin"), LOG_END);
	bool ret = MPI_Barrier(comm->comm);
	OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG, LogText("at", "MPI_Barrier"), LogText("#", "finish"), LOG_END);
	RETURNb_(ret);
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
	RETURNb_(ret);
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
	RETURNb_(ret);
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
	RETURN_(new_ReturnCppObject(kctx, sfp, WRAP(ret)));
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
	RETURN_(new_ReturnCppObject(kctx, sfp, WRAP(ret)));
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
	RETURNb_(ret);
}

//## boolean MPIComm.scatter(MPIData sdata, int scount, MPIData rdata, int rcount, int root_rank);
static KMETHOD MPIComm_scatter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int scount = sfp[2].intValue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].o);
	int rcount = sfp[4].intValue;
	int root = sfp[5].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Scatter(getbuf(sdata), scount, sdata->type, 
			getbuf(rdata), rcount, rdata->type, root, comm->comm));
	RETURNb_(ret);
}

//## boolean MPIComm.gather(MPIData sdata, int scount, MPIData rdata, int rcount, int root_rank);
static KMETHOD MPIComm_gather(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int scount = sfp[2].intValue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].o);
	int rcount = sfp[4].intValue;
	int root = sfp[5].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Gather(getbuf(sdata), scount, sdata->type, 
			getbuf(rdata), rcount, rdata->type, root, comm->comm));
	RETURNb_(ret);
}

//## boolean MPIComm.allGather(MPIData sdata, int scount, MPIData rdata, int rcount);
static KMETHOD MPIComm_allGather(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int scount = sfp[2].intValue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].o);
	int rcount = sfp[4].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Allgather(getbuf(sdata), scount, sdata->type, getbuf(rdata),
			rcount, rdata->type, comm->comm));
	RETURNb_(ret);
}

//## boolean MPIComm.allToAll(MPIData sdata, int scount, MPIData rdata, int rcount);
static KMETHOD MPIComm_allToAll(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	int scount = sfp[2].intValue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].o);
	int rcount = sfp[4].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Alltoall(getbuf(sdata), scount, sdata->type, getbuf(rdata), 
			rcount, rdata->type, comm->comm));
	RETURNb_(ret);
}

//## boolean MPIComm.reduce(MPIData sdata, MPIData rdata, int rcount, MPIOp op, int root_rank);
static KMETHOD MPIComm_reduce(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].o);
	int rcount = sfp[3].intValue;
	MPI_Op op = (MPI_Op)sfp[4].intValue;
	int root = sfp[5].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Reduce(getbuf(sdata), getbuf(rdata), rcount, rdata->type, 
			op, root, comm->comm));
	RETURNb_(ret);
}

//## boolean MPIComm.allReduce(MPIData sdata, MPIData rdata, int rcount, MPIOp op);
static KMETHOD MPIComm_allReduce(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].o);
	int rcount = sfp[3].intValue;
	MPI_Op op = (MPI_Op)sfp[4].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Allreduce(getbuf(sdata), getbuf(rdata), rcount, rdata->type, 
			op, comm->comm));
	RETURNb_(ret);
}

//## boolean MPIComm.scan(MPIData sdata, MPIData rdata, int rcount, MPIOp op);
static KMETHOD MPIComm_scan(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].asObject);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].asObject);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].o);
	int rcount = sfp[3].intValue;
	MPI_Op op = (MPI_Op)sfp[4].intValue;
	MPIData_extend(kctx, rdata, rcount);
	bool ret = (MPI_Scan(getbuf(sdata), getbuf(rdata), rcount, rdata->type, op, comm->comm));
	RETURNb_(ret);
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
	RETURNb_(ret);
}

//## boolean MPIRequest.test()
static KMETHOD MPIRequest_test(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIRequest *req = toRawPtr(kMPIRequest *, sfp[0].asObject);
	MPI_Status stat;
	int flag;
	MPI_Test(&req->req, &flag, &stat);
	RETURNb_(flag != 0);
}

//## boolean MPIRequest.cancel()
static KMETHOD MPIRequest_cancel(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIRequest *req = toRawPtr(kMPIRequest *, sfp[0].asObject);
	bool ret = (MPI_Cancel(&req->req));
	RETURNb_(ret);
}

/* ------------------------------------------------------------------------ */
//## MPIData MPIData.fromBytes(Bytes b)
static KMETHOD MPIData_fromBytes(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *d = newMPIData(kctx);
	d->type = MPI_CHAR;
	d->b = sfp[1].ba;
	d->size = sfp[1].ba->bytesize;
	d->offset = 0;
	d->typeId = KMPI_BYTES;
	RETURN_(new_ReturnCppObject(kctx, sfp, WRAP(d)));
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
//	RETURN_(new_ReturnCppObject(kctx, sfp, WRAP(d)));
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
//	RETURN_(new_ReturnCppObject(kctx, sfp, WRAP(d)));
//}

//## Bytes MPIData.toBytes()
static KMETHOD MPIData_toBytes(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(toRawPtr(kMPIData *, sfp[0].asObject)->b);
}

//## void MPIData.setOffset(int offset)
static KMETHOD MPIData_setOffset(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].asObject);
	d->offset = sfp[1].intValue;
	RETURNvoid_();
}

//## int MPIData.getOffset()
static KMETHOD MPIData_getOffset(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].asObject);
	RETURNi_(d->offset);
}

//## int MPIData.getSize()
static KMETHOD MPIData_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].asObject);
	RETURNi_(d->size - d->offset);
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
	d->fa = KCALLOC(sizeof(kfloat_t), size);
	d->typeId = KMPI_FARRAY;
	RETURN_(new_ReturnCppObject(kctx, sfp, WRAP(d)));
}

//## MPIData MPIData.newIntArray(int size)
static KMETHOD MPIData_newIntArray(KonohaContext *kctx, KonohaStack *sfp)
{
	int size = sfp[1].intValue;
	kMPIData *d = newMPIData(kctx);
	d->type = MPI_LONG;
	d->size = size;
	d->offset = 0;
	d->fa = KCALLOC(sizeof(kint_t), size);
	d->typeId = KMPI_IARRAY;
	RETURN_(new_ReturnCppObject(kctx, sfp, WRAP(d)));
}

//## float MPIData.getf(int n)
static KMETHOD MPIData_getf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].asObject);
	int n = sfp[1].intValue + sdata->offset;
	RETURNf_(sdata->fa[n]);
}

//## void MPIData.setf(int n, float v)
static KMETHOD MPIData_setf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].asObject);
	int n = sfp[1].intValue + sdata->offset;
	double v = sfp[2].floatValue;
	sdata->fa[n] = v;
	RETURNvoid_();
}

//## int MPIData.geti(int n)
static KMETHOD MPIData_geti(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].asObject);
	int n = sfp[1].intValue + sdata->offset;
	RETURNi_(sdata->ia[n]);
}

//## void MPIData.seti(int n, int v)
static KMETHOD MPIData_seti(KonohaContext *kctx, KonohaStack *sfp)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].asObject);
	int n = sfp[1].intValue + sdata->offset;
	kint_t v = sfp[2].intValue;
	sdata->ia[n] = v;
	RETURNvoid_();
}

/* ------------------------------------------------------------------------ */
#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

typedef struct {
	KonohaModule h;
	KonohaClass *cValue;
} kmodmpi_t;

static void kmodmpi_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	(void)kctx;(void)def;(void)newctx;
}

static void kmodmpi_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
	(void)kctx;(void)baseh;
}

static void kmodmpi_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	MPI_Finalize();
}

#define MOD_mpi 19/*TODO*/

static kbool_t mpi_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("konoha.bytes", pline);
	KRequirePackage("konoha.float", pline);
	kmodmpi_t *base = (kmodmpi_t*)KCALLOC(sizeof(kmodmpi_t), 1);
	base->h.name     = "mpi";
	base->h.setup    = kmodmpi_setup;
	base->h.reftrace = kmodmpi_reftrace;
	base->h.free     = kmodmpi_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_mpi, &base->h, pline);

	MPI_Init(&argc, (char ***)&args);
	g_comm_world = (kMPIComm *)KMALLOC(sizeof(kMPIComm));
	g_comm_world->comm = MPI_COMM_WORLD;
	static KDEFINE_CLASS MPIDef = {
		"MPI"/*structname*/,
		TY_newid/*cid*/,  0/*cflag*/,
		0/*baseTypeId*/, 0/*superTypeId*/, 0/*cstruct_size*/,
		0/*fields*/, 0/*fieldsize*/, 0/*fieldAllocSize*/,
		0/*packageId*/, 0/*packageDomain*/,
		0/*init*/,
		0/*reftrace*/,
		0/*free*/,
		0/*fnull*/,
		0/*p*/, 0/*unbox*/,
		0/*compareTo*/,
		0/*getkey*/,
		0/*hashCode*/,
		0/*initdef*/
	};
	static KDEFINE_CLASS MPICommDef = {
		"MPIComm"/*structname*/,
		TY_newid/*cid*/,  0/*cflag*/,
		0/*baseTypeId*/, 0/*superTypeId*/, 0/*cstruct_size*/,
		0/*fields*/, 0/*fieldsize*/, 0/*fieldAllocSize*/,
		0/*packageId*/, 0/*packageDomain*/,
		0/*init*/,
		0/*reftrace*/,
		0/*free*/,
		0/*fnull*/,
		0/*p*/, 0/*unbox*/,
		0/*compareTo*/,
		0/*getkey*/,
		0/*hashCode*/,
		0/*initdef*/
	};
	static KDEFINE_CLASS MPIRequestDef = {
		"MPIRequest"/*structname*/,
		TY_newid/*cid*/,  0/*cflag*/,
		0/*baseTypeId*/, 0/*superTypeId*/, 0/*cstruct_size*/,
		0/*fields*/, 0/*fieldsize*/, 0/*fieldAllocSize*/,
		0/*packageId*/, 0/*packageDomain*/,
		0/*init*/,
		0/*reftrace*/,
		MPIRequest_ptr_free/*free*/,
		0/*fnull*/,
		0/*p*/, 0/*unbox*/,
		0/*compareTo*/,
		0/*getkey*/,
		0/*hashCode*/,
		0/*initdef*/
	};
	static KDEFINE_CLASS MPIDataDef = {
		"MPIData"/*structname*/,
		TY_newid/*cid*/,  0/*cflag*/,
		0/*baseTypeId*/, 0/*superTypeId*/, 0/*cstruct_size*/,
		0/*fields*/, 0/*fieldsize*/, 0/*fieldAllocSize*/,
		0/*packageId*/, 0/*packageDomain*/,
		0/*init*/,
		0/*reftrace*/,
		MPIData_ptr_free/*free*/,
		0/*fnull*/,
		0/*p*/, 0/*unbox*/,
		0/*compareTo*/,
		0/*getkey*/,
		0/*hashCode*/,
		0/*initdef*/
	};
	//static KDEFINE_CLASS MPIOpDef = {
	//	"MPIOp"/*structname*/,
	//	TY_newid/*cid*/,  0/*cflag*/,
	//	0/*baseTypeId*/, 0/*superTypeId*/, 0/*cstruct_size*/,
	//	0/*fields*/, 0/*fieldsize*/, 0/*fieldAllocSize*/,
	//	0/*packageId*/, 0/*packageDomain*/,
	//	0/*init*/,
	//	0/*reftrace*/,
	//	0/*free*/,
	//	0/*fnull*/,
	//	0/*p*/, 0/*unbox*/,
	//	0/*compareTo*/,
	//	0/*getkey*/,
	//	0/*hashCode*/,
	//	0/*initdef*/
	//};
	KonohaClass *CT_MPI = KLIB kNameSpace_defineClass(kctx, ns, NULL, &MPIDef, pline);
	KonohaClass *CT_MPIComm = KLIB kNameSpace_defineClass(kctx, ns, NULL, &MPICommDef, pline);
	KonohaClass *CT_MPIRequest = KLIB kNameSpace_defineClass(kctx, ns, NULL, &MPIRequestDef, pline);
	KonohaClass *CT_MPIData = KLIB kNameSpace_defineClass(kctx, ns, NULL, &MPIDataDef, pline);
	//KonohaClass *CT_MPIOp = KLIB kNameSpace_defineClass(kctx, ns, NULL, &MPIOpDef, pline);
#define TY_MPI         (CT_MPI->typeId)
#define TY_MPIComm     (CT_MPIComm->typeId)
#define TY_MPIRequest  (CT_MPIRequest->typeId)
#define TY_MPIData     (CT_MPIData->typeId)
#define TY_MPIOp       TY_int //(CT_MPIOp->typeId) // TODO
	KDEFINE_METHOD MethodData[] = {
		/* class MPI */
		_Public|_Static, _F(MPI_getWtime), TY_float, TY_MPI, MN_("getWtime"), 0,

		/* class MPIComm */
		_Public|_Static, _F(MPIComm_getWorld), TY_MPIComm, TY_MPIComm, MN_("getWorld"), 0,
		_Public, _F(MPIComm_getRank), TY_int, TY_MPIComm, MN_("getRank"), 0,
		_Public, _F(MPIComm_getSize), TY_int, TY_MPIComm, MN_("getSize"), 0,
		_Public, _F(MPIComm_barrier), TY_int, TY_MPIComm, MN_("barrier"), 0,
		_Public, _F(MPIComm_send), TY_boolean, TY_MPIComm, MN_("send"), 4,
			TY_MPIData, FN_("buf"), TY_int, FN_("count"), TY_int, FN_("dest"), TY_int, FN_("tag"),
		_Public, _F(MPIComm_recv), TY_boolean, TY_MPIComm, MN_("recv"), 4,
			TY_MPIData, FN_("buf"), TY_int, FN_("count"), TY_int, FN_("src"), TY_int, FN_("tag"),
		_Public, _F(MPIComm_iSend), TY_MPIRequest, TY_MPIComm, MN_("iSend"), 4,
			TY_MPIData, FN_("buf"), TY_int, FN_("count"), TY_int, FN_("dest"), TY_int, FN_("tag"),
		_Public, _F(MPIComm_iRecv), TY_MPIRequest, TY_MPIComm, MN_("iRecv"), 4,
			TY_MPIData, FN_("buf"), TY_int, FN_("count"), TY_int, FN_("src"), TY_int, FN_("tag"),
		_Public, _F(MPIComm_bcast), TY_boolean, TY_MPIComm, MN_("bcast"), 3,
			TY_MPIData, FN_("sdata"), TY_int, FN_("count"), TY_int, FN_("root"),
		_Public, _F(MPIComm_scatter), TY_boolean, TY_MPIComm, MN_("scatter"), 5,
			TY_MPIData, FN_("sdata"), TY_int, FN_("scount"), TY_MPIData, FN_("rdata"), TY_int, FN_("rcount"), TY_int, FN_("root"),
		_Public, _F(MPIComm_gather), TY_boolean, TY_MPIComm, MN_("gather"), 5,
			TY_MPIData, FN_("sdata"), TY_int, FN_("scount"), TY_MPIData, FN_("rdata"), TY_int, FN_("rcount"), TY_int, FN_("root"),
		_Public, _F(MPIComm_allGather), TY_boolean, TY_MPIComm, MN_("allGather"), 4,
			TY_MPIData, FN_("sdata"), TY_int, FN_("scount"), TY_MPIData, FN_("rdata"), TY_int, FN_("rcount"),
		_Public, _F(MPIComm_allToAll), TY_boolean, TY_MPIComm, MN_("allToAll"), 4,
			TY_MPIData, FN_("sdata"), TY_int, FN_("scount"), TY_MPIData, FN_("rdata"), TY_int, FN_("rcount"),
		_Public, _F(MPIComm_reduce), TY_boolean, TY_MPIComm, MN_("reduce"), 5,
			TY_MPIData, FN_("sdata"), TY_MPIData, FN_("rdata"), TY_int, FN_("rcount"), TY_MPIOp, FN_("op"), TY_int, FN_("root"),
		_Public, _F(MPIComm_allReduce), TY_boolean, TY_MPIComm, MN_("allReduce"), 4,
			TY_MPIData, FN_("sdata"), TY_MPIData, FN_("rdata"), TY_int, FN_("rcount"), TY_MPIOp, FN_("op"),
		_Public, _F(MPIComm_scan), TY_boolean, TY_MPIComm, MN_("scan"), 4,
			TY_MPIData, FN_("sdata"), TY_MPIData, FN_("rdata"), TY_int, FN_("rcount"), TY_MPIOp, FN_("op"),

		/* class MPIRequest */
		_Public, _F(MPIRequest_wait), TY_boolean, TY_MPIRequest, MN_("wait"), 0,
		_Public, _F(MPIRequest_test), TY_boolean, TY_MPIRequest, MN_("test"), 0,
		_Public, _F(MPIRequest_cancel), TY_boolean, TY_MPIRequest, MN_("cancel"), 0,

		/* class MPIData */
		_Public|_Static, _F(MPIData_fromBytes), TY_MPIData, TY_MPIData, MN_("fromBytes"), 1, TY_Bytes, FN_("b"),
		//_Public|_Static, _F(MPIData_fromIntArray), TY_MPIData, TY_MPIData, MN_("fromIntArray"), 1, TY_Array, FN_("b"),
		//_Public|_Static, _F(MPIData_fromFloatArray), TY_MPIData, TY_MPIData, MN_("fromFloatArray"), 1, TY_Array, FN_("b"),
		_Public|_Static, _F(MPIData_newFloatArray), TY_MPIData, TY_MPIData, MN_("newFloatArray"), 1, TY_int, FN_("n"),
		_Public|_Static, _F(MPIData_newIntArray), TY_MPIData, TY_MPIData, MN_("newIntArray"), 1, TY_int, FN_("n"),
		_Public, _F(MPIData_toBytes), TY_Bytes, TY_MPIData, MN_("toBytes"), 0,
		//_Public, _F(MPIData_toIntArray), TY_Array, TY_MPIData, MN_("toIntArray"), 0,
		//_Public, _F(MPIData_toFloatArray), TY_Array, TY_MPIData, MN_("toFloatArray"), 0,
		_Public, _F(MPIData_getf), TY_float, TY_MPIData, MN_("getf"), 1, TY_int, FN_("n"),
		_Public, _F(MPIData_setf), TY_void, TY_MPIData, MN_("setf"), 2, TY_int, FN_("n"), TY_float, FN_("v"),
		_Public, _F(MPIData_geti), TY_int, TY_MPIData, MN_("geti"), 1, TY_int, FN_("n"),
		_Public, _F(MPIData_seti), TY_void, TY_MPIData, MN_("seti"), 2, TY_int, FN_("n"), TY_int, FN_("v"),
		_Public, _F(MPIData_setOffset), TY_void, TY_MPIData, MN_("setOffset"), 1, TY_int, FN_("offset"),
		_Public, _F(MPIData_getOffset), TY_int, TY_MPIData, MN_("getOffset"), 0,
		_Public, _F(MPIData_getSize), TY_int, TY_MPIData, MN_("getSize"), 0, 
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, NULL, MethodData);
	KDEFINE_INT_CONST OpData[] = {
			{"MAX",  TY_MPIOp, (kint_t)MPI_MAX},
			{"MIN",  TY_MPIOp, (kint_t)MPI_MIN},
			{"SUM",  TY_MPIOp, (kint_t)MPI_SUM},
			{"PROD", TY_MPIOp, (kint_t)MPI_PROD},
			{"LAND", TY_MPIOp, (kint_t)MPI_LAND},
			{"BAND", TY_MPIOp, (kint_t)MPI_BAND},
			{"LOD",  TY_MPIOp, (kint_t)MPI_LOR},
			{"BOR",  TY_MPIOp, (kint_t)MPI_BOR},
			{"LXOR", TY_MPIOp, (kint_t)MPI_LXOR},
			{"BXOR", TY_MPIOp, (kint_t)MPI_BXOR},
			{}
	};
	KLIB kNameSpace_loadConstData(kctx, ns, (const char **)OpData, pline);
	return true;
}

static kbool_t mpi_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t mpi_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t mpi_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* mpi_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("mpi", "1.0"),
		.initPackage    = mpi_initPackage,
		.setupPackage   = mpi_setupPackage,
		.initNameSpace  = mpi_initNameSpace,
		.setupNameSpace = mpi_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
