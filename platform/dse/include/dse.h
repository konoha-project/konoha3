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

#ifndef DSE_H_
#define DSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <event.h>
#include <evhttp.h>
#include <event2/buffer.h>
#define K_USE_PTHREAD 1
#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
extern int verbose_debug;
#include <minikonoha/platform_posix.h>
#ifndef K_USE_PTHREAD
#include <pthread.h>
#endif /* K_USE_PTHREAD */

extern char *logpoolip;

// for debug
//#define DSE_DEBUG 1
#if defined(DSE_DEBUG)
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define DEBUG_ASSERT(stmt) assert(stmt);
#else
#define DEBUG_PRINT(fmt, ...)
#define DEBUG_ASSERT(stmt)
#endif

// for memory management
extern size_t totalMalloc;

void *dse_malloc(size_t size);
void  dse_free(void *ptr, size_t size);

/* Message */
typedef unsigned char Message;

Message *Message_new(unsigned char *requestLine);
void     Message_delete(Message *msg);

/* Scheduler */
#define MSG_QUEUE_SIZE 16
#define next(index) (((index) + 1) % MSG_QUEUE_SIZE)

struct Scheduler {
	Message        *msgQueue[MSG_QUEUE_SIZE];
	int             front;
	int             last;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
};
typedef struct Scheduler Scheduler;

Scheduler   *Scheduler_new(void);
void         Scheduler_delete(Scheduler *sched);
bool         dse_enqueue(Scheduler *sched, Message *msg);
Message     *dse_dequeue(Scheduler *sched);

/* DSE */
struct DSE {
	struct event_base   *base;
	struct evhttp       *httpd;
	Scheduler           *sched;
};
typedef struct DSE DSE;

DSE *DSE_new(void);
void DSE_start(DSE *dse, const char *addr, int ip);
void DSE_delete(DSE *dse);

#endif /* DSE_H_ */
