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

#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>
#include <konoha2/float.h>
//#include <konoha2/bytes.h>
#include "mpi_glue.h"
#include "../konoha.bytes/bytes_glue.h"//FIXME

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
kMPIComm *g_comm_world;

typedef struct {
	kObjectHeader h;
	void *rawptr;
} kRawPtr;

static inline kObject *new_ReturnCppObject(CTX,ksfp_t *sfp, void *ptr _RIX) {
	kObject *defobj = sfp[K_RIX].o;
	kObject *ret = new_kObject(O_ct(defobj), ptr);
	((kRawPtr *)ret)->rawptr = ptr;
	return ret;
}

#define WRAP(a) a
#define toRawPtr(type, o) ((type)((kRawPtr *)o)->rawptr)

static void *getbuf(kMPIData *p) {
	switch(p->cid) {
		case KMPI_BYTES:  return p->b->buf + p->offset;
		case KMPI_FARRAY: return p->fa + p->offset;
		case KMPI_IARRAY: return p->ia + p->offset;
		default: abort();
	}
} 

static void MPIData_extend(CTX, kMPIData *p, int size) {
	size_t newSize = p->offset + size;
	if(p->size < newSize) {
		switch(p->cid) {
		case KMPI_BYTES: {
			kBytes *b = (kBytes *)new_kObject(CT_Bytes, (void *)newSize);
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
static kMPIRequest *newMPIRequest(CTX) {
	kMPIRequest* p = (kMPIRequest*)KMALLOC(sizeof(kMPIRequest));
	return p;
}

static kMPIData *newMPIData(CTX) {
	kMPIData* p = (kMPIData*)KMALLOC(sizeof(kMPIData));
	return p;
}

static void MPIRequest_ptr_free(CTX , kObject *po)
{
	kMPIRequest *p = toRawPtr(kMPIRequest *, po);
	KFREE(p, sizeof(kMPIRequest));
}

static void MPIData_ptr_free(CTX , kObject *po)
{
	kMPIData *p = toRawPtr(kMPIData *, po);
	switch(p->cid) {
		case KMPI_BYTES: break;
		case KMPI_FARRAY: KFREE(p->fa, p->size * sizeof(kfloat_t)); break;
		case KMPI_IARRAY: KFREE(p->ia, p->size * sizeof(kint_t)); break;
		default: abort();
	}
	KFREE(p, sizeof(kMPIData));
}

/* ------------------------------------------------------------------------ */
//## float MPI.getWtime();
static KMETHOD MPI_getWtime(CTX, ksfp_t *sfp _RIX)
{
	RETURNf_(MPI_Wtime());
}

/* ------------------------------------------------------------------------ */
//## MPIComm MPIComm.getWorld();
static KMETHOD MPIComm_getWorld(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(new_ReturnCppObject(_ctx, sfp, WRAP(g_comm_world) K_RIXPARAM));
}

//## int MPIComm.getRank();
static KMETHOD MPIComm_getRank(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	int ret;
	MPI_Comm_rank(comm->comm, &ret);
	RETURNi_(ret);
}

//## int MPIComm.getSize();
static KMETHOD MPIComm_getSize(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	int ret;
	MPI_Comm_size(comm->comm, &ret);
	RETURNi_(ret);
}

//## boolean MPIComm.barrier();
static KMETHOD MPIComm_barrier(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	RETURNb_(MPI_Barrier(comm->comm));
}

/* ------------------------------------------------------------------------ */
//## boolean MPIComm.send(MPIData data, int count, int dest, int tag);
static KMETHOD MPIComm_send(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *data = toRawPtr(kMPIData *, sfp[1].o);
	int count = sfp[2].ivalue;
	int dest = sfp[3].ivalue;
	int tag = sfp[4].ivalue;
	RETURNb_(MPI_Send(getbuf(data), count, data->type, dest, tag, comm->comm));
}

//## boolean MPIComm.recv(MPIData data, int count, int src, int tag);
static KMETHOD MPIComm_recv(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *data = toRawPtr(kMPIData *, sfp[1].o);
	int count = sfp[2].ivalue;
	int src = sfp[3].ivalue;
	int tag = sfp[4].ivalue;
	MPIData_extend(_ctx, data, count);
	MPI_Status stat;
	RETURNb_(MPI_Recv(getbuf(data), count, data->type, src, tag, comm->comm, &stat));
}

//## MPIRequest MPIComm.iSend(Bytes buf, int count, int dest, int tag);
static KMETHOD MPIComm_iSend(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *data = toRawPtr(kMPIData *, sfp[1].o);
	int count = sfp[2].ivalue;
	int dest = sfp[3].ivalue;
	int tag = sfp[4].ivalue;
	kMPIRequest *ret = newMPIRequest(_ctx);
	MPI_Isend(getbuf(data), count, data->type, dest, tag, comm->comm, &ret->req);
	RETURN_(new_ReturnCppObject(_ctx, sfp, WRAP(ret) K_RIXPARAM));
}

//## MPIRequest MPIComm.iRecv(Bytes buf, int count, int src, int tag);
static KMETHOD MPIComm_iRecv(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *data = toRawPtr(kMPIData *, sfp[1].o);
	int count = sfp[2].ivalue;
	int src = sfp[3].ivalue;
	int tag = sfp[4].ivalue;
	kMPIRequest *ret = newMPIRequest(_ctx);
	MPIData_extend(_ctx, data, count);
	MPI_Irecv(getbuf(data), count, data->type, src, tag, comm->comm, &ret->req);
	RETURN_(new_ReturnCppObject(_ctx, sfp, WRAP(ret) K_RIXPARAM));
}

/* ------------------------------------------------------------------------ */
//## boolean MPIComm.bcast(MPIData sdata, int count, int root_rank);
static KMETHOD MPIComm_bcast(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].o);
	int count = sfp[2].ivalue;
	int root = sfp[3].ivalue;
	RETURNb_(MPI_Bcast(getbuf(sdata), count, sdata->type, root, comm->comm));
}

//## boolean MPIComm.scatter(MPIData sdata, int scount, MPIData rdata, int rcount, int root_rank);
static KMETHOD MPIComm_scatter(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].o);
	int scount = sfp[2].ivalue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].o);
	int rcount = sfp[4].ivalue;
	int root = sfp[5].ivalue;
	MPIData_extend(_ctx, rdata, rcount);
	RETURNb_(MPI_Scatter(getbuf(sdata), scount, sdata->type, 
			getbuf(rdata), rcount, rdata->type, root, comm->comm));
}

//## boolean MPIComm.gather(MPIData sdata, int scount, MPIData rdata, int rcount, int root_rank);
static KMETHOD MPIComm_gather(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].o);
	int scount = sfp[2].ivalue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].o);
	int rcount = sfp[4].ivalue;
	int root = sfp[5].ivalue;
	MPIData_extend(_ctx, rdata, rcount);
	RETURNb_(MPI_Gather(getbuf(sdata), scount, sdata->type, 
			getbuf(rdata), rcount, rdata->type, root, comm->comm));
}

//## boolean MPIComm.allGather(MPIData sdata, int scount, MPIData rdata, int rcount);
static KMETHOD MPIComm_allGather(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].o);
	int scount = sfp[2].ivalue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].o);
	int rcount = sfp[4].ivalue;
	MPIData_extend(_ctx, rdata, rcount);
	RETURNb_(MPI_Allgather(getbuf(sdata), scount, sdata->type, getbuf(rdata),
			rcount, rdata->type, comm->comm));
}

//## boolean MPIComm.allToAll(MPIData sdata, int scount, MPIData rdata, int rcount);
static KMETHOD MPIComm_allToAll(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].o);
	int scount = sfp[2].ivalue;
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[3].o);
	int rcount = sfp[4].ivalue;
	MPIData_extend(_ctx, rdata, rcount);
	RETURNb_(MPI_Alltoall(getbuf(sdata), scount, sdata->type, getbuf(rdata), 
			rcount, rdata->type, comm->comm));
}

//## boolean MPIComm.reduce(MPIData sdata, MPIData rdata, int rcount, MPIOp op, int root_rank);
static KMETHOD MPIComm_reduce(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].o);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].o);
	int rcount = sfp[3].ivalue;
	MPI_Op op = (MPI_Op)sfp[4].ivalue;
	int root = sfp[5].ivalue;
	MPIData_extend(_ctx, rdata, rcount);
	RETURNb_(MPI_Reduce(getbuf(sdata), getbuf(rdata), rcount, rdata->type, 
			op, root, comm->comm));
}

//## boolean MPIComm.allReduce(MPIData sdata, MPIData rdata, int rcount, MPIOp op);
static KMETHOD MPIComm_allReduce(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].o);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].o);
	int rcount = sfp[3].ivalue;
	MPI_Op op = (MPI_Op)sfp[4].ivalue;
	MPIData_extend(_ctx, rdata, rcount);
	RETURNb_(MPI_Allreduce(getbuf(sdata), getbuf(rdata), rcount, rdata->type, 
			op, comm->comm));
}

//## boolean MPIComm.scan(MPIData sdata, MPIData rdata, int rcount, MPIOp op);
static KMETHOD MPIComm_scan(CTX, ksfp_t *sfp _RIX)
{
	kMPIComm *comm = toRawPtr(kMPIComm *, sfp[0].o);
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[1].o);
	kMPIData *rdata = toRawPtr(kMPIData *, sfp[2].o);
	int rcount = sfp[3].ivalue;
	MPI_Op op = (MPI_Op)sfp[4].ivalue;
	MPIData_extend(_ctx, rdata, rcount);
	RETURNb_(MPI_Scan(getbuf(sdata), getbuf(rdata), rcount, rdata->type, op, comm->comm));
}

//## boolean MPIComm.reduceScatter(MPIData sdata, MPIData rdata, int[] rcounts, MPIOp op);
//static KMETHOD MPIComm_reduceScatter(CTX, ksfp_t *sfp _RIX)
//{
//	//TODO
//}

/* ------------------------------------------------------------------------ */
//## boolean MPIRequest.wait()
static KMETHOD MPIRequest_wait(CTX, ksfp_t *sfp _RIX)
{
	kMPIRequest *req = toRawPtr(kMPIRequest *, sfp[0].o);
	MPI_Status stat;
	RETURNb_(MPI_Wait(&req->req, &stat));
}

//## boolean MPIRequest.test()
static KMETHOD MPIRequest_test(CTX, ksfp_t *sfp _RIX)
{
	kMPIRequest *req = toRawPtr(kMPIRequest *, sfp[0].o);
	MPI_Status stat;
	int flag;
	MPI_Test(&req->req, &flag, &stat);
	RETURNb_(flag != 0);
}

//## boolean MPIRequest.cancel()
static KMETHOD MPIRequest_cancel(CTX, ksfp_t *sfp _RIX)
{
	kMPIRequest *req = toRawPtr(kMPIRequest *, sfp[0].o);
	RETURNb_(MPI_Cancel(&req->req));
}

/* ------------------------------------------------------------------------ */
//## MPIData MPIData.fromBytes(Bytes b)
static KMETHOD MPIData_fromBytes(CTX, ksfp_t *sfp _RIX)
{
	kMPIData *d = newMPIData(_ctx);
	d->type = MPI_CHAR;
	d->b = sfp[1].ba;
	d->size = sfp[1].ba->bytesize;
	d->offset = 0;
	d->cid = KMPI_BYTES;
	RETURN_(new_ReturnCppObject(_ctx, sfp, WRAP(d) K_RIXPARAM));
}

////## MPIData MPIData.fromIntArray(Array[int] a)
//static KMETHOD MPIData_fromIntArray(CTX, ksfp_t *sfp _RIX)
//{
//	kMPIData *d = newMPIData(_ctx);
//	d->type = MPI_LONG;
//	d->size = 0;
//	d->offset = 0;
//	d->a = sfp[1].a;
//	d->cid = KMPI_IARRAY;
//	RETURN_(new_ReturnCppObject(_ctx, sfp, WRAP(d) K_RIXPARAM));
//}
//
////## MPIData MPIData.fromFloatArray(Array[float] a)
//static KMETHOD MPIData_fromFloatArray(CTX, ksfp_t *sfp _RIX)
//{
//	kMPIData *d = newMPIData(_ctx);
//	d->type = MPI_DOUBLE;
//	d->size = 0;
//	d->offset = 0;
//	d->a = sfp[1].a;
//	d->cid = KMPI_FARRAY;
//	RETURN_(new_ReturnCppObject(_ctx, sfp, WRAP(d) K_RIXPARAM));
//}

//## Bytes MPIData.toBytes()
static KMETHOD MPIData_toBytes(CTX, ksfp_t *sfp _RIX)
{
	RETURN_(toRawPtr(kMPIData *, sfp[0].o)->b);
}

//## void MPIData.setOffset(int offset)
static KMETHOD MPIData_setOffset(CTX, ksfp_t *sfp _RIX)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].o);
	d->offset = sfp[1].ivalue;
	RETURNvoid_();
}

//## int MPIData.getOffset()
static KMETHOD MPIData_getOffset(CTX, ksfp_t *sfp _RIX)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].o);
	RETURNi_(d->offset);
}

//## int MPIData.getSize()
static KMETHOD MPIData_getSize(CTX, ksfp_t *sfp _RIX)
{
	kMPIData *d = toRawPtr(kMPIData *, sfp[0].o);
	RETURNi_(d->size - d->offset);
}

/* ------------------------------------------------------------------------ */
//## MPIData MPIData.newFloatArray(int size)
static KMETHOD MPIData_newFloatArray(CTX, ksfp_t *sfp _RIX)
{
	int size = sfp[1].ivalue;
	kMPIData *d = newMPIData(_ctx);
	d->type = MPI_DOUBLE;
	d->size = size;
	d->offset = 0;
	d->fa = KCALLOC(sizeof(kfloat_t), size);
	d->cid = KMPI_FARRAY;
	RETURN_(new_ReturnCppObject(_ctx, sfp, WRAP(d) K_RIXPARAM));
}

//## MPIData MPIData.newIntArray(int size)
static KMETHOD MPIData_newIntArray(CTX, ksfp_t *sfp _RIX)
{
	int size = sfp[1].ivalue;
	kMPIData *d = newMPIData(_ctx);
	d->type = MPI_LONG;
	d->size = size;
	d->offset = 0;
	d->fa = KCALLOC(sizeof(kint_t), size);
	d->cid = KMPI_IARRAY;
	RETURN_(new_ReturnCppObject(_ctx, sfp, WRAP(d) K_RIXPARAM));
}

//## float MPIData.getf(int n)
static KMETHOD MPIData_getf(CTX, ksfp_t *sfp _RIX)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].o);
	int n = sfp[1].ivalue + sdata->offset;
	RETURNf_(sdata->fa[n]);
}

//## void MPIData.setf(int n, float v)
static KMETHOD MPIData_setf(CTX, ksfp_t *sfp _RIX)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].o);
	int n = sfp[1].ivalue + sdata->offset;
	double v = sfp[2].fvalue;
	sdata->fa[n] = v;
	RETURNvoid_();
}

//## int MPIData.geti(int n)
static KMETHOD MPIData_geti(CTX, ksfp_t *sfp _RIX)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].o);
	int n = sfp[1].ivalue + sdata->offset;
	RETURNi_(sdata->ia[n]);
}

//## void MPIData.seti(int n, int v)
static KMETHOD MPIData_seti(CTX, ksfp_t *sfp _RIX)
{
	kMPIData *sdata = toRawPtr(kMPIData *, sfp[0].o);
	int n = sfp[1].ivalue + sdata->offset;
	kint_t v = sfp[2].ivalue;
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
	kmodshare_t h;
	kclass_t *cValue;
} kmodmpi_t;

static void kmodmpi_setup(CTX, struct kmodshare_t *def, int newctx)
{
	(void)_ctx;(void)def;(void)newctx;
}

static void kmodmpi_reftrace(CTX, struct kmodshare_t *baseh)
{
	(void)_ctx;(void)baseh;
}

static void kmodmpi_free(CTX, struct kmodshare_t *baseh)
{
	MPI_Finalize();
}

#define MOD_mpi 19/*TODO*/

static kbool_t mpi_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	kmodmpi_t *base = (kmodmpi_t*)KCALLOC(sizeof(kmodmpi_t), 1);
	base->h.name     = "mpi";
	base->h.setup    = kmodmpi_setup;
	base->h.reftrace = kmodmpi_reftrace;
	base->h.free     = kmodmpi_free;
	Konoha_setModule(MOD_mpi, &base->h, pline);

	MPI_Init(&argc, (char ***)&args);
	g_comm_world = (kMPIComm *)KMALLOC(sizeof(kMPIComm));
	g_comm_world->comm = MPI_COMM_WORLD;
	static KDEFINE_CLASS MPIDef = {
		"MPI"/*structname*/,
		CLASS_newid/*cid*/,  0/*cflag*/,
		0/*bcid*/, 0/*supcid*/, 0/*cstruct_size*/,
		0/*fields*/, 0/*fsize*/, 0/*fallocsize*/,
		0/*packid*/, 0/*packdom*/,
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
		CLASS_newid/*cid*/,  0/*cflag*/,
		0/*bcid*/, 0/*supcid*/, 0/*cstruct_size*/,
		0/*fields*/, 0/*fsize*/, 0/*fallocsize*/,
		0/*packid*/, 0/*packdom*/,
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
		CLASS_newid/*cid*/,  0/*cflag*/,
		0/*bcid*/, 0/*supcid*/, 0/*cstruct_size*/,
		0/*fields*/, 0/*fsize*/, 0/*fallocsize*/,
		0/*packid*/, 0/*packdom*/,
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
		CLASS_newid/*cid*/,  0/*cflag*/,
		0/*bcid*/, 0/*supcid*/, 0/*cstruct_size*/,
		0/*fields*/, 0/*fsize*/, 0/*fallocsize*/,
		0/*packid*/, 0/*packdom*/,
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
	//	CLASS_newid/*cid*/,  0/*cflag*/,
	//	0/*bcid*/, 0/*supcid*/, 0/*cstruct_size*/,
	//	0/*fields*/, 0/*fsize*/, 0/*fallocsize*/,
	//	0/*packid*/, 0/*packdom*/,
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
	kclass_t *CT_MPI = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &MPIDef, pline);
	kclass_t *CT_MPIComm = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &MPICommDef, pline);
	kclass_t *CT_MPIRequest = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &MPIRequestDef, pline);
	kclass_t *CT_MPIData = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &MPIDataDef, pline);
	//kclass_t *CT_MPIOp = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &MPIOpDef, pline);
#define TY_MPI         (CT_MPI->cid)
#define TY_MPIComm     (CT_MPIComm->cid)
#define TY_MPIRequest  (CT_MPIRequest->cid)
#define TY_MPIData     (CT_MPIData->cid)
#define TY_MPIOp       TY_Int //(CT_MPIOp->cid) // TODO
	KDEFINE_METHOD MethodData[] = {
		/* class MPI */
		_Public|_Static, _F(MPI_getWtime), TY_Float, TY_MPI, MN_("getWtime"), 0,

		/* class MPIComm */
		_Public|_Static, _F(MPIComm_getWorld), TY_MPIComm, TY_MPIComm, MN_("getWorld"), 0,
		_Public, _F(MPIComm_getRank), TY_Int, TY_MPIComm, MN_("getRank"), 0,
		_Public, _F(MPIComm_getSize), TY_Int, TY_MPIComm, MN_("getSize"), 0,
		_Public, _F(MPIComm_barrier), TY_Int, TY_MPIComm, MN_("barrier"), 0,
		_Public, _F(MPIComm_send), TY_Boolean, TY_MPIComm, MN_("send"), 4,
			TY_MPIData, FN_("buf"), TY_Int, FN_("count"), TY_Int, FN_("dest"), TY_Int, FN_("tag"),
		_Public, _F(MPIComm_recv), TY_Boolean, TY_MPIComm, MN_("recv"), 4,
			TY_MPIData, FN_("buf"), TY_Int, FN_("count"), TY_Int, FN_("src"), TY_Int, FN_("tag"),
		_Public, _F(MPIComm_iSend), TY_MPIRequest, TY_MPIComm, MN_("iSend"), 4,
			TY_MPIData, FN_("buf"), TY_Int, FN_("count"), TY_Int, FN_("dest"), TY_Int, FN_("tag"),
		_Public, _F(MPIComm_iRecv), TY_MPIRequest, TY_MPIComm, MN_("iRecv"), 4,
			TY_MPIData, FN_("buf"), TY_Int, FN_("count"), TY_Int, FN_("src"), TY_Int, FN_("tag"),
		_Public, _F(MPIComm_bcast), TY_Boolean, TY_MPIComm, MN_("bcast"), 3,
			TY_MPIData, FN_("sdata"), TY_Int, FN_("count"), TY_Int, FN_("root"),
		_Public, _F(MPIComm_scatter), TY_Boolean, TY_MPIComm, MN_("scatter"), 5,
			TY_MPIData, FN_("sdata"), TY_Int, FN_("scount"), TY_MPIData, FN_("rdata"), TY_Int, FN_("rcount"), TY_Int, FN_("root"),
		_Public, _F(MPIComm_gather), TY_Boolean, TY_MPIComm, MN_("gather"), 5,
			TY_MPIData, FN_("sdata"), TY_Int, FN_("scount"), TY_MPIData, FN_("rdata"), TY_Int, FN_("rcount"), TY_Int, FN_("root"),
		_Public, _F(MPIComm_allGather), TY_Boolean, TY_MPIComm, MN_("allGather"), 4,
			TY_MPIData, FN_("sdata"), TY_Int, FN_("scount"), TY_MPIData, FN_("rdata"), TY_Int, FN_("rcount"),
		_Public, _F(MPIComm_allToAll), TY_Boolean, TY_MPIComm, MN_("allToAll"), 4,
			TY_MPIData, FN_("sdata"), TY_Int, FN_("scount"), TY_MPIData, FN_("rdata"), TY_Int, FN_("rcount"),
		_Public, _F(MPIComm_reduce), TY_Boolean, TY_MPIComm, MN_("reduce"), 5,
			TY_MPIData, FN_("sdata"), TY_MPIData, FN_("rdata"), TY_Int, FN_("rcount"), TY_MPIOp, FN_("op"), TY_Int, FN_("root"),
		_Public, _F(MPIComm_allReduce), TY_Boolean, TY_MPIComm, MN_("allReduce"), 4,
			TY_MPIData, FN_("sdata"), TY_MPIData, FN_("rdata"), TY_Int, FN_("rcount"), TY_MPIOp, FN_("op"),
		_Public, _F(MPIComm_scan), TY_Boolean, TY_MPIComm, MN_("scan"), 4,
			TY_MPIData, FN_("sdata"), TY_MPIData, FN_("rdata"), TY_Int, FN_("rcount"), TY_MPIOp, FN_("op"),

		/* class MPIRequest */
		_Public, _F(MPIRequest_wait), TY_Boolean, TY_MPIRequest, MN_("wait"), 0,
		_Public, _F(MPIRequest_test), TY_Boolean, TY_MPIRequest, MN_("test"), 0,
		_Public, _F(MPIRequest_cancel), TY_Boolean, TY_MPIRequest, MN_("cancel"), 0,

		/* class MPIData */
		_Public|_Static, _F(MPIData_fromBytes), TY_MPIData, TY_MPIData, MN_("fromBytes"), 1, TY_Bytes, FN_("b"),
		//_Public|_Static, _F(MPIData_fromIntArray), TY_MPIData, TY_MPIData, MN_("fromIntArray"), 1, TY_Array, FN_("b"),
		//_Public|_Static, _F(MPIData_fromFloatArray), TY_MPIData, TY_MPIData, MN_("fromFloatArray"), 1, TY_Array, FN_("b"),
		_Public|_Static, _F(MPIData_newFloatArray), TY_MPIData, TY_MPIData, MN_("newFloatArray"), 1, TY_Int, FN_("n"),
		_Public|_Static, _F(MPIData_newIntArray), TY_MPIData, TY_MPIData, MN_("newIntArray"), 1, TY_Int, FN_("n"),
		_Public, _F(MPIData_toBytes), TY_Bytes, TY_MPIData, MN_("toBytes"), 0,
		//_Public, _F(MPIData_toIntArray), TY_Array, TY_MPIData, MN_("toIntArray"), 0,
		//_Public, _F(MPIData_toFloatArray), TY_Array, TY_MPIData, MN_("toFloatArray"), 0,
		_Public, _F(MPIData_getf), TY_Float, TY_MPIData, MN_("getf"), 1, TY_Int, FN_("n"),
		_Public, _F(MPIData_setf), TY_void, TY_MPIData, MN_("setf"), 2, TY_Int, FN_("n"), TY_Float, FN_("v"),
		_Public, _F(MPIData_geti), TY_Int, TY_MPIData, MN_("geti"), 1, TY_Int, FN_("n"),
		_Public, _F(MPIData_seti), TY_void, TY_MPIData, MN_("seti"), 2, TY_Int, FN_("n"), TY_Int, FN_("v"),
		_Public, _F(MPIData_setOffset), TY_void, TY_MPIData, MN_("setOffset"), 1, TY_Int, FN_("offset"),
		_Public, _F(MPIData_getOffset), TY_Int, TY_MPIData, MN_("getOffset"), 0,
		_Public, _F(MPIData_getSize), TY_Int, TY_MPIData, MN_("getSize"), 0, 
		DEND,
	};
	kNameSpace_loadMethodData(NULL, MethodData);
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
	kNameSpace_loadConstData(ks, OpData, pline);
	return true;
}

static kbool_t mpi_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t mpi_initNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t mpi_setupLingo(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* mpi_init(void)
{
	static const KDEFINE_PACKAGE d = {
		KPACKNAME("mpi", "1.0"),
		.initPackage  = mpi_initPackage,
		.setupPackage = mpi_setupPackage,
		.initNameSpace = mpi_initNameSpace,
		.setupPackage = mpi_setupLingo,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif

