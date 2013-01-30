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

/* ************************************************************************ */

#ifndef SUBPROC_RESOURCEMONITOR_LINUX_H_
#define SUBPROC_RESOURCEMONITOR_LINUX_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct subproc_resource_mon_t {
	FILE *procfs;
} subproc_resource_mon_t;


static void init_resourcemonitor(KonohaContext *kctx, subproc_resource_mon_t *mon)
{
	mon->procfs = NULL;
}

static int setup_resourcemonitor(KonohaContext *kctx, subproc_resource_mon_t *mon)
{
	// DO NOTHING;
	return 0;
}

static int cleanup_resourcemonitor(KonohaContext *kctx, subproc_resource_mon_t *mon)
{
	if(mon->procfs != NULL) {
		fclose(mon->procfs);
	}
	return 0;
}

static int setup_resourcemonitor_for_chlid(KonohaContext *kctx, subproc_resource_mon_t *mon)
{
	return 0;
}

static int attach_resourcemonitor_for_child(KonohaContext *kctx, subproc_resource_mon_t *mon, int pid)
{
	char statm[32];
	sprintf(&statm[0], "/proc/%d/statm", pid);
	mon->procfs = fopen(statm, "r");
	if(mon->procfs == NULL) {
		DBG_P("cannot open proc filesystem:%s\n", statm);
	}
	return 0;
}

//static int fetch_resourcemonitor_about(KonohaContext *kctx, subproc_resource_mon_t *mon, enum e_resource res)
//{
//	int mem = 0;
//	size_t dummy, vm;
//	switch(res) {
//	case R_MEMORY:
//		do {
//			vm = 0;
//			fscanf(mon->procfs, "%ld %ld ", &dummy, &vm); // get resident
//			if(mem < vm) mem = vm;
//			if(!vm) break;
//		} while(!usleep(SLEEP_NSEC));
//		mem *= getpagesize();
//	default:
//		break;
//	}
//	return mem;
//}

#ifdef __cplusplus
}
#endif
#endif /* SUBPROC_RESOURCEMONITOR_LINUX_H_ */
