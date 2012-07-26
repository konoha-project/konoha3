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

/* ************************************************************************ */

#ifndef DSE_PROTOCOL_H_
#define DSE_PROTOCOL_H_

struct dReq {
	struct evhttp_request *req;
	int method;
	int context;
	int taskid;
	char logpoolip[16];
	char *scriptfilepath;
};

struct dRes {
	int taskid;
	int status;
	char *status_symbol;
	char *status_detail;
};

enum {
	E_METHOD_EVAL,
	E_METHOD_TYCHECK,
	E_STATUS_OK,
};

#define DSE_FILEPATH_SIZE 128

static struct dReq *newDReq()
{
	struct dReq *ret = (struct dReq *)malloc(sizeof(struct dReq));
	ret->method = 0;
	ret->context = 0;
	ret->taskid = 0;
	memset(ret->logpoolip, 16, 0);
	ret->scriptfilepath = (char*)dse_malloc(DSE_FILEPATH_SIZE);
	memset(ret->scriptfilepath, 128, 0);
	return ret;
}

static void deleteDReq(struct dReq *req)
{
	if (req == NULL) return;
	dse_free(req->scriptfilepath, DSE_FILEPATH_SIZE);
	dse_free(req, sizeof(struct dReq));
}

static struct dRes *newDRes()
{
	struct dRes *ret = (struct dRes *)dse_malloc(sizeof(struct dRes));
	ret->taskid = 0;
	ret->status = 0;
	ret->status_detail = NULL;
	ret->status_symbol = NULL;
	return ret;
}

static void deleteDRes (struct dRes *res)
{
	// check if satus_* is set
	if (res == NULL) return;
	dse_free(res, sizeof(struct dRes));
}

#endif /* DSE_PROTOCOL_H_ */
