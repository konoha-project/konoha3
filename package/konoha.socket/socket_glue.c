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

// **************************************************************************
// LIST OF CONTRIBUTERS
//  shinpei - Shinpei Nakata, Yokohama National University, Japan
//  kimio - Kimio Kuramitsu, Yokohama National University, Japan
//  goccy - Masaaki Goshima, Yokohama National University, Japan
// **************************************************************************

#include<konoha2/konoha2.h>
#include<konoha2/sugar.h>
#include<konoha2/bytes.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <signal.h>
#include "socket_glue.h"

#ifdef __cplusplus
extern "C" {
#endif


KDEFINE_PACKAGE* socket_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("socket", "1.0"),
		.initPackage = socket_initPackage,
		.setupPackage = socket_setupPackage,
		.initNameSpace = socket_initNameSpace,
		.setupNameSpace = socket_setupNameSpace,
	};
	return &d;
}

#undef new_T

#ifdef __cplusplus
}
#endif

