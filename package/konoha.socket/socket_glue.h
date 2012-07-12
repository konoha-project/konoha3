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

#ifndef SOCKET_GLUE_H_
#define SOCKET_GLUE_H_

#define WORD2INT(val) (sizeof(val)==8) ? (val&0x7FFFFFFF)|((val>>32)&0x80000000) : val

/*
 #define KNH_TODO(msg) do {\
	fprintf(stderr, "TODO(%s) : %s at %s:%d",\
			msg, __FUNCTION__, __FILE__, __LINE__);\
	abort();\
} while (0)
*/
#define KNH_NTRACE2(...) KNH_TODO("ntrace")

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct _kSockAddr kSockAddr;
struct _kSockAddr {
	kObjectHeader h;
	struct sockaddr_in *sockaddr_in;
};

/* ------------------------------------------------------------------------ */
#define ADDRBUFSIZE 256

static void SockAddr_init(CTX, kObject *o, void *conf)
{
	struct _kSockAddr *sa = (struct _kSockAddr*)o;
	sa->sockaddr_in = (struct sockaddr_in *)KCALLOC(sizeof(struct sockaddr_in), 1);
}

static void SockAddr_free(CTX, kObject *o)
{
	struct _kSockAddr *sa = (struct _kSockAddr*)o;
	if (sa->sockaddr_in != NULL) {
		KFREE(sa->sockaddr_in, sizeof(struct sockaddr_in));
		sa->sockaddr_in = NULL;
	}
}

/* ======================================================================== */
// [private functions]

// <String, int, int> => sockaddr_in*
void toSockaddr(struct sockaddr_in *addr, char *ip, const int port, const int family)
{
	memset(addr, 0, sizeof(*addr));
	addr->sin_addr.s_addr = (*ip==0) ? 0 : inet_addr(ip);
	addr->sin_port        = htons(port);
	addr->sin_family      = family;
}

// sockaddr_in* => Map
//void fromSockaddr(CTX, struct kMap* info, struct sockaddr_in addr)
//{
//	if (info != NULL ) {
//		knh_DataMap_setString(_ctx, info, "addr", inet_ntoa(addr.sin_addr));
//		knh_DataMap_setInt(_ctx, info, "port", ntohs(addr.sin_port));
//		knh_DataMap_setInt(_ctx, info, "family", addr.sin_family);
//	}
//}

// for select :: kArray* => fd_set*
static fd_set* toFd(fd_set* s, kArray *a )
{
	if(s == NULL || kArray_size(a) <= 0) {
		return NULL;
	}
	FD_ZERO(s);
	int indx;
	int fd;
	for(indx = 0; indx < kArray_size(a); indx++ ) {
		fd = WORD2INT(a->ilist[indx]);
		if((fd >= 0) && (fd < FD_SETSIZE)) {
			FD_SET(fd, s);
		}
	}
	return s;
}

// for select :: fd_set* => kArray*
static void fromFd(CTX, fd_set* s, kArray *a )
{
	if(s != NULL && kArray_size(a) > 0 ) {
		int indx;
		for(indx = 0; indx < kArray_size(a) ; indx++ ) {
			if(!FD_ISSET(WORD2INT(a->ilist[indx]), s) ) {
//				kArray_remove(a, indx);
			}
		}
	}
}

// for select
static int getArrayMax(kArray *a)
{
	int ret = -1;
	if(kArray_size(a) > 0) {
		int cnt;
		int fd;
		for(cnt = 0; cnt < kArray_size(a); cnt++) {
			if((fd = WORD2INT(a->ilist[cnt])) > ret) {
				ret = fd;
			}
		}
	}
	return ret;
}

// for select
static int getNfd(kArray *a1, kArray *a2, kArray *a3)
{
	int ret = -1;
	int tmp;

	if((tmp=getArrayMax(a1)) > ret) {
		ret = tmp;
	}
	if((tmp=getArrayMax(a2)) > ret) {
		ret = tmp;
	}
	if((tmp=getArrayMax(a3)) > ret) {
		ret = tmp;
	}
	return ret;
}

/* ======================================================================== */
// [KMETHODS]

//## int System.accept(int socket, Map remoteInfo);
//KMETHOD System_accept(CTX, ksfp_t* sfp _RIX)
//{
//	struct sockaddr_in addr;
//	int addrLen = sizeof(addr);
//	memset(&addr, 0, addrLen);
//
//	int ret = accept(
//			WORD2INT(sfp[1].ivalue),
//			(struct sockaddr*)&addr,
//			(socklen_t*)&addrLen
//	);
//	if(ret >= 0 ) {
//		 fromSockaddr(_ctx, sfp[2].m, addr);
//	} else {
//		ktrace(_SystemFault,
//				KEYVALUE_s("@", "accept"),
//				KEYVALUE_u("errno", errno),
//				KEYVALUE_s("errstr", strerror(errno))
//		);
//	}
//	RETURNi_(ret);
//}

//## int System.accept(int socket, SockAddr remoteInfo);
KMETHOD System_accept(CTX, ksfp_t* sfp _RIX)
{
	struct _kSockAddr *sa = (struct _kSockAddr*)sfp[2].o;
	struct sockaddr_in *addr = sa->sockaddr_in;
	int addrLen = sizeof(struct sockaddr_in);

	int ret = accept(
			WORD2INT(sfp[1].ivalue),
			(struct sockaddr *)addr,
			(socklen_t*)&addrLen
	);
	if(ret >= 0) {
//		fromSockaddr(_ctx, sa, addr);
	}
	else {
		ktrace(_SystemFault,
				KEYVALUE_s("@", "accept"),
				KEYVALUE_u("errno", errno),
				KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.bind(int socket, String srcIP, int srcPort, int family);
KMETHOD System_bind(CTX, ksfp_t* sfp _RIX)
{
	struct sockaddr_in addr;
	toSockaddr(&addr,
			(char*)sfp[2].s,
			WORD2INT(sfp[3].ivalue),
			WORD2INT(sfp[4].ivalue)
	);
	int ret = bind(WORD2INT(sfp[1].ivalue),
			(struct sockaddr*)&addr,
			sizeof(addr)
	);
	if(ret != 0) {
		ktrace(_SystemFault,
			KEYVALUE_s("@", "bind"),
			KEYVALUE_u("errno", errno),
			KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.close(int fd);
KMETHOD System_close(CTX, ksfp_t* sfp _RIX)
{
	int ret = close(WORD2INT(sfp[1].ivalue) );

	if(ret != 0 ) {
		ktrace(_SystemFault,
			KEYVALUE_s("@", "close"),
			KEYVALUE_u("errno", errno),
			KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.connect(int socket, String dstIP, int dstPort, int family);
KMETHOD System_connect(CTX, ksfp_t* sfp _RIX)
{
	struct sockaddr_in addr;
	toSockaddr(&addr,
				(char*)S_text(sfp[2].s),
				WORD2INT(sfp[3].ivalue),
				WORD2INT(sfp[4].ivalue)
	);

	int ret = connect(WORD2INT(sfp[1].ivalue),
			(struct sockaddr*)&addr,
			sizeof(addr)
	);
	if(ret != 0) {
		ktrace(_SystemFault,
			KEYVALUE_s("@", "connect"),
			KEYVALUE_u("errno", errno),
			KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.listen(int socket, int backlog);
KMETHOD System_listen(CTX, ksfp_t* sfp _RIX)
{
	int ret = listen(WORD2INT(sfp[1].ivalue), WORD2INT(sfp[2].ivalue));
	if(ret != 0) {
		ktrace(_SystemFault,
			KEYVALUE_s("@", "listen"),
			KEYVALUE_u("errno", errno),
			KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## String System.getsockname(int socket);
//KMETHOD System_getsockname(CTX, ksfp_t *sfp _RIX)
//{
//	struct sockaddr_in addr;
//	int addrLen = sizeof(addr);
//	memset(&addr, 0, addrLen);
//
//	kMap *ret_s = KNH_TNULL(Map);
//	if(getsockname(WORD2INT(sfp[1].ivalue),
//					   (struct sockaddr*)&addr,
//					   (socklen_t*)&addrLen ) == 0 ) {
//		ret_s = new_DataMap(ctx);
//		fromSockaddr(_ctx, ret_s, addr);
//	} else {
//		KNH_NTRACE2(_ctx, "konoha.socket.name ", K_PERROR, KNH_LDATA0);
//	}
//	RETURN_(ret_s);
//}

//## int System.getsockopt(int socket, int option);
KMETHOD System_getsockopt(CTX, ksfp_t* sfp _RIX)
{
	int val;
	int valLen = sizeof(val);

	int ret = getsockopt(
			WORD2INT(sfp[1].ivalue),
			SOL_SOCKET,
			(int)sfp[2].ivalue,
			&val,
			(socklen_t*)&valLen
	);
	if(ret == 0) {
		ret = val;
	}
	else {
		ktrace(_SystemFault,
			KEYVALUE_s("@", "getsockopt"),
			KEYVALUE_u("errno", errno),
			KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.setsockopt(int socket, int option, int value);
KMETHOD System_setsockopt(CTX, ksfp_t* sfp _RIX)
{
	int ret = setsockopt(
			WORD2INT(sfp[1].ivalue),
			SOL_SOCKET,
			(int)sfp[2].ivalue,
			&sfp[3].ivalue,
			sizeof(sfp[3].ivalue)
	);
	if(ret != 0) {
		ktrace(_SystemFault,
			KEYVALUE_s("@", "setsockopt"),
			KEYVALUE_u("errno", errno),
			KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## Map System.getpeername(int socket);
//KMETHOD System_getpeername(CTX, ksfp_t* sfp _RIX)
//{
//	struct sockaddr_in addr;
//	int addrLen = sizeof(addr);
//	memset(&addr, 0, addrLen);
//
//	kMap *ret_s = KNH_TNULL(Map);
//	if(getpeername(WORD2INT(sfp[1].ivalue),
//					   (struct sockaddr*)&addr,
//					   (socklen_t*)&addrLen ) == 0 ) {
//		ret_s = new_DataMap(ctx);
//		fromSockaddr(_ctx, ret_s, addr);
//	} else {
//		KNH_NTRACE2(_ctx, "konoha.socket.peername ", K_PERROR, KNH_LDATA0);
//	}
//
//	RETURN_(ret_s );
//}

//## int System.recv(int socket, byte[] buffer, int flags);
static KMETHOD System_recv(CTX, ksfp_t* sfp _RIX)
{
	kBytes *ba  = sfp[2].ba;
	int ret = recv(WORD2INT(sfp[1].ivalue),
					  ba->buf,
					  ba->bytesize,
					  (int)sfp[3].ivalue );
	if(ret < 0 ) {
		ktrace(_SystemFault,
				KEYVALUE_s("@", "recv"),
				KEYVALUE_s("perror", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.recvfrom(int socket, byte[] buffer, int flags, Map remoteInfo);
//static KMETHOD System_recvfrom(CTX, ksfp_t* sfp _RIX)
//{
//	struct sockaddr_in addr;
//	int addrLen = sizeof(addr);
//	memset(&addr, 0, addrLen);
//
//	kBytes *ba  = sfp[2].ba;
//	int ret = recvfrom(WORD2INT(sfp[1].ivalue),
//			  	  	  	   ba->buf,
//			  	  	  	   ba->bytesize,
//			  	  	  	   (int)sfp[3].ivalue,
//			  	  	  	   (struct sockaddr *)&addr,
//			  	  	  	   (socklen_t*)&addrLen );
//	if(ret >= 0 ) {
//		fromSockaddr(_ctx, sfp[4].m, addr);
//	} else {
//		KNH_NTRACE2(_ctx, "konoha.socket.recvfrom ", K_PERROR, KNH_LDATA0);
//	}
//	RETURNi_(ret);
//}

//## int System.select(int[] readsock, int[] writesock, int[] exceptsock, long timeoutSec, long timeoutUSec);
static KMETHOD System_select(CTX, ksfp_t* sfp _RIX)
{
	kArray *a1 = sfp[1].a;
	kArray *a2 = sfp[2].a;
	kArray *a3 = sfp[3].a;
	int nfd = getNfd(a1, a2, a3 );

	fd_set rfds, wfds, efds;
	fd_set *rfd = toFd(&rfds, a1 );
	fd_set *wfd = toFd(&wfds, a2 );
	fd_set *efd = toFd(&efds, a3 );

	struct timeval tv;
	tv.tv_sec  = (long)sfp[4].ivalue;
	tv.tv_usec = (long)sfp[5].ivalue;

	int ret = select(nfd+1, rfd, wfd, efd, &tv );
	if(ret > 0) {
		fromFd(_ctx, rfd, a1 );
		fromFd(_ctx, wfd, a2 );
		fromFd(_ctx, efd, a3 );
	}
	else {
		if(ret < 0 ) {
			ktrace(_SystemFault,
					KEYVALUE_s("@", "select"),
					KEYVALUE_s("perror", strerror(errno))
			);
		}
		// TODO::error or timeout is socket list all clear [pending]
		kArray_clear(a1, 0);
		kArray_clear(a2, 0);
		kArray_clear(a3, 0);
	}
	RETURNi_(ret);
}

//## int System.send(int socket, byte[] message, int flags);
static KMETHOD System_send(CTX, ksfp_t* sfp _RIX)
{
	kBytes *ba = sfp[2].ba;
	// Broken Pipe Signal Mask
#ifndef __APPLE__
	__sighandler_t oldset = signal(SIGPIPE, SIG_IGN);
	__sighandler_t ret_signal = SIG_ERR;
#else
	sig_t oldset = signal(SIGPIPE, SIG_IGN);
	sig_t ret_signal = SIG_ERR;
#endif
	if(oldset == SIG_ERR) {
		ktrace(_DataFault,
				KEYVALUE_s("@", "signal"),
				KEYVALUE_s("perror", strerror(errno))
		);
	}
	int ret = send(WORD2INT(sfp[1].ivalue),
					  ba->buf,
					  ba->bytesize,
					  (int)sfp[3].ivalue );
	if(ret < 0) {
		ktrace(_DataFault,
				KEYVALUE_s("@", "send"),
				KEYVALUE_s("perror", strerror(errno))
		);
	}
	if(oldset != SIG_ERR) {
		ret_signal = signal(SIGPIPE, oldset);
		if(ret_signal == SIG_ERR) {
			ktrace(_DataFault,
					KEYVALUE_s("@", "signal"),
					KEYVALUE_s("perror", strerror(errno))
			);
		}
	}
	RETURNi_(ret);
}

//## int System.sendto(int socket, Bytes message, int flags, String dstIP, int dstPort, int family);
static KMETHOD System_sendto(CTX, ksfp_t* sfp _RIX)
{
	kBytes *ba = sfp[2].ba;
	struct sockaddr_in addr;
	kString* s = sfp[4].s;
	toSockaddr(&addr, (char*)S_text(s), WORD2INT(sfp[5].ivalue), WORD2INT(sfp[6].ivalue));
	// Broken Pipe Signal Mask
#ifndef __APPLE__
	__sighandler_t oldset = signal(SIGPIPE, SIG_IGN);
	__sighandler_t ret_signal = SIG_ERR;
#else
	sig_t oldset = signal(SIGPIPE, SIG_IGN);
	sig_t ret_signal = SIG_ERR;
#endif
	int ret = sendto(
			WORD2INT(sfp[1].ivalue),
			ba->buf,
			ba->bytesize,
			(int)sfp[3].ivalue,
			(struct sockaddr *)&addr,
			sizeof(struct sockaddr)
	);
	if(ret < 0) {
		ktrace(_SystemFault,
				KEYVALUE_s("@", "sendto"),
				KEYVALUE_u("errno", errno),
				KEYVALUE_s("errstr", strerror(errno))
		);
	}
	if(oldset != SIG_ERR) {
		ret_signal = signal(SIGPIPE, oldset);
		if(ret_signal == SIG_ERR) {
			ktrace(_SystemFault,
				KEYVALUE_s("@", "signal"),
				KEYVALUE_u("errno", errno),
				KEYVALUE_s("errstr", strerror(errno))
			);
		}
	}
	RETURNi_(ret);
}

//## int System.shutdown(int socket, int how);
KMETHOD System_shutdown(CTX, ksfp_t* sfp _RIX)
{
	int ret = shutdown(WORD2INT(sfp[1].ivalue), WORD2INT(sfp[2].ivalue));
	if(ret != 0) {
		ktrace(_SystemFault,
			KEYVALUE_s("@", "shutdown"),
			KEYVALUE_u("errno", errno),
			KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.sockatmark(int socket);
KMETHOD System_sockatmark(CTX, ksfp_t* sfp _RIX)
{
	int ret = sockatmark(WORD2INT(sfp[1].ivalue));
	if(ret < 0) {
		ktrace(_SystemFault,
			KEYVALUE_s("@", "sockadmark"),
			KEYVALUE_u("errno", errno),
			KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.socket(int family, int type, int protocol);
KMETHOD System_socket(CTX, ksfp_t* sfp _RIX)
{
	int ret = socket(WORD2INT(sfp[1].ivalue),
					WORD2INT(sfp[2].ivalue),
					WORD2INT(sfp[3].ivalue));
	if(ret < 0) {
		ktrace(_SystemFault,
				KEYVALUE_s("@", "socket"),
				KEYVALUE_u("errno", errno),
				KEYVALUE_s("errstr", strerror(errno))
		);
	}
	RETURNi_(ret);
}

//## int System.socketpair(int family, int type, int protocol, int[] pairCSock);
static KMETHOD System_socketpair(CTX, ksfp_t* sfp _RIX)
{
	int ret = -2;
	kArray *a = sfp[4].a;
	if(kArray_size(a)) {
		int pairFd[2];
		if((ret = socketpair(WORD2INT(sfp[1].ivalue),
				WORD2INT(sfp[2].ivalue),
				WORD2INT(sfp[3].ivalue),
				pairFd)) == 0) {
			a->ilist[0] = pairFd[0];
			a->ilist[1] = pairFd[1];
		}
		else {
			ktrace(_SystemFault,
					KEYVALUE_s("@", "socketpair"),
					KEYVALUE_u("errno", errno),
					KEYVALUE_s("errstr", strerror(errno))
			);
		}
	}
	RETURNi_(ret);
}


// --------------------------------------------------------------------------

static KMETHOD SockAddr_new (CTX, ksfp_t *sfp _RIX)
{
	RETURN_(new_kObject(O_ct(sfp[K_RTNIDX].o), 0));
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define CT_SockAddr         cSockAddr
#define TY_SockAddr         cSockAddr->cid
#define IS_SockAddr(O)      ((O)->h.ct == CT_SockAddr)

static	kbool_t socket_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	KDEFINE_CLASS defSockAddr = {
		STRUCTNAME(SockAddr),
		.cflag = kClass_Final,
		.init = SockAddr_init,
		.free = SockAddr_free,
	};
	kclass_t *cSockAddr = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &defSockAddr, pline);
	kparam_t pi = {TY_Int, FN_("ivalue")};
	kclass_t *CT_IntArray = kClassTable_Generics(CT_Array, TY_Int, 1, &pi);
	kcid_t TY_IntArray = CT_IntArray->cid;

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(System_accept), TY_Int, TY_System, MN_("accept"), 2, TY_Int, FN_("fd"), TY_SockAddr, FN_("sockaddr"),
		_Public|_Const|_Im, _F(System_bind), TY_Int, TY_System, MN_("bind"), 4, TY_Int, FN_("fd"), TY_String, FN_("srcIP"), TY_Int, FN_("srcPort"), TY_Int, FN_("family"),
		_Public|_Const|_Im, _F(System_close), TY_Int, TY_System, MN_("close"), 1, TY_Int, FN_("fd"),
		_Public|_Const|_Im, _F(System_connect), TY_Int, TY_System, MN_("connect"), 4, TY_Int, FN_("fd"), TY_String, FN_("dstIP"), TY_Int, FN_("dstPort"), TY_Int, FN_("family"),
		_Public|_Const|_Im, _F(System_listen), TY_Int, TY_System, MN_("listen"), 2, TY_Int, FN_("fd"), TY_Int, FN_("backlog"),
//		_Public|_Const|_Im, _F(System_getsockname), TY_Map TY_System, MN_("getsockname"),1, TY_Int, FN_("fd"),
		_Public|_Const|_Im, _F(System_getsockopt), TY_Int, TY_System, MN_("getsockopt"), 2, TY_Int, FN_("fd"), TY_Int, FN_("opt"),
		_Public|_Const|_Im, _F(System_setsockopt), TY_Int, TY_System, MN_("setsockopt"), 3, TY_Int, FN_("fd"), TY_Int, FN_("opt"), TY_Int, FN_("value"),
//		_Public|_Const|_Im, _F(System_getpeername), TY_Map, TY_System, MN_("getpeername"), 1, TY_Int, FN_("fd"),
		_Public, _F(System_select), TY_Int, TY_System, MN_("select"), 5, TY_IntArray, FN_("readsocks"), TY_IntArray, FN_("writesocks"), TY_IntArray, FN_("exceptsocks"), TY_Int, FN_("timeoutSec"), TY_Int, FN_("timeoutUSec"),
		_Public|_Const|_Im, _F(System_shutdown), TY_Int, TY_System, MN_("shutdown"), 2, TY_Int, FN_("fd"), TY_Int, FN_("how"),
		_Public|_Const|_Im, _F(System_sockatmark), TY_Int, TY_System, MN_("sockatmark"), 1, TY_Int, FN_("fd"),
		_Public|_Const|_Im, _F(System_socket), TY_Int, TY_System, MN_("socket"), 3, TY_Int, FN_("family"), TY_Int, FN_("type"), TY_Int, FN_("protocol"),
		_Public|_Const|_Im, _F(System_socketpair), TY_Int, TY_System, MN_("socketpair"), 4, TY_Int, FN_("family"), TY_Int, FN_("type"), TY_Int, FN_("protocol"), TY_IntArray, FN_("pairsock"),
		_Public|_Const|_Im, _F(SockAddr_new), TY_SockAddr, TY_SockAddr, MN_("new"), 0,
		DEND,
	};
	kNameSpace_loadMethodData(ks, MethodData);
	if(IS_defineBytes()) {
		KDEFINE_METHOD MethodData2[] = {
				_Public|_Const|_Im, _F(System_sendto), TY_Int, TY_System, MN_("sendto"), 6, TY_Int, FN_("socket"), TY_Bytes, FN_("msg"), TY_Int, FN_("flag"), TY_String, FN_("dstIP"), TY_Int, FN_("dstPort"), TY_Int, FN_("family"),
				_Public|_Const|_Im, _F(System_recv), TY_Int, TY_System, MN_("recv"), 3, TY_Int, FN_("fd"), TY_Bytes, FN_("buf"), TY_Int, FN_("flags"),
//				_Public|_Const|_Im, _F(System_recvfrom), TY_Int, TY_System, MN_("recvfrom"), 4, TY_Int, FN_x, TY_Bytes, FN_y, TY_Int, FN_z, TY_Map, FN_v,
				_Public|_Const|_Im, _F(System_send), TY_Int, TY_System, MN_("send"), 3, TY_Int, FN_("fd"), TY_Bytes, FN_("msg"), TY_Int, FN_("flags"),
				DEND,
			};
		kNameSpace_loadMethodData(ks, MethodData2);
	}
	else {
		kreportf(INFO_, pline, "konoha.bytes package hasn't imported. Some features are still disabled.");
	}
	KDEFINE_INT_CONST IntData[] = {
			{"PF_LOCAL",TY_Int, PF_LOCAL},
			{"PF_UNIX", TY_Int, PF_UNIX},
			{"PF_INET", TY_Int, PF_INET},
			{"PF_INET6", TY_Int, PF_INET6},
			{"PF_APPLETALK", TY_Int, PF_APPLETALK},
#ifndef __APPLE__
			{"PF_PACKET", TY_Int, PF_PACKET},
#endif
			{"AF_LOCAL", TY_Int, AF_LOCAL},
			{"AF_UNIX", TY_Int, AF_UNIX},
			{"AF_INET", TY_Int, AF_INET},
			{"AF_INET6", TY_Int, AF_INET6},
			{"AF_APPLETALK", TY_Int, AF_APPLETALK},
#ifndef __APPLE__
			{"AF_PACKET", TY_Int, AF_PACKET},
#endif
			// Types of sockets
			{"SOCK_STREAM", TY_Int, SOCK_STREAM},
			{"SOCK_DGRAM", TY_Int, SOCK_DGRAM},
			{"SOCK_RAW", TY_Int, SOCK_RAW},
			{"SOCK_RDM", TY_Int, SOCK_RDM},
			// send & recv flags
			{"MSG_OOB", TY_Int, MSG_OOB},
			{"MSG_PEEK", TY_Int, MSG_PEEK},
			{"MSG_DONTROUTE", TY_Int, MSG_DONTROUTE},
			{"MSG_OOB", TY_Int, MSG_OOB},
			{"MSG_TRUNC", TY_Int, MSG_TRUNC},
			{"MSG_DONTWAIT", TY_Int, MSG_DONTWAIT},
			{"MSG_EOR", TY_Int, MSG_EOR},
			{"MSG_WAITALL", TY_Int, MSG_WAITALL},
#ifndef	__APPLE__
			{"MSG_CONFIRM", TY_Int, MSG_CONFIRM},
			{"MSG_ERRQUEUE", TY_Int, MSG_ERRQUEUE},
			{"MSG_NOSIGNAL", TY_Int, MSG_NOSIGNAL},
			{"MSG_MORE", TY_Int, MSG_MORE},
#endif
			// socket options
			{"SO_REUSEADDR", TY_Int, SO_REUSEADDR},
			{"SO_TYPE", TY_Int, SO_TYPE},
			{"SO_ERROR", TY_Int, SO_ERROR},
			{"SO_DONTROUTE", TY_Int, SO_DONTROUTE},
			{"SO_BROADCAST", TY_Int, SO_BROADCAST},
			{"SO_SNDBUF", TY_Int, SO_SNDBUF},
			{"SO_RCVBUF", TY_Int, SO_RCVBUF},
			{"SO_KEEPALIVE", TY_Int, SO_KEEPALIVE},
			{"SO_OOBINLINE", TY_Int, SO_OOBINLINE},
#ifndef __APPLE__
			{"SO_NO_CHECK", TY_Int, SO_NO_CHECK},
			{"SO_PRIORITY", TY_Int, SO_PRIORITY},
#endif
			{"SHUT_RD", TY_Int, SHUT_RD},
			{"SHUT_WR", TY_Int, SHUT_WR},
			{"SHUT_RDWR", TY_Int, SHUT_RDWR},
			{}
	};
	kNameSpace_loadConstData(ks, IntData, pline);
	return true;
}

static kbool_t socket_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t socket_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t socket_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

#ifdef __cplusplus
}
#endif
#endif /* SOCKET_GLUE_H_ */
