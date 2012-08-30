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

#include <stdbool.h>
#include "dse.h"

Scheduler *Scheduler_new(void)
{
	Scheduler *sched = dse_malloc(sizeof(Scheduler));
	sched->front = 0;
	sched->last = 0;
	pthread_mutex_init(&sched->lock, NULL);
	pthread_cond_init(&sched->cond, NULL);
	return sched;
}

void Scheduler_delete(Scheduler *sched)
{
	pthread_mutex_destroy(&sched->lock);
	pthread_cond_destroy(&sched->cond);
	dse_free(sched, sizeof(Scheduler));
}

bool dse_enqueue(Scheduler *sched, Message *msg)
{
	int front = sched->front;
	int last = sched->last;
	if(next(front) == last) return false;
	sched->msgQueue[front] = msg;
	sched->front = next(front);
	return true;
}

Message *dse_dequeue(Scheduler *sched)
{
	int front = sched->front;
	int last = sched->last;
	if(front == last) return NULL;
	Message *msg = sched->msgQueue[last];
	sched->last = next(last);
	return msg;
}
