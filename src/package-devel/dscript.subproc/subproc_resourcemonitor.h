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
#ifndef SUBPROC_RESOURCEMONITOR_H_
#define SUBPROC_RESOURCEMONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define SUBPROC_ENABLE_RESOURCEMONITOR

// for resource monitoring

#if defined(SUBPROC_ENABLE_RESOURCEMONITOR)
enum e_resource{
	R_MEMORY,
	R_CPU,
	R_CONTEXTSWITCH,
};

#define SLEEP_NSEC 10000

#define SUBPROC_RESOURCEMON_INSTANCE subproc_resource_mon_t rmon

#define INIT_RESOURCE_MONITOR(spd) init_resourcemonitor(kctx, &(spd->rmon))
#define SETUP_RESOURCE_MONITOR(spd) setup_resourcemonitor(kctx, &(spd->rmon))
#define CLEANUP_RESOURCE_MONITOR(spd) cleanup_resourcemonitor(kctx, &(spd->rmon))
#define SETUP_RESOURCE_MONITOR_FOR_CHILD(spd) setup_resourcemonitor_for_chlid(kctx, &(spd->rmon))
#define ATTACH_RESOURCE_MONITOR_FOR_CHILD(spd, pid) attach_resourcemonitor_for_child(kctx, &(spd->rmon), pid)

#define FETCH_MEM_FROM_RESOURCE_MONITOR(spd) fetch_resourcemonitor_about(kctx, &(spd->rmon), R_MEMORY)

#if defined(__APPLE__)
#include "subproc_resourcemonitor_mac.h"
#elif defined(__linux__) || defined(__NetBSD__)
#include "subproc_resourcemonitor_linux.h"
#else
#warning TODO: for example, BSD
#endif

#else
#define SUBPROC_RESOURCEMON_INSTANCE
#define INIT_RESOURCE_MONITOR(spd)
#define SETUP_RESOURCE_MONITOR(spd)
#define CLEANUP_RESOURCE_MONITOR(spd)
#define ATTACH_RESOURCE_MONITOR_FOR_CHILD(spd, pid)
#define RECV_RESOURCE_MONITOR_FROM_CHILD(spd)

#define FETCH_MEM_FROM_RESOURCE_MONITOR(spd) 0
#undef SUBPROC_ENABLE_RESOURCEMONITOR
#endif /* SUBPROC_RESOURCE_MONITOR */



#ifdef __cplusplus
}
#endif
#endif /* SUBPROC_RESOURCEMONITOR_H_ */
