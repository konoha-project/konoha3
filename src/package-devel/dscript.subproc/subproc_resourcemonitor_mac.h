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

#ifndef SUBPROC_RESOURCEMONITOR_MAC_H_
#define SUBPROC_RESOURCEMONITOR_MAC_H_

// structure definiton
#include <mach/mach.h>
typedef struct mach_send_port_msg {
	mach_msg_header_t          header;
	mach_msg_body_t            body;
	mach_msg_port_descriptor_t task_port;
}mach_send_port_msg;

typedef struct mach_recv_port_msg {
	mach_msg_header_t header;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t task_port;
	mach_msg_trailer_t trailer;
} mach_recv_port_msg;

typedef struct subproc_resource_mon_t {
	mach_port_t parent_recv_port;
	mach_port_t child_recv_port;
	task_t     task;
} subproc_resource_mon_t;

#define setup_recv_port(port) k_Setup_recv_port(kctx, port)
#define send_port(p1, p2) k_send_port (kctx, p1, p2)
#define recv_port(p1, p2) k_recv_port (kctx, p1, p2)

static int k_Setup_recv_port (KonohaContext *kctx, mach_port_t *recv_port) {
	kern_return_t err;
	mach_port_t port = MACH_PORT_NULL;
	err = mach_port_allocate (mach_task_self (), MACH_PORT_RIGHT_RECEIVE, &port);
	// TODO: error handling
	err = mach_port_insert_right (mach_task_self (), port, port, MACH_MSG_TYPE_MAKE_SEND);
	//TODO: error handling
	*recv_port = port;
	return 0;
}

static int k_send_port (KonohaContext *kctx, mach_port_t remote_port, mach_port_t port) {
	mach_send_port_msg msg;
	kern_return_t err;
	msg.header.msgh_remote_port = remote_port;
	msg.header.msgh_local_port  = MACH_PORT_NULL;
	msg.header.msgh_bits        = MACH_MSGH_BITS (MACH_MSG_TYPE_COPY_SEND, 0) |
			MACH_MSGH_BITS_COMPLEX;
	msg.header.msgh_size        = sizeof (msg);

	msg.body.msgh_descriptor_count = 1;
	msg.task_port.name             = port;
	msg.task_port.disposition      = MACH_MSG_TYPE_COPY_SEND;
	msg.task_port.type             = MACH_MSG_PORT_DESCRIPTOR;

	err = mach_msg_send (&msg.header);
	if(err != KERN_SUCCESS) {
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
				LogText("@", "mach_msg_send"),
				LogText("msg", "mach msg failed")
		);
	}
	return 0;
}

static int k_recv_port (KonohaContext *kctx, mach_port_t recv_port, mach_port_t *port) {
	mach_recv_port_msg msg;
	kern_return_t err;
	err = mach_msg (&msg.header, MACH_RCV_MSG, 0, sizeof (msg), recv_port,
					MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	if(err != KERN_SUCCESS){
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
				LogText("@", "mach_msg"),
				LogText("msg", "recv port failed")
		);
	}
	*port = msg.task_port.name;
	return 0;
}


static void init_resourcemonitor(KonohaContext *kctx, subproc_resource_mon_t *mon) {
	mon->parent_recv_port = MACH_PORT_NULL;
	mon->child_recv_port = MACH_PORT_NULL;
	mon->task = MACH_PORT_NULL;
}

static int setup_resourcemonitor (KonohaContext *kctx, subproc_resource_mon_t *mon) {
	kern_return_t err;
	if(setup_recv_port(&(mon->parent_recv_port)) != 0) return -1;
	err = task_Set_bootstrap_port(mach_task_self(), mon->parent_recv_port);
	if(err != KERN_SUCCESS) {
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
				LogText("@", "failed to setup resourcemonitor")
		);
	}
	return err;
}

static int cleanup_resourcemonitor(KonohaContext *kctx, subproc_resource_mon_t *mon) {
	if(KERN_SUCCESS != mach_port_deallocate (mach_task_self(), mon->parent_recv_port)) {
				OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
						LogText("@", "dup2"),
						LogUint("errno", errno),
						LogText("errstr", strerror(errno))
				);
				return -1;
	}
	return 0;
}

static int setup_resourcemonitor_for_chlid(KonohaContext *kctx, subproc_resource_mon_t *mon) {

	kern_return_t err = task_get_bootstrap_port(mach_task_self(), &(mon->parent_recv_port));
	if(setup_recv_port(&(mon->child_recv_port)) != 0) return -1;
	if(send_port (mon->parent_recv_port, mach_task_self()) != 0) return -1;
	if(send_port(mon->parent_recv_port, mon->child_recv_port) != 0) return -1;
	if(recv_port(mon->child_recv_port, &bootstrap_port) != 0) return -1;
	err = task_Set_bootstrap_port(mach_task_self(), bootstrap_port);
	return err;
}


static int attach_resourcemonitor_for_child(KonohaContext *kctx, subproc_resource_mon_t *mon, int pid) {
	kern_return_t err = task_Set_bootstrap_port(mach_task_self(), bootstrap_port);
	if(recv_port(mon->parent_recv_port, &(mon->task)) != 0) return -1;
	if(recv_port(mon->parent_recv_port, &(mon->child_recv_port)) != 0) return -1;
	if(send_port(mon->child_recv_port, bootstrap_port) != 0) return -1;
	return err;
}

//static int fetch_resourcemonitor_about(KonohaContext *kctx, subproc_resource_mon_t *mon, enum e_resource res) {
//	mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
//	struct task_basic_info t_info;
//	int mem = 0;
//	switch(res) {
//	case R_MEMORY:
//		do {
//			if(KERN_SUCCESS != task_info(mon->task, TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count))
//					break;
//			if(mem < t_info.resident_size) mem = t_info.resident_size;
//		}while(!usleep(SLEEP_NSEC));
//		return mem;
//	default:
//		return 0;
//	}
//}

#endif /* SUBPROC_RESOURCEMONITOR_MAC_H_ */
