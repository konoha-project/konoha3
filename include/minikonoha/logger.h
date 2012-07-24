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

#ifndef EVIDENCE_H_
#define EVIDENCE_H_
#ifndef MINIOKNOHA_H_
#error Do not include logger.h without minikonoha.h.
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LOGPOL_RECORD       (1<<0) /* logger works only with this flag */

/* CRIT, ERR */
#define _SystemFault        (1<<1)  /* operating system, program runtime */
#define _FileSystemFault    (1<<1)  /* file system fault */
#define _ScriptFault        (1<<2)  /* developers' fault */
#define _DeveloperFault     (1<<2)  /* developers' fault */
#define _DataFault          (1<<3)  /* data fault */
#define _UserInputFault     (1<<3)  /* users' input fault */
#define _ExternalFault      (1<<4)  /* explicit connected network service */

/* WARN, NOTICE, INFO */
#define _Prediction         (1<<5)  /* prediction WARN */
#define _PreAction          (1<<6)  /* preaction */
#define _ChangeEnv          (1<<7)  /* change environment */
#define _SecurityAudit      (1<<8)  /* security auditing */
#define _PrivacyCaution     (1<<9)  /* including privacy information */

/* DEBUG, TRACE */
#define LOGPOL_DEBUG        (1<<10)  /* debug information (interanal state) */
#define LOGPOL_TRACE        (1<<11)  /* more detailed debug information */

#define LOGPOL_INIT         (1<<12) /* DONT USE THIS */
#define LOGPOL_CFUNC        (1<<13) /* DONT USE THIS */

typedef struct klogconf_t {
	int policy;
	void *ptr; // for precompiled formattings
	union {
		const char *func;
		kMethod *mtd;
	};
} klogconf_t ;

#define LOG_END 0
#define LOG_s   1
#define LOG_u   2

#define KEYVALUE_u(K,V)    LOG_u, (K), ((uintptr_t)V)
#define KEYVALUE_s(K,V)    LOG_s, (K), (V)
#define KEYVALUE_p(K,V)    LOG_u, (K), (V)

#define LOG_ScriptFault          KEYVALUE_u("uline", sfp[K_RTNIDX].uline)

#define ktrace(POLICY, ...)    do {\
		static klogconf_t _logconf = {LOGPOL_RECORD|LOGPOL_INIT|LOGPOL_CFUNC|POLICY, NULL, {__FUNCTION__}};\
		if(TFLAG_is(int, _logconf.policy, LOGPOL_RECORD)) {\
			(KPI)->Ktrace(kctx, &_logconf, ## __VA_ARGS__, LOG_END);\
		}\
	}while(0)\

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* EVIDENCE_H_ */
